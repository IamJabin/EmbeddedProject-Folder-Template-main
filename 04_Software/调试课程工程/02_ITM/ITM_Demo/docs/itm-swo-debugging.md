# STM32F411 ITM/SWO 排障手册

## 适用范围

本文记录本项目已验证的 ITM/SWO 配置和完整排障过程，用于分离应用、CoreSight、
STM32 引脚路由、物理接触、J-Link 接收和 Keil Trace 配置问题。

| 项目 | 已验证值 |
|---|---|
| MCU | STM32F411CEUx，UFQFPN48 |
| CPU/SystemCoreClock | 100 MHz |
| SWO 引脚 | PB3，AF0 |
| SWO 编码 | NRZ/UART |
| SWO 位速率 | 1 MHz |
| TPIU 分频 | `ACPR=99` |
| ITM 端口 | Port 0 |
| J-Link | 标准 20 针，SWO 输入 pin 13 |

## 永久固件要求

在 `SystemClock_Config()` 之后、第一次 Trace 写入之前初始化。除 Cortex-M
CoreSight 寄存器外，必须打开 STM32 的 Trace 引脚门控：

```c
CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
MODIFY_REG(DBGMCU->CR,
           DBGMCU_CR_TRACE_IOEN | DBGMCU_CR_TRACE_MODE,
           DBGMCU_CR_TRACE_IOEN);

ITM->LAR = 0xC5ACCE55UL;
TPI->ACPR = (SystemCoreClock / ITM_SWO_BAUDRATE_HZ) - 1UL;
TPI->SPPR = 2UL;

ITM->TPR = 0UL;
ITM->TER = 1UL;
ITM->TCR = ITM_TCR_ITMENA_Msk |
           ITM_TCR_SYNCENA_Msk |
           ITM_TCR_DWTENA_Msk |
           (1UL << ITM_TCR_TraceBusID_Pos);
```

日志重定向必须非阻塞：

```c
int fputc(int ch, FILE *f)
{
  (void)f;

  if (((ITM->TCR & ITM_TCR_ITMENA_Msk) != 0UL) &&
      ((ITM->TER & 1UL) != 0UL) &&
      (ITM->PORT[0U].u32 != 0UL))
  {
    ITM->PORT[0U].u8 = (uint8_t)ch;
  }

  return ch;
}
```

不要直接使用可能在 stimulus ready 上无限等待的 `ITM_SendChar()`。

## 分层诊断

### 1. 证明新镜像正在运行

核对源码、AXF/HEX 和 Flash 操作时间。不要从“烧写成功”推断应用已运行，直接读 PC。

本项目曾读到：

```text
PC = 0x20000000
```

CPU 当时停在 RAM Flash Loader，PB12 和 SWO 代码都没有运行。执行 J-Link `r`
再执行 `g`，或启用 Keil `Reset and Run`。PC 回到 `0x08000000` Flash 区域后，
才能继续诊断应用。

### 2. 证明字符写入确实执行

不要只确认 `printf()` 出现在源码。检查 map 和反汇编，再对当前镜像的 stimulus
写指令设置临时硬件断点。本次实测：

```text
0x08000EF4: STRB r0,[r1]
r0 = 0x00000041        ; 'A'
r1 = 0xE0000000        ; ITM stimulus port 0
r2 = 0x00000001        ; port ready
```

链接后地址会变化，不能永久复用 `0x08000EF4`。J-Link 断点句柄只在当前会话
有效，应在同一会话清除并恢复运行。

### 3. 读取 Trace 配置

| 寄存器 | 地址 | 预期解释 |
|---|---:|---|
| `CoreDebug->DEMCR` | `0xE000EDFC` | bit 24 `TRCENA` 置位 |
| `DBGMCU->CR` | `0xE0042004` | bit 5 `TRACE_IOEN` 置位，`TRACE_MODE=0` |
| `ITM->PORT[0]` | `0xE0000000` | Ready 时读值非零 |
| `ITM->TER` | `0xE0000E00` | bit 0 置位 |
| `ITM->TCR` | `0xE0000E80` | `ITMENA` 置位；本项目为 `0x0001000D` |
| `TPI->ACPR` | `0xE0040010` | 100 MHz / 1 MHz 时为 `99` |
| `TPI->SPPR` | `0xE00400F0` | `2` 表示 NRZ |
| `GPIOB->MODER` | `0x40020400` | PB3 模式为 `10`，复用功能 |
| `GPIOB->AFRL` | `0x40020420` | PB3 为 AF0 |

