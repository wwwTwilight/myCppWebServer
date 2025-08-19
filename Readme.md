# 🌐 MyPoorWebServerInCpp

> 一个轻量级、高性能的 C++ HTTP Web 服务器，专为学习和实践而设计

[![Language](https://img.shields.io/badge/Language-C++20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS-lightgrey.svg)](https://github.com/wwwTwilight/myCppWebServer)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

一个从零开始实现的 C++ HTTP Web 服务器，支持多线程并发、用户认证、文件管理等功能。项目名为 "MyPoorWebServer"，寓意虽然简陋但功能完整，是学习网络编程和 Web 服务器原理的绝佳实践项目。

## ✨ 核心特性

- 🚀 **高性能多线程** - 基于线程池的并发请求处理
- 🌐 **完整 HTTP/1.1** - 标准 HTTP 协议实现，支持持久连接
- 🔐 **安全认证系统** - Cookie 会话管理与用户权限控制
- 📁 **分用户文件管理** - 独立用户空间的文件上传、预览、下载
- 🎨 **现代化界面** - 响应式设计，支持拖拽上传
- 📡 **RESTful API** - 标准化 JSON 接口设计
- 🛡️ **安全防护** - 路径遍历防护、文件名过滤、权限验证
- 📊 **实时反馈** - 文件操作进度显示与状态提示

## 📁 项目架构

```
MyPoorWebServerInCpp/
├── 📄 main.cpp                    # 🚀 主程序入口 & 服务器启动
├── 📄 Makefile                    # 🔧 构建配置文件
├── 📄 list_sources.sh             # 📜 源码统计脚本
├── 📂 include/                    # 📚 头文件目录
│   ├── webserverSet.h             #   ⚙️  服务器配置管理
│   ├── requestHandler.h           #   🔄 HTTP 请求处理器
│   ├── route.h                    #   🛤️  路由系统定义
│   ├── httpMes.h                  #   📨 HTTP 消息解析
│   ├── cookie.h                   #   🍪 Cookie 会话管理
│   ├── utils.h                    #   🛠️  通用工具函数
│   ├── error.h                    #   ❌ 错误处理定义
│   └── sql.h                      #   🗄️  数据库接口
├── 📂 src/                        # 💻 源代码实现
│   ├── webserverSet.cpp           #   ⚙️  服务器核心配置
│   ├── requestHandler.cpp         #   🔄 请求分发处理
│   ├── route.cpp                  #   🛤️  路由逻辑实现
│   ├── httpMes.cpp                #   📨 HTTP 协议解析
│   ├── cookie.cpp                 #   🍪 会话状态管理
│   ├── utils.cpp                  #   🛠️  工具函数实现
│   ├── error.cpp                  #   ❌ 错误响应处理
│   └── sql.cpp                    #   🗄️  用户数据管理
├── 📂 httpdocs/                   # 🌐 Web 静态资源
│   ├── index.html                 #   🏠 项目主页
│   ├── post.html                  #   🔐 用户登录页面
│   ├── register.html              #   📝 用户注册页面
│   ├── upload.html                #   📤 文件上传界面
│   ├── files.html                 #   📂 文件管理中心
│   ├── about.html                 #   ℹ️  项目介绍页面
│   └── contact.html               #   📞 联系信息页面
└── 📂 upload/                     # 💾 用户文件存储
    ├── admin/                     #   👤 管理员文件目录
    ├── test/                      #   🧪 测试用户目录
    └── [username]/                #   👥 动态用户目录
```

## 🔧 核心功能模块

### 🛤️ 智能路由系统
- **静态路由**: HTML/CSS/JS 等静态资源服务
- **动态路由**: API 接口与数据交互
- **权限路由**: 基于 Cookie 的访问控制
- **文件路由**: 分用户目录的文件访问

### 👤 用户认证与管理  
```cpp
// 默认测试账户
用户名: admin    密码: 123456
用户名: test     密码: password
```
- 🔐 安全的密码验证机制
- 🍪 基于 Cookie 的会话管理
- ⏰ 自动过期与权限验证
- 🚪 统一的登录/注册入口

### 📁 分用户文件系统
- **独立空间**: 每个用户拥有独立的文件目录
- **安全隔离**: 用户间文件完全隔离，无法跨用户访问
- **实时管理**: 上传、预览、下载、删除一体化
- **格式支持**: 图片、文档、视频、压缩包等多格式

### 📡 RESTful API 接口

| 端点 | 方法 | 功能描述 | 参数 | 权限 |
|------|------|----------|------|------|
| `/api/register` | POST | 用户注册 | `username`, `password` | 公开 |
| `/api/post` | POST | 用户登录 | `username`, `password` | 公开 |
| `/api/files` | GET | 获取用户文件列表 | 无 | 需登录 |
| `/api/upload` | POST | 文件上传 | `multipart/form-data` | 需登录 |
| `/api/download` | GET | 强制下载文件 | `file=文件名` | 需登录 |
| `/api/delete` | GET | 删除用户文件 | `file=文件名` | 需登录 |
| `/upload/[user]/[file]` | GET | 预览/打开文件 | 路径参数 | 需登录 |

## 🚀 快速开始

### 📋 环境要求
- **操作系统**: Linux / macOS
- **编译器**: GCC 7.0+ (支持 C++17/20)
- **依赖**: 标准 C++ 库 (无第三方依赖)
- **内存**: 最低 256MB RAM
- **存储**: 至少 50MB 可用空间

### ⚡ 一键启动
```bash
# 1. 克隆项目
git clone https://github.com/wwwTwilight/myCppWebServer.git
cd MyPoorWebServerInCpp

# 2. 编译项目
make

# 3. 启动服务器
./httpd
# 或者使用 make 命令
make run

# 🎉 打开浏览器访问: http://localhost:8080
```

### 🔧 编译选项
```bash
make           # 标准编译
make rebuild   # 重新编译
make clean     # 清理编译文件
make info      # 查看项目信息
make debug     # 调试版本编译
```

## 📖 使用指南

### 🏠 1. 访问主页
```bash
http://localhost:8080           # 主页
http://localhost:8080/index.html
```

### 👤 2. 用户系统
```bash
# 注册新用户
http://localhost:8080/register.html
- 输入用户名和密码
- 系统自动创建用户目录

# 用户登录  
http://localhost:8080/post.html
- 默认账户: admin / 123456
- 登录成功获得文件管理权限
```

### 📤 3. 文件上传
```bash
http://localhost:8080/upload.html
```
- 🖱️ **拖拽上传**: 直接拖拽文件到上传区域
- 📁 **点击选择**: 点击上传区域选择文件
- 📊 **实时进度**: 上传进度条显示
- ✅ **格式检查**: 自动验证文件类型和大小

### 📂 4. 文件管理
```bash
http://localhost:8080/files.html  # 需要登录
```
- 👁️ **文件预览**: 点击文件名在浏览器中打开
- ⬇️ **文件下载**: 点击下载按钮保存到本地
- 🗑️ **文件删除**: 一键删除不需要的文件
- 🔄 **实时刷新**: 自动更新文件列表

## 🔬 技术实现

### 🏗️ 系统架构
```cpp
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   HTTP Client   │───▶│  Web Server     │───▶│  File System    │
│   (Browser)     │    │  (C++ Backend)  │    │  (User Dirs)    │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         │              ┌─────────────────┐              │
         └──────────────▶│  Static Files   │◀─────────────┘
                         │  (HTML/CSS/JS)  │
                         └─────────────────┘
```

### 🧵 多线程并发模型
```cpp
// 主服务器循环 - 接受连接
while (true) {
    int client_socket = accept(server_socket, ...);
    // 为每个客户端创建独立线程
    std::thread client_thread(handle_request, client_socket);
    client_thread.detach();  // 分离线程，自动清理
}

// 线程安全的文件操作
std::mutex file_mutex;
std::lock_guard<std::mutex> lock(file_mutex);
```

### 📨 HTTP 协议处理
- **请求解析**: 完整的 HTTP/1.1 请求头解析
- **响应构建**: 标准 HTTP 响应格式
- **内容协商**: 自动 MIME 类型识别
- **传输编码**: Content-Length 精确控制

### 🔒 安全机制
```cpp
// 路径遍历防护
string safe_path = validate_path(user_input);
if (safe_path.find("../") != string::npos) {
    return FORBIDDEN;
}

// 文件名安全过滤
string secure_filename(const string& filename) {
    string result = filename;
    // 移除危险字符: \ / : * ? " < > |
    // 防止路径注入攻击
    return result;
}
```

## ⚙️ 配置说明

### 🌐 服务器配置
```cpp
// main.cpp 中可修改的配置
const int PORT = 8080;              // 服务器端口
const string ROOT_DIR = "httpdocs"; // 静态文件根目录  
const string UPLOAD_DIR = "upload"; // 文件上传目录
const int MAX_THREADS = 100;        // 最大并发线程数
```

### 📋 支持的文件类型
| 类别 | 扩展名 | MIME 类型 | 浏览器行为 |
|------|--------|-----------|------------|
| 🌐 **网页** | `.html`, `.htm` | `text/html` | 直接显示 |
| 🎨 **样式** | `.css` | `text/css` | 应用样式 |
| ⚡ **脚本** | `.js` | `application/javascript` | 执行代码 |
| 🖼️ **图片** | `.jpg`, `.png`, `.gif` | `image/*` | 图片预览 |
| 📄 **文档** | `.pdf`, `.txt` | `application/pdf`, `text/plain` | 在线阅读 |
| 🎵 **音频** | `.mp3`, `.wav` | `audio/*` | 音频播放 |
| 🎬 **视频** | `.mp4`, `.avi` | `video/*` | 视频播放 |
| 📦 **压缩** | `.zip`, `.rar` | `application/octet-stream` | 强制下载 |

### 🔧 自定义配置
```bash
# 修改端口号
vim main.cpp  # 找到 PORT 常量

# 修改上传限制
vim src/route.cpp  # 修改文件大小检查

# 添加新的 MIME 类型
vim src/utils.cpp  # 在 get_mime_type() 函数中添加
```

## 🛠️ 开发扩展

### 🛤️ 添加新路由
```cpp
// 在 src/route.cpp 的 routeInit() 函数中添加
void routeInit() {
    // GET 路由
    get_routes["/api/new-endpoint"] = your_get_handler;
    
    // POST 路由  
    post_routes["/api/new-action"] = your_post_handler;
    
    // 带权限验证的路由
    get_routes["/protected-page"] = get_page_with_verify;
}

// 实现处理函数
int your_get_handler(HttpMessage& http_message) {
    // 处理 GET 请求
    string response = build_json_response(data);
    send_response(http_message.client_socket, response);
    return 1;
}
```

### 🎨 自定义错误页面
```cpp
// 修改 src/error.cpp
void not_found(int client_socket) {
    string custom_404 = 
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/html; charset=utf-8\r\n\r\n"
        "<html><body><h1>🔍 页面未找到</h1></body></html>";
    send(client_socket, custom_404.c_str(), custom_404.length(), 0);
}
```

### 📊 添加新的 MIME 类型
```cpp
// 在 src/utils.cpp 的 get_mime_type() 函数中添加
string get_mime_type(const string& path) {
    string ext = get_file_extension(path);
    
    if (ext == ".json") return "application/json";
    if (ext == ".xml")  return "application/xml";
    if (ext == ".svg")  return "image/svg+xml";
    // ... 添加更多类型
    
    return "application/octet-stream";  // 默认类型
}
```

### 🗄️ 数据库集成示例
```cpp
// 在 src/sql.cpp 中实现数据库操作
class DatabaseManager {
public:
    bool createUser(const string& username, const string& password);
    bool authenticateUser(const string& username, const string& password);
    vector<FileInfo> getUserFiles(const string& username);
    
private:
    sqlite3* db;  // 或者 MySQL 连接
};
```

## 🚧 开发路线图

### 🎯 当前版本 (v1.0)
- ✅ 基础 HTTP 服务器实现
- ✅ 多线程并发处理
- ✅ 用户认证系统
- ✅ 分用户文件管理
- ✅ 现代化 Web 界面

### 🔮 下一版本 (v2.0)
- 🔲 **MySQL 数据库集成**
  - 用户信息持久化存储
  - 文件元数据管理
  - 操作日志记录

- 🔲 **高级文件系统**
  - 文件夹层次管理
  - 批量文件操作
  - 文件重命名功能
  - 文件分享链接

- 🔲 **并发安全优化**
  - 文件读写锁机制
  - 线程池管理优化
  - 内存使用优化

### 🌟 长期规划 (v3.0+)
- 🔲 **HTTPS 安全连接**
- 🔲 **WebSocket 实时通信**
- 🔲 **Redis 缓存系统**
- 🔲 **Docker 容器化部署**
- 🔲 **RESTful API 文档**
- 🔲 **单元测试覆盖**

## 🤝 贡献指南

### 📝 提交代码
1. Fork 项目到你的 GitHub
2. 创建特性分支: `git checkout -b feature-name`
3. 提交更改: `git commit -m 'Add some feature'`
4. 推送分支: `git push origin feature-name`
5. 提交 Pull Request

### 🐛 报告问题
- 使用 GitHub Issues 报告 Bug
- 提供详细的复现步骤
- 包含系统环境信息

### 💡 功能建议
- 在 Issues 中描述新功能需求
- 说明使用场景和预期效果
- 欢迎提供设计方案

## 📄 许可证

本项目采用 [MIT 许可证](LICENSE) - 详见 LICENSE 文件

## 👨‍💻 作者信息

**wwwTwilight**
- 🐙 GitHub: [@wwwTwilight](https://github.com/wwwTwilight)  
- 📧 Email: 通过 GitHub 联系
- 🌐 项目: [myCppWebServer](https://github.com/wwwTwilight/myCppWebServer)

## ⭐ 致谢

感谢所有为这个项目做出贡献的开发者！如果这个项目对你有帮助，请给个 ⭐ Star 支持一下！

---

<div align="center">

**🌟 MyPoorWebServerInCpp - 从简陋到强大的 C++ Web 服务器之路 🌟**

Made with ❤️ by wwwTwilight

</div>
