# MyCppWebServer

这是一个用 C++ 编写的简单 Web 服务器项目，主要用于学习和理解 Web 服务器的基本实现原理。项目目前采用过程式编程风格，后续会考虑用面向对象思想进行重构。

## 功能简介

- 支持 HTTP/1.0 和 HTTP/1.1 协议的基础请求处理
- 支持 GET 和 POST 请求
- 支持静态文件访问（如 HTML、PDF、JPG、PNG、DOC 等常见文件类型）
- 支持 CGI 脚本执行
- 多线程处理客户端连接
- 简单的错误处理（404、501）

## 目录结构

```
MyCppWebServer/
├── myCppWebServer/
│   ├── httpd.cpp      # 主程序源码
│   └── httpdocs/      # 静态资源目录，放置网页和文件
```

## 使用方法

1. **编译项目**

   在项目根目录下执行：

   ```bash
   g++ -std=c++11 -pthread myCppWebServer/httpd.cpp -o myCppWebServer/httpd
   ```

2. **运行服务器**

   ```bash
   ./myCppWebServer/httpd
   ```

   默认监听 8080 端口。

3. **访问网页或文件**

   将你的 HTML、图片、PDF、文档等文件放到 `httpdocs` 目录下，通过浏览器访问：

   ```
   http://localhost:8080/文件名
   ```

   例如：

   ```
   http://localhost:8080/index.html
   ```

## 主要代码说明

- `accept_request`：处理每个客户端连接，解析请求并响应
- `open_file`：读取并发送静态文件内容
- `exec_cgi`：执行 CGI 脚本并返回结果
- `not_found`、`method_not_supported`：错误响应处理
- `getHttpLine`：读取 HTTP 请求行

## 注意事项

- 仅支持基础的 HTTP 功能，安全性和性能未做优化
- 静态文件需放在 `httpdocs` 目录下
- 仅支持 GET/POST，其他方法会返回 501 错误
- 仅适合学习和实验，不建议用于生产环境

## 后续计划

- 用面向对象方式重构
- 增加更多 MIME 类型支持
- 支持更复杂的 CGI 和路由
- 增强错误处理和日志功能

---

欢迎学习和交流！