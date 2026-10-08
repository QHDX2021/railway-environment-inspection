# C++ 三端采集与回放框架实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking. 本计划尚待用户审阅并选择执行方式，不自动授权启动子代理。

**Goal:** 建立可编译的 C++ 框架，完成模拟设备采集、版本化记录、离线回放及错误验证，为真实 PCB/Linux/Windows 接入建立边界。

**Architecture:** 公共协议、参数和记录格式独立于平台。PCB 提供可主机测试的采集与电源状态机，Linux 服务负责采集和差分转发，Windows 核心负责回放与后处理接口。首版通过模拟适配器运行完整数据链路，不声称完成真实驱动、联网服务或解算算法。

**Tech Stack:** C++17、CMake、CTest；默认仅依赖标准库。Qt6、ROS2、厂商 SDK 不作为首版编译前提。

**Spec:** [C++ 框架设计](04_CPP框架设计.md)，并读取 [选型确认](05_硬件选型确认表.md) 与 [工控机截图核对](06_工控机截图参数核对.md)。

日期：2026-10-08。保存于用户指定 `prj` 目录，代码根目录为 `prj/framework/`。本计划针对设计中的首版 F01～F10，不把后续真实硬件、Qt 界面和算法实现混入本次验收。

## Global Constraints

- C++17；所有二进制整数显式小端编码，不发送 C++ 结构体内存。
- Linux 不在线解算或运行 AI；Windows 承担全部后处理职责。
- PCB 采集 ADIS16477 系列 IMU；UMD982 数据归 Linux、PPS 归 PCB；Linux 管理合宙 Cat4/NTRIP。
- 工控机按 Orin NX 系列 ARM64 适配；普通/SUPER SKU、载板接口、JetPack 版本仍不得臆定。
- 本机运行通过不能视为 Linux ARM64 或 MCU 实机通过。
- 未接入算法返回 `NotSupported`；模拟记录必须带模拟标志。
- 参数和任务快照不写凭据；采集失败、失锁、溢出不可伪装成功。
- 不改动原始历史资料，不自动下载厂商 SDK 或执行真实设备命令。
- 当前无产品源码；执行时再检查 Git 状态，不为本计划自动初始化仓库。存在 Git 时每个已验证任务可单独提交，否则保留任务勾选及验证日志，不声称已提交。

## Review Focus

1. 串口噪声、半包和恶意超长长度：有界解析、可重同步，不按未校验长度分配内存（任务 1）。
2. PCB 重启/失锁使时间倒退：启动标识与时间质量保持原样，不按接收时刻伪造源时刻（任务 1、3、6）。
3. 录制中磁盘失败或尾部断电截断：错误传播、任务不完整、严格读取失败，恢复模式报告丢失范围（任务 3、4）。
4. 配置被并发修改或设备读回不一致：版本冲突与逐项结果，不保存虚假的成功快照（任务 2）。
5. 停采时缓冲仍有数据、后台任务取消或差分连接失败：停止顺序可测、不死锁、不输出成功假象（任务 4～6）。

## 文件与构建约定

以下路径相对于 `prj/framework/`。公共命名空间为 `rail`。头文件与实现按职责同目录放置，应用只链接所需库，固件核心不链接文件系统或网络。

构建目标：`rail_protocol`、`rail_control`、`rail_firmware_core`、`rail_recording`、`rail_acquisition`、`rail_processing`，以及 `capture_demo`、`replay_cli` 和独立测试程序。

首版可移植验证命令：

```text
cmake -S prj/framework -B prj/framework/build -DBUILD_TESTING=ON
cmake --build prj/framework/build --config Debug
ctest --test-dir prj/framework/build -C Debug --output-on-failure
```

执行者先检测工具链，选择已安装的生成器；如果可执行文件不在 PATH，使用本机实际工具路径。编译器/平台结果写入验证记录。

测试不依赖 `assert`，用简单 `CHECK` 宏在失败时返回非零，保证 Release 下仍检查条件。每项功能先写对应失败测试并运行，再实现及验证；构建缺失可作为新接口的初始失败，但必须在实现后覆盖运行时断言。

## 任务 1：公共类型与 PCB 流式协议（F01、F02）

**Files:** `CMakeLists.txt`、`shared/types/types.hpp`、`shared/protocol/frame.hpp/.cpp`、`tests/test_support.hpp`、`tests/protocol_test.cpp`。

**Interfaces:**

- 定义 `ErrorCode {Ok, InvalidArgument, InvalidState, Conflict, IoError, CorruptData, UnsupportedVersion, NotSupported, Overflow, Cancelled, Timeout}` 和 `Status {ErrorCode code; std::string message;}`（主机服务使用）。固件底层接口仅返回 `ErrorCode`。
- `TimeTag`：`uint64_t ticks`、`uint32_t ticks_per_second`、`uint32_t boot_id`、`TimeQuality quality`；质量枚举 `Unsynchronized/Locked/Holdover/Invalid`。
- `ByteView {const uint8_t* data; size_t size;}`、`MutableByteView {uint8_t* data; size_t size;}`；容量与空指针关系必须验证。
- `FrameView {uint16_t type; uint32_t sequence; ByteView payload;}`。
- `ErrorCode encode_frame(FrameView, MutableByteView, size_t& written)`。
- `FrameDecoder::feed(ByteView, void* context, void(*on_frame)(void*, FrameView))`；回调视图仅在回调期间有效，解码器使用固定大小缓冲。

协议 v1：两字节魔数 `0x52 0x49`、版本 u8=1、保留 u8=0、类型 u16、序号 u32、负载长度 u16、负载、CRC32 u32。负载上限 4096 字节，CRC 覆盖版本至负载末尾，CRC32/ISO-HDLC（反射多项式 0xEDB88320、初值与最终异或均 0xFFFFFFFF）。未知消息类型交给分发层返回不支持；未知协议版本不交付。

- [ ] 写失败测试：`crc_known_vector` 检查 `123456789 → 0xCBF43926`；`bytewise_and_concatenated` 检查分字节/粘包均准确交付；`corrupt_then_valid` 检查坏 CRC 后恢复；`oversize_and_unknown_version` 拒绝长度 4097 和版本 2；缓冲不足返回错误且不越界。
- [ ] 建立最小 CMake/CTest，运行 `ctest ... -R protocol` 记录失败。
- [ ] 实现类型、显式编码、固定缓冲解码与错误统计，不在固件解码路径动态分配。
- [ ] 运行协议测试和 Debug 构建，记录结果；有 Git 时提交本任务。

## 任务 2：参数管理与 PCB 状态机（F03、F04）

**Files:** `shared/configuration/parameters.hpp/.cpp`、`firmware/core/state_machine.hpp/.cpp`、`firmware/hal/interfaces.hpp`、`tests/control_test.cpp`。

**Interfaces:**

- `ParameterValue = std::variant<bool,int64_t,double,std::string>`；`ParameterDescriptor` 包含名称、默认值、可选范围/单位、只读、`ApplyMode {Immediate, IdleOnly, Restart}` 和 `secret` 标志。
- `IParameterDevice::write(const std::string&, const ParameterValue&) -> Status`；`read(const std::string&, ParameterValue&) -> Status`。
- `ParameterService::set(name, value, uint64_t expected_revision) -> Status`，只在读回一致后递增版本；重启参数作为 pending 保存，不伪装已生效。
- `snapshot()` 返回去除 secret 参数后的键值；当前值、待生效值分别表示。
- `AcquisitionStateMachine::dispatch(AcquisitionEvent) -> ErrorCode`；状态按设计 Idle/Starting/Recording/Stopping/Fault。
- `PowerStateMachine::dispatch(PowerEvent) -> PowerAction`；动作可要求停采、通知、请求系统关机、断电或故障。把 `DataFlushed` 和 `OsShutdownComplete` 分开；超时是否断电由显式策略提供。

- [ ] 写测试：采集中修改 IdleOnly 触发频率被拒；曝光越界被拒；旧版本返回 Conflict；读回不一致不更新当前值；secret 不出现在快照；Restart 修改只进入 pending。
- [ ] 写状态测试：非法启停被拒；重复同一控制请求编号不重复启动；仅 DataFlushed 不断电，收到 OsShutdownComplete 才允许断电；超时按策略分支执行。
- [ ] 运行 `ctest ... -R control` 确认失败，然后实现参数服务、请求去重和状态机。
- [ ] 测试通过后记录参数与状态图；有 Git 时提交本任务。

