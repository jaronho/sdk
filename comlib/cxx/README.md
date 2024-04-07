# 公共组件

存放基于**base基础库**的公共组件，各组件按目录划分。

* hfs，HTTP文件服务（基于boost.asio）。
* httpclient，HTTP客户端（基于libcurl）。

* nac，客户端网络访问控制（基于boost.asio），TCP协议（自定义网络协议）。

* rpc，跨进程通信（基于boost.asio）。

* toolkit，工具箱：
  * 应用程序单例；
  * 文件过期删除器；
  * 网络配置；
  * 其他工具；

* diagnostor，诊断器（基于boost），主要用于代码耗时诊断。