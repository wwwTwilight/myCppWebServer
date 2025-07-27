# MyCppWebServer

这是一个用 C++ 编写的简易多线程 Web 服务器项目，支持静态文件服务和 CGI 脚本，适合学习 Web 服务器原理、Socket 编程和 C++ 面向对象设计。

## 主要特性

- 支持 HTTP/1.0 和 HTTP/1.1 协议
- 支持 GET、POST 请求
- 静态文件服务（HTML、图片、PDF、文档等常见类型）
- CGI 脚本执行（如 Python、Shell 等）
- 多线程并发处理客户端连接
- 基础错误处理（404、501）
- 代码包含过程式和面向对象两种实现风格，便于对比学习

## 目录结构

```
myCppWebServer/
├── httpd.cpp           # 过程式实现的主程序
├── httpd_oop.cpp       # 面向对象实现（完整注释版）
├── httpdocs/           # 静态资源与CGI脚本目录
│   ├── index.html      # 示例首页
│   ├── post.html       # 表单页面
│   ├── post.cgi        # CGI脚本示例（需可执行权限）
│   └── ...             # 其他静态文件和脚本
├── plan.md             # 项目未来改进规划
└── Readme.md           # 项目说明文档
```

## 编译与运行

1. **编译（任选其一）**

   - 过程式版本：
     ```bash
     g++ -std=c++11 -pthread httpd.cpp -o httpd
     ```
   - 面向对象版本（此版本为AI根据过程式版本改写）：
     ```bash
     g++ -std=c++11 -pthread httpd_oop.cpp -o httpd_oop
     ```

2. **运行服务器**
   ```bash
   ./httpd         # 或 ./httpd_oop
   ```
   默认监听 8080 端口。

3. **访问服务**
   - 浏览器访问：http://localhost:8080/index.html
   - 也可访问其他静态文件或 CGI 脚本

## 主要代码说明

- `httpd.cpp`：单文件过程式实现，适合入门理解
- `httpd_oop.cpp`：面向对象实现，结构清晰，便于扩展
- `httpdocs/`：存放网页、图片、CGI脚本等资源
- `plan.md`：未来模块化、Makefile、请求头解析等改进计划

## 注意事项

- CGI 脚本需有可执行权限（如 `chmod +x post.cgi`）
- 静态文件和脚本需放在 `httpdocs/` 目录下
- 仅支持基础 HTTP 功能，安全性和健壮性未做深入优化
- 仅适合学习和实验，不建议用于生产环境

## 未来规划

详见 `plan.md`，包括：
- 代码模块化与头文件分离
- Makefile 自动化编译
- 更健壮的请求头解析与 Cookie 支持

---

欢迎学习、交流与改进建议！