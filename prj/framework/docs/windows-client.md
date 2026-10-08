# Windows 上位机核心

Windows 上位机（Surface 平板上的 Qt 页面或其他 UI）只调用 `rail_desktop` 的核心接口。

`DeviceClient` 负责能力读取、参数快照、revision/request-id 校验的写入以及任务启停；Linux 端仍是设备控制者。`TaskDownloader` 先写 `.part` 临时文件，支持取消、进度回调和从已有临时文件继续下载；下载校验成功后再原子提交。取消或传输错误会保留有效前缀供下次续传，源文件不存在、范围非法等不可续传错误会清理临时文件。

`ProcessingJob` 保存输入任务、配置快照、标定引用和输出目录，状态经过排队、运行、完成/失败/取消。未安装算法时返回 `NotSupported`，缺少输入引用时在提交阶段失败；失败和取消不会生成完成清单。Qt 页面不应在 UI 线程直接读盘或执行算法。

当前客户端使用进程内 `ControlService`/`DownloadService` 验证生命周期。接入实际局域网时只需替换传输适配器，保留 request id、revision、范围下载、摘要和状态模型。
