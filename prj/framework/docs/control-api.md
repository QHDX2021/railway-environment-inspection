# Linux 控制、差分与下载边界

## 4G/NTRIP

Cat4 拨号和 NTRIP 会话归 Linux 管理。`INtripClient` 只输出带接收单调时刻的 `CorrectionChunk`，`CorrectionForwarder` 拒绝过期数据、断开的源和 GNSS 写入失败；收到 RTCM 不等于 UMD982 已经 RTK fixed，定位状态必须来自 GNSS 原始解。

当前 `NtripClient` 是进程内可替换会话，用于测试重连、超时和龄期。实际部署时由平台适配层负责合宙 Cat4 拨号、SIM、TCP/TLS、NTRIP 请求头和串口/网口转发。

## 参数和控制锁

`ControlLock` 保证同一时间只有一个 Windows 客户端拥有设备控制权。参数写入携带期望 revision 和 request id；revision 冲突、重复请求、未知参数和只读参数都返回失败，不能静默覆盖。

任务运行期间不能释放控制锁。启停任务只接受持锁者的请求，后续 HTTP/JSON 或 Qt 层应把这些结构映射到网络，而不能绕过 `ControlService` 直接改设备。

## 局域网下载

`DownloadService` 只接受任务 ID、白名单文件 ID 和闭合范围；路径穿越、未知任务、未完成任务和越界范围均拒绝。下载以分段回调输出，返回实际字节数、文件总长和范围摘要，客户端可据此续传并校验。

当前 HTTP server 只保留服务边界和运行状态，真正监听、认证、TLS/局域网访问控制应在 Linux 部署层接入，并复用同一控制锁和下载服务。