## 任务 3：任务记录与恢复读取（F05、F06）

**Files:** `shared/recording/record.hpp`、`shared/recording/codec.hpp/.cpp`、`shared/recording/reader.hpp/.cpp`、`linux/storage/task_writer.hpp/.cpp`、`tests/recording_test.cpp`。

**Interfaces:**

- `Record`：类型、流编号、设备 ID、TimeTag、u64 序号、u64 主机单调接收时间、模拟标志、字节负载。
- `IRecordingWriter::append(const Record&) -> Status`、`finish() -> Status`；写入失败后 finish 不可变为成功。
- `TaskWriter::create(const std::filesystem::path&, const TaskMetadata&) -> Status`，已有目录拒绝覆盖。
- `RecordReader::read(path, ReadMode, callback) -> ReadReport`；`ReadMode {Strict, Recover}`，报告状态、有效数量、损坏位置、是否正常关闭和时间质量统计。

采用磁盘格式 v1：文件魔数 `RIREC001`；每条记录长度 u32、显式编码记录内容、CRC32。单条负载上限 16MiB（与 PCB 帧上限不同）；固定字段定义及长度写入 `docs/recording-format.md`。任务清单 UTF-8 键值，记录分段、设备、配置版本、时间单位、模拟标识和结束状态。

- [ ] 写 roundtrip 测试：模拟标志、不同 boot_id、失锁标签、嵌入零字节负载完整保留；中文/空格目录可创建并回放。
- [ ] 写失败测试：尾部截断、坏 CRC、超长记录、未知文件版本；Strict 返回失败，Recover 仅报告有效前缀并显式标记不完整，不跳过损坏后假装完整。
- [ ] 写 I/O 注入测试：中途 writer 失败后任务保持 incomplete；现有输出目录不能覆盖；不依赖真实“磁盘满”制造测试条件。
- [ ] 运行 `ctest ... -R recording` 记录失败后实现写入/分段/读取和异常报告；索引可从有效记录重建，索引不作为数据真值。
- [ ] 全部通过后写格式文档与验证记录；有 Git 时提交本任务。

## 任务 4：有界采集管线与模拟设备（F05、F07）

**Files:** `linux/acquisition/pipeline.hpp/.cpp`、`linux/devices/interfaces.hpp`、`adapters/simulation/sample_source.hpp/.cpp`、`apps/capture_demo/main.cpp`、`tests/acquisition_test.cpp`。

**Interfaces:**

- `ISampleSource::next(Record&) -> SourceResult`，`SourceResult {Status status; bool eof;}`；`stop()` 要求有界退出，适配器不能永久阻塞。
- `Pipeline::run(ISampleSource&, IRecordingWriter&, const PipelineOptions&) -> RunReport`；options 包括容量和取消标志，报告包含生产/保存/丢失数量、错误和正常结束标志。
- 固定种子模拟源生成每流 100 条 IMU/GNSS/图像元数据/雷达元数据记录，共 400 条；图像与雷达元数据不能被标记为真实图像或点云。
- CLI：`capture_demo --output <new-directory> --samples-per-stream 100`；非法参数返回非零。

- [ ] 写端到端测试：400 条记录准确回读，流内序号连续；文件中显式模拟来源。
- [ ] 写背压测试：容量 1，用受控慢写入器和同步闸门制造队列满；明确 Overflow、不完整任务和计数，不靠任意 sleep 碰运气。
- [ ] 写停止测试：正常停源后排空队列，finish 最后执行；I/O 错误或取消能唤醒等待线程并 join；取消任务标记 Cancelled/incomplete。
- [ ] 运行 `ctest ... -R acquisition` 确认失败，实现有界队列、管线和模拟 CLI。
- [ ] 运行 CLI 生成目录并用 reader 测试验证；有 Git 时提交本任务。

## 任务 5：差分和控制服务边界（F04、F08）

**Files:** `linux/corrections/forwarder.hpp/.cpp`、`linux/control/service.hpp/.cpp`、`adapters/simulation/gnss_session.hpp/.cpp`、`tests/corrections_test.cpp`、`docs/control-api.md`。

**Interfaces:**

