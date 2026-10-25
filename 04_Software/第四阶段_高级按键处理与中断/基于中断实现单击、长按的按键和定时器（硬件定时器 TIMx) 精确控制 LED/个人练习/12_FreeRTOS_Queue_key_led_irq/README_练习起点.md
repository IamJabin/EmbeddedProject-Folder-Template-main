# 按键与硬件定时器控制 LED：练习起点

准备日期：2026-10-25。当前代码是讲师起始工程的个人副本，尚未完成本课 TIM2 功能。

## 来源与仓库边界

- 用户指定的起始目录：`F:\MCU\LiXin\01_课程内容\工程\04_Software\第四阶段_高级按键处理与中断\基于中断实现单击、长按的按键和定时器（硬件定时器 TIMx) 精确控制 LED\12_FreeRTOS_Queue_key_led_irq`。
- 实际源工程：该目录内的 `11_FreeRTOS_Queue_key_led_irq`。
- 个人练习目录：`F:\MCU\LiXin\01_课程内容\工程\04_Software\第四阶段_高级按键处理与中断\基于中断实现单击、长按的按键和定时器（硬件定时器 TIMx) 精确控制 LED\个人练习\12_FreeRTOS_Queue_key_led_irq`。
- Keil 入口：`MDK-ARM/homework_06.uvprojx`；CubeMX 入口：`homework_06.ioc`。
- 原始讲师仓库远端：`https://gitee.com/br76530067/pingxing_-homework.git`；讲师原目录及其 `.git` 未移动、未修改。
- 个人仓库：`https://github.com/IamJabin/EmbeddedProject-Folder-Template-main.git`。
- 个人练习分支：`key-tim-led-practice`；分支从上一课完成提交 `64bb13e` 建立。
- 只在个人副本中练习。完成版 `12_demo_FreeRTOS_Queue_key_led_irq/12_FreeRTOS_Queue_key_led_irq` 仅作对照。

## 原样保留的内容

复制源码、依赖及工程配置，排除嵌套 Git 元数据、常见编译产物和个人会话状态。复制文件逐一与起始工程核对 SHA-256；没有改写 C/H、IOC、UVPROJX 文件，没有提前接通本课功能或优化教程中的坑。

原工程已包含按键中断、长短按状态机、LED 队列和延时闪灯；APP 当前仅打印心跳，尚未消费按键结果。实际练习从先编译、确认此状态开始。TIM1 已用于 HAL 时基，本课将新增 TIM2，二者职责不同。

CubeMX 配置版本为 6.11.1，STM32 固件包为 F4 V1.28.1，芯片 STM32F411CEUx。

完整教学步骤和最新进度见对应视频目录的 `笔记`。串口日志存放于 `F:\_\log`，归档证据以本课调试记录为准。
