# PCB 固件核心边界

固件核心只依赖 `firmware/hal` 中的接口，不依赖主机文件系统、网络、厂商 SDK 或 Qt。目标硬件为 STM32H750，具体 SPI、DRDY、PPS、GPIO 和定时器寄存器在板级 HAL 中实现。

`ImuCapture` 使用固定数组保存 IMU 样本。数据就绪中断路径只读取一次 burst、分配递增序号并入队；缓存满时返回 `Overflow` 并累计统计，不能静默覆盖。主循环通过 `pop` 取走样本，协议应答和日志留在任务上下文。

`IImuHal`、`ITimeHal`、`IPowerHal` 和 `IHostTransport` 是板级替换点。主机协议处理任务启停、状态和能力消息，所有响应带回原请求编号。采集状态机与电源状态机负责启动、停止、故障、外部掉电、数据刷写和安全断电顺序。

主机侧 `firmware_core_test` 覆盖固定缓存、连续序号、溢出统计、PPS 映射、协议启停和电源事件。真实板级验证仍需在 STM32H750 样板上测量 SPI 截止时间、DRDY 抖动、PPS 映射误差、触发输出和掉电握手。
