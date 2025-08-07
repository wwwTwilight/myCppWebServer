# MyCppWebServer

一个使用 C++ 实现的多线程 HTTP Web 服务器，支持静态文件服务、用户认证、文件上传/下载和文件管理功能，类似一个在线网盘项目。

## 特性

- 🚀 **多线程并发处理** - 使用线程池处理多个客户端连接
- 🌐 **HTTP/1.1 协议支持** - 完整的 HTTP 请求解析和响应
- 🔐 **用户身份验证** - 基于 Cookie 的登录认证系统
- 📁 **文件管理系统** - 文件上传、下载、删除功能
- 🎨 **现代化 Web 界面** - 响应式设计的前端页面
- 📡 **RESTful API** - JSON 格式的文件操作接口
- 🛡️ **安全防护** - 路径遍历攻击防护，文件名安全处理

## 项目结构

```
myCppWebServer/
├── main.cpp                    # 主程序入口
├── Makefile                    # 编译配置文件
├── include/                    # 头文件目录
│   ├── webserverSet.h         # 服务器配置
│   ├── requestHandler.h       # 请求处理器
│   ├── route.h                # 路由管理
│   ├── httpMes.h              # HTTP 消息解析
│   ├── cookie.h               # Cookie 处理
│   ├── utils.h                # 工具函数
│   └── error.h                # 错误处理
├── src/                       # 源文件目录
│   ├── webserverSet.cpp       # 服务器设置实现
│   ├── requestHandler.cpp     # 请求处理实现
│   ├── route.cpp              # 路由系统实现
│   ├── httpMes.cpp            # HTTP 消息处理
│   ├── cookie.cpp             # Cookie 管理
│   ├── utils.cpp              # 工具函数实现
│   └── error.cpp              # 错误处理实现
├── httpdocs/                  # 静态文件目录
│   ├── index.html             # 主页
│   ├── login.html             # 登录页面
│   ├── upload.html            # 文件上传页面
│   ├── files.html             # 文件管理页面
│   ├── about.html             # 关于页面
│   └── contact.html           # 联系页面
└── upload/                    # 上传文件存储目录
    └── testFile.jpeg          # 示例上传文件
```

## 功能模块

### 1. 路由系统
- **GET 路由**: 静态页面服务、文件下载、API 接口
- **POST 路由**: 用户登录、文件上传
- **动态路由**: 支持查询参数和路径参数

### 2. 用户认证
- 用户名: `admin`
- 密码: `123456`
- 基于 Cookie 的会话管理
- 受保护的页面访问控制

### 3. 文件管理
- **上传**: 支持 multipart/form-data 格式
- **下载**: 提供文件下载接口
- **删除**: RESTful API 删除文件
- **列表**: JSON 格式返回文件列表

### 4. API 接口

| 接口 | 方法 | 功能 | 参数 |
|------|------|------|------|
| `/api/files` | GET | 获取文件列表 | 无 |
| `/api/download` | GET | 下载文件 | `file=文件名` |
| `/api/delete` | GET | 删除文件 | `file=文件名` |
| `/upload` | POST | 上传文件 | multipart 表单数据 |

## 编译和运行

### 编译要求
- C++20 或更高版本
- GCC 编译器
- Linux/macOS 系统

### 编译命令
```bash
# 编译项目
make

# 或者重新编译
make rebuild

# 查看项目信息
make info

# 清理编译文件
make clean
```

### 运行服务器
```bash
# 直接运行
./httpd

# 或使用 Makefile
make run
```

服务器将在 `http://localhost:8080` 启动

## 使用说明

### 1. 访问主页
打开浏览器访问 `http://localhost:8080` 或 `http://localhost:8080/index.html`

### 2. 用户登录
1. 点击"登录账号"或访问 `/post.html`
2. 输入用户名: `admin`，密码: `123456`
3. 登录成功后获得文件管理权限

### 3. 文件上传
1. 访问 `/upload.html` 页面
2. 选择要上传的文件
3. 点击上传按钮

### 4. 文件管理
1. 访问 `/files.html` 页面（需要登录）
2. 查看、下载或删除已上传的文件

## 技术实现

### HTTP 协议处理
- 支持 HTTP/1.1 协议
- 完整的请求头解析
- Content-Length 和 Transfer-Encoding 支持
- MIME 类型自动识别

### 多线程架构
```cpp
// 主循环接受连接
while (1) {
    client_socket = accept(server_socket, ...);
    thread new_thread(accept_request, client_socket);
    new_thread.detach();
}
```

### 安全特性
- URL 解码防护
- 路径遍历攻击防护
- 文件名安全处理
- Cookie 验证机制

## 配置说明

### 服务器配置
- **端口**: 8080 (可在 main.cpp 中修改)
- **文档根目录**: `httpdocs/`
- **上传目录**: `upload/`
- **最大连接数**: 无限制 (系统限制)

### 支持的文件类型
- HTML: `text/html`
- CSS: `text/css`
- JavaScript: `application/javascript`
- 图片: `image/jpeg`, `image/png`, `image/gif`
- 二进制文件: `application/octet-stream`

## 开发扩展

### 添加新路由
在 `src/route.cpp` 的 `routeInit()` 函数中添加:
```cpp
get_routes["/new-page"] = your_handler_function;
post_routes["/new-api"] = your_post_handler;
```

### 自定义错误页面
修改 `src/error.cpp` 中的错误处理函数

### 添加新的 MIME 类型
在 `src/utils.cpp` 的 `get_mime_type()` 函数中添加

## 未来规划

### MySQL的引入

计划使用MySQL数据库来存储用户信息和文件元数据，提升数据管理能力和扩展性。

### 文件系统完善

目前仅支持上传文件以及删除下载文件，未来将增加文件夹管理、批量操作和重命名等功能。

### 为文件的读写加锁

目前没有对文件的读写进行加锁，未来将实现文件锁机制，防止并发读写冲突。

## 作者

- GitHub: [wwwTwilight](https://github.com/wwwTwilight)
- 项目: myCppWebServer

---
