# C++ 三端框架首版

这是车载铁路周边环境巡检项目的 C++17/CMake Native 首版框架。它已经把 PCB 核心、Linux 设备/差分/控制边界、Windows 参数/下载/作业以及离线空间和 AI 契约串起来；真实 PCB、雷达、相机、UMD982、合宙 Cat4 和 Qt 页面仍需要按硬件门禁接入。

## 已实现

- PCB/主机通用二进制帧：版本、类型、序号、长度、CRC32、半包/粘包解析和坏帧重同步。
- 参数服务：范围、版本冲突、只读/保密标志及生效模式。
- 采集/掉电状态机基础。
- 版本化记录：时间标签、启动编号、同步质量、设备标识、序号、模拟标志、CRC 和严格/恢复读取。
- 确定性的四流模拟采集：IMU、GNSS、图像元数据、雷达元数据共 400 条记录。
- 差分转发边界：过期 RTCM 拒绝，源断开和 GNSS 写入错误传播。
- Linux 设备适配器边界：PCB、UMD982、XT32、海康相机、设备管理器和健康状态。
- 控制锁、参数 revision/request id、局域网范围下载和断点临时文件。
- Windows 设备客户端、可取消处理作业、标定/轨迹/点云合成基线、数据集清单、AI/跨次变化契约。
- 未安装模型或算法时返回 `NotSupported`，不生成空成功结果。

## 构建

在 Windows Developer PowerShell 中执行：

```text
cmake -S prj/framework -B prj/framework/build -DBUILD_TESTING=ON
cmake --build prj/framework/build --config Debug --parallel 4
ctest --test-dir prj/framework/build -C Debug --output-on-failure
```

如果 `cmake` 不在 PATH，使用 Visual Studio 安装目录中的 CMake。当前验证环境为 Visual Studio 18、MSVC 19.51、Windows SDK 10.0.26100。

## 运行模拟闭环

```text
prj/framework/build/Debug/capture_demo.exe --output prj/framework/demo_task
prj/framework/build/Debug/replay_cli.exe --input prj/framework/demo_task/data.rrec
```

`capture_demo` 要求输出目录不存在；它会创建任务目录、写入 400 条模拟记录和 `complete` 标记。`replay_cli` 默认严格读取，损坏或截断数据返回非零；必要时显式加 `--recover`。

## 仍需硬件和产品化接入

真实 STM32H750 BSP/寄存器级驱动、ADIS16477 SPI/DRDY、XT32 驱动、海康相机 SDK、UMD982 双天线串口与 PPS、合宙 Cat4/NTRIP 实网、HTTP 监听/认证、Qt 触控界面、生产级融合和模型推理仍属于后续接入工作。工控机按 Orin NX ARM64 设计，但普通/SUPER SKU、载板接口和 JetPack 版本仍由实物资料确定。

整机测试模板和交付门禁见 [`../验证记录/整机测试总表.md`](../验证记录/整机测试总表.md) 与 [`../交付清单.md`](../交付清单.md)。
