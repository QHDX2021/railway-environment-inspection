# Linux 设备接入边界

Linux 设备层只向采集管线暴露接口，不把串口、网口、USB 或厂商 SDK 细节泄漏到公共记录和回放代码。

`IByteTransport` 负责打开、读写和关闭一个物理端点；`IDevice` 负责能力、参数、启停和健康状态。当前 `PcbDevice`、`GnssDevice`、`LidarDevice` 和 `CameraDevice` 是可替换的字节传输适配器，分别预留 PCB、UMD982 双天线、XT32 和海康工业相机的设备标识。真实驱动应在这些类中接入，不改变 `Record` 和 `TaskWriter` 接口。

`DeviceManager` 是任务级唯一控制者：按设备 ID 选择设备，启动失败时回滚已经启动的设备，停止时汇总首个错误，并提供全量健康快照。UMD982 数据口仍由 Linux 独占，PPS 通过 PCB 同步链路进入记录时间标签。

采集管线完成正常 EOF 才生成任务完成标记；取消、源错误或存储写入错误调用 `abort`，任务保留失败/取消状态而不生成 `complete`。真实 ARM64 部署还需固定 JetPack、SDK、串口权限、相机驱动和设备拓扑，并执行单设备、多流、断线重连、慢盘和长时间采集测试。
