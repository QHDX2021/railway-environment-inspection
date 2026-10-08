# 工控机商品页 Python 抓取记录

日期：2026-09-30。

请求：用户提供的商品 ID 718486986814、SKU 5650971165647 对应公开页面。使用 Python 标准库 urllib，未提供浏览器 Cookie 或账号认证。

## 结果

- HTTP 状态：200。
- 响应长度：5,143 字节。
- 类型：text/html，UTF-8。
- 页面标题和可见正文为空。
- HTML 中根据浏览器类型选择 `login.taobao.com/member/login.jhtml` 或 `login.m.taobao.com/login.htm`，并通过 JavaScript 跳转登录。
- 没有取得商品名称、载板、接口、套餐或价格；不能据此补填硬件参数。

## 文件

- [抓取脚本](../../tools/fetch_product_page.py)
- [响应 HTML](response.html)
- [请求结果](fetch_report.json)
- [提取正文](visible_text.txt)：本次为空。

本次没有执行页面登录脚本、导出浏览器凭据或尝试绕过验证。继续读取需要可正常访问商品详情的浏览器会话，或者商品页提供的公开资料。