- `CorrectionChunk {std::vector<uint8_t> data; uint64_t received_monotonic_ns;}`。
- `ICorrectionSource::next(CorrectionChunk&) -> SourceResult`；`IGnssSession::write_correction(ByteView) -> Status`，以单一所有者串行化 GNSS 控制和数据写入。
- `CorrectionForwarder::step(uint64_t now_ns) -> Status`；失效时间由配置传入，不能硬编码为所有差分服务均适用的常量。
- `ControlService` 暴露能力查询、参数读写及任务状态，复用任务 2 服务，不另写一套参数规则。

- [ ] 测试有效字节原样转发；源断开、GNSS 写失败向上传播；超龄数据不转发；收到字节不自动标记 RTK fixed。
- [ ] 测试回退时钟/重连清空旧会话积压；重连不会让过期数据变为“刚收到”。
- [ ] 运行 `ctest ... -R corrections` 确认失败后实现模拟链路。
- [ ] 文档定义 `/v1/devices`、参数 revision、任务启停、封闭文件下载、错误和请求编号；明确真实 HTTP/认证/Range 下载仍未实现。
- [ ] 测试通过并记录模拟与实网边界；有 Git 时提交本任务。

## 任务 6：Windows 回放、处理接口及整体交付（F09、F10）

**Files:** `desktop/playback/playback.hpp/.cpp`、`desktop/processing/interfaces.hpp`、`desktop/application/job.hpp/.cpp`、`apps/replay_cli/main.cpp`、`tests/playback_test.cpp`、`tests/processing_test.cpp`、`README.md`、`docs/verification.md`、`docs/hardware-integration.md`。

**Interfaces:**

- `Playback::open(path, ReadMode) -> Status`、`next(Record&) -> SourceResult`；时间筛选显式选定时钟域/boot_id，同步未锁定时不得按 UTC 无提示排序。
- `ProcessingRequest` 带输入任务、配置和输出目录；`ProcessingResult` 带 Status 与成果引用，失败时不写“完成”清单。
- 设计中的五类处理器均提供 `run(const ProcessingRequest&, const CancellationToken&, ProgressCallback) -> ProcessingResult`。`CancellationToken` 持有原子取消状态，`ProgressCallback` 为明确生命周期的主机回调。
- 未安装处理器统一返回 NotSupported；任务生命周期区分排队、运行、完成、失败、取消。
- CLI：`replay_cli --input <task-directory>`；`--recover` 显式启用恢复；严格模式发现损坏返回非零。

- [ ] 测试完整任务计数、流筛选、时间质量、不同 boot_id 的隔离；损坏输入默认失败。
- [ ] 测试未安装算法不产生空“成功”结果；模拟长任务可取消，终态不显示完成；失败保留原因。
- [ ] 运行 `ctest ... -R "playback|processing"` 确认失败，再实现回放 CLI、处理接口及任务状态。
- [ ] 完成全套构建和 CTest；执行 capture_demo → replay_cli，保存实际记录数量、命令、编译器和测试平台结果。
- [ ] 文档列明已实现/模拟/未实现；硬件接入说明覆盖 UMD982 双天线、PCB 同步、ARM64 SDK、合宙具体型号、Surface 架构、网口和存储资源。
- [ ] 更新 `prj/README.md` 到真实代码入口；有 Git 时提交最终验证结果。实现未完成前不勾选任务或声称通过。

## 计划自查与范围映射

F01/F02→任务 1；F03/F04→任务 2；F05/F06→任务 3、4；F07→任务 4；F08→任务 5；F09/F10→任务 6。

五个 Review Focus 均已对应到具体测试步骤。协议帧负载 4096 字节与磁盘记录负载 16MiB 属于不同层，不相互套用。固件仅依赖 ErrorCode/固定缓冲协议和状态机，std::string/文件系统/线程属于主机服务。实现不包含真实 NTRIP 会话、BSP、厂商 SDK、HTTP 服务器、Qt 完整界面及五种算法的数学实现，均保持原产品后续范围。

## 执行方式待选择

建议在当前会话顺序执行：各任务共享协议和数据类型，顺序实现便于及时校正接口，最后做一次整体审查。也可选择分任务子代理实现和独立审查，代价是更多上下文与协调。书面计划审阅及执行方式明确后进入代码实现。