原始固件的决定性缺口：

```text
DBGMCU->CR = 0x00000007   ; TRACE_IOEN 为 0
```

修复后：

```text
DBGMCU->CR = 0x00000027   ; TRACE_IOEN 已置位
```

IDE 或 Viewer 可能打开额外 ITM 端口或 formatter 位。验证必需位，不要求每个
寄存器完全等于一次快照。

### 4. 分离引脚路由和物理接触

NRZ SWO 空闲时为高电平。逻辑分析仪只看到高，不能证明线路连通，也不能证明
TPIU 已输出数据。

本次采用的可恢复验证：

1. 读取并保存 GPIOB 配置。
2. 临时把 PB3 从 AF0 改为推挽 GPIO 输出。
3. 强制拉低，确认逻辑分析仪也读到低。
4. 系统复位，由固件恢复 PB3 AF0 和 Trace 初始化。

开发板插在面包板上时存在排针接触不良。将开发板按紧后，PB3 GPIO 拉低和 SWO
接收都恢复正常。这是本项目的真实故障，不应泛化成所有 SWO 故障的默认原因。

逻辑分析仪建议使用至少 10 MS/s，优先 20 MS/s 或更高；下降沿触发，捕获至少
200 ms。1 MHz NRZ 的单个位宽约 1 us，波形应为空闲高并周期性出现短数据包。

### 5. 绕开 Keil 接收端

关闭或释放 Keil Debug 会话，再运行独立 Viewer：

```powershell
& 'D:\software\major\JLink\JLinkSWOViewerCL.exe' `
  -device STM32F411CE `
  -cpufreq 100000000 `
  -swofreq 1000000 `
  -itmport 1
```

当前安装版本把 `-itmport` 解释为位掩码：`1` 选择 Port 0，`0` 表示不选择
任何端口。修复 `TRACE_IOEN` 和物理接触后，独立 Viewer 连续收到：

```text
A
A
A
```

这证明了从 `printf()`、ITM、TPIU、PB3、线缆、J-Link pin 13 到主机解码器的
完整链路。

## J-Link 只读检查模板

把 `g` 预先放在命令列表中，避免读取后遗留 Halt 状态：

```powershell
@(
  'halt',
  'regs',
  'mem32 0xE0042004 1',
  'mem32 0xE000EDFC 1',
  'mem32 0xE0000000 1',
  'mem32 0xE0000E00 1',
  'mem32 0xE0000E80 1',
  'mem32 0xE0040010 1',
  'mem32 0xE00400F0 1',
  'mem32 0x40020400 10',
  'g',
  'exit'
) | & 'D:\software\major\JLink\JLink.exe' `
  -NoGui 1 -Device STM32F411CE -If SWD -Speed 4000 -AutoConnect 1
```

先读寄存器，再决定是否修改源码或 Flash。Reset、临时断点、GPIO 覆盖和 Flash
下载都会改变目标状态，必须提供明确恢复路径。

## 故障分类

| 证据 | 下一步重点 |
|---|---|
| PC 位于 SRAM Flash Loader | reset/run 流程，不是 GPIO/SWO |
| `printf()` 未进入 `fputc()` | C 库重定向和链接配置 |
| Stimulus 写入条件为假 | ITM、TER、权限和调试状态 |
| 写入执行但 `TRACE_IOEN=0` | STM32 DBGMCU Trace 门控 |
| 寄存器正确但 PB3 无波形 | 引脚复用、封装、接触或板级负载 |
| 波形到达 J-Link pin 13，Viewer 为空 | J-Link 输入、固件或主机解码 |
| 独立 Viewer 正常，Keil 为空 | Keil 时钟、编码和 Port 0 配置 |

## 官方资料

- ST STM32F411 文档和 RM0383：
  https://www.st.com/en/microcontrollers-microprocessors/stm32f411/documentation.html
- Arm Cortex-M4：
  https://developer.arm.com/compute-ip/cortex-m4
- Arm ITM 示例：
  https://developer.arm.com/community/arm-community-blogs/b/tools-software-ides-blog/posts/trace-cortex-m-software-with-the-instruction-trace-macrocell-itm
- SEGGER SWO：
  https://kb.segger.com/SWO
- SEGGER J-Link 接口说明：
  https://www.segger.com/products/debug-probes/j-link/technology/interface-description/

