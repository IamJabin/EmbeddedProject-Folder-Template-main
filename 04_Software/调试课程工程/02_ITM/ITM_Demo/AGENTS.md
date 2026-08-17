# 项目执行规则

## 目标与 Trace 配置

- 目标芯片：STM32F411CEUx，UFQFPN48。
- 系统时钟：HSI + PLL，100 MHz。
- SWO：PB3、AF0、异步 NRZ、1 MHz。
- 标准 J-Link 20 针连接器通过 pin 13 接收 SWO。使用适配器或 Cortex 小型
  连接器时必须重新核对引脚定义。
- 修改 ITM、TPIU、DBGMCU、PB3、Keil Trace 或 J-Link 设置前，先完整阅读
  `docs/itm-swo-debugging.md`。

## 源码修改

- 保留 CubeMX 用户代码区，不要把自定义 Trace 逻辑放入会被重新生成覆盖的区域。
- 仅在 `SystemClock_Config()` 之后初始化 ITM，确保 `SystemCoreClock` 与 TPIU
  分频使用同一时钟。
- 保持 `fputc()` 非阻塞。Trace 接收端不可用时，应用不能卡在
  `ITM_SendChar()` 或无界 stimulus-port 等待中。
- 显式配置 STM32 Trace 引脚路由。本芯片的异步 SWO 需要
  `DBGMCU_CR.TRACE_IOEN=1` 且 `TRACE_MODE=0`。
- PB12 翻转只是逻辑分析仪自检，不是永久 ITM 配置。实验结束后应删除或使用
  条件编译隔离。

## 构建与硬件验证

- 在 `MDK-ARM` 目录执行：

  ```powershell
  & 'D:\software\major\Keil_v5\UV4\UV4.exe' -b 'ITM_Demo.uvprojx'
  ```

- 确认构建报告为 `0 Error(s), 0 Warning(s)`。
- Flash 下载成功不能证明应用已运行。PC 若位于 `0x20000000` 附近的 SRAM，
  先复位并运行目标，再判断 GPIO 或 SWO。
- J-Link 现场检查默认先只读。执行 `halt` 后，离开会话前必须用 `g` 恢复运行，
  除非用户明确要求保持 Halt。
- 未经明确要求，不要擦除 Flash、修改 Option Bytes，也不要遗留临时断点或
  GPIO 覆盖。

