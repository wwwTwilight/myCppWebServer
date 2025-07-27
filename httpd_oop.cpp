/*
 * 面向对象结构的简易Web服务器
 * 仅用于演示OOP结构，功能与原httpd.cpp一致
 * - 支持静态文件服务和CGI脚本执行
 * - 多线程处理客户端请求
 * - HTTP/1.1协议支持
 * - 代码结构清晰，易于扩展
 */
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <thread>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
using namespace std;

// ==================== HTTP请求类 ====================
class HttpRequest {
public:
    string method, url, path, query;
    int content_length = -1;
    bool is_cgi = false;
    string body;
    map<string, string> headers;

    bool parse(int client_socket) {
        string line;
        if (!read_line(client_socket, line)) return false;
        size_t pos = url.find('?');
        if (pos != string::npos) {
            query = url.substr(pos + 1);
            path = url.substr(0, pos);
        } else {
            path = url;
        }
        if (path[path.size() - 1] == '/') path += "index.html";
        parse_headers(client_socket);
        if (strcasecmp(method.data(), "POST") == 0) {
            is_cgi = true;
            if (headers.count("Content-Length")) {
                content_length = stoi(headers["Content-Length"]);
                body.resize(content_length);
                int total = 0;
                while (total < content_length) {
                    int n = recv(client_socket, &body[total], content_length-total, 0);
                    if (n <= 0) break;
                    total += n;
                }
            }
        } else if (!query.empty()) {
            is_cgi = true;
        }
        return true;
    }
private:
    bool read_line(int socket, string& line) {
        line.clear(); char c = '\0'; int n = 0;
        while (c != '\n') {
            n = recv(socket, &c, 1, 0);
            if (n > 0) {
                if (c == '\r') {
                    char next;
                    n = recv(socket, &next, 1, MSG_PEEK);
                    if (n > 0 && next == '\n') recv(socket, &c, 1, 0);
                    else c = '\n';
                }
                line += c;
            } else c = '\n';
        }
        return !line.empty();
    }
    void parse_headers(int socket) {
        string line;
        while (read_line(socket, line) && line != "\n") {
            size_t pos = line.find(':');
            if (pos != string::npos) {
                string name = line.substr(0, pos);
                string value = line.substr(pos + 1);
                name.erase(0, name.find_first_not_of(" \t"));
                name.erase(name.find_last_not_of(" \t\r\n") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
                headers[name] = value;
            }
        }
    }
};

// ==================== MIME类型管理器 ====================
class MimeTypeManager {
    map<string, string> mime_types;
public:
    MimeTypeManager() {
        mime_types[".html"] = "text/html; charset=UTF-8";
        mime_types[".htm"] = "text/html; charset=UTF-8";
        mime_types[".txt"] = "text/plain; charset=UTF-8";
        mime_types[".css"] = "text/css";
        mime_types[".js"] = "application/javascript";
        mime_types[".jpg"] = "image/jpeg";
        mime_types[".jpeg"] = "image/jpeg";
        mime_types[".png"] = "image/png";
        mime_types[".gif"] = "image/gif";
        mime_types[".pdf"] = "application/pdf";
        mime_types[".doc"] = "application/msword";
        mime_types[".docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
        mime_types[".zip"] = "application/zip";
        mime_types[".mp4"] = "video/mp4";
        mime_types[".mp3"] = "audio/mpeg";
    }
    string get(const string& filename) const {
        size_t pos = filename.find_last_of('.');
        if (pos == string::npos) return "application/octet-stream";
        string ext = filename.substr(pos);
        string ext_lower = ext;
        transform(ext_lower.begin(), ext_lower.end(), ext_lower.begin(), ::tolower);
        auto it = mime_types.find(ext_lower);
        return it != mime_types.end() ? it->second : "application/octet-stream";
    }
};

// ==================== CGI处理器 ====================
class CgiHandler {
    string doc_root;
public:
    CgiHandler(const string& root): doc_root(root) {}
    bool handle(const HttpRequest& req, int client_socket) {
        string script_path = doc_root + req.path;
        int cgi_input[2], cgi_output[2];
        if (pipe(cgi_input) < 0 || pipe(cgi_output) < 0) return false;
        pid_t pid = fork();
        if (pid < 0) return false;
        if (pid == 0) {
            dup2(cgi_input[0], STDIN_FILENO);
            dup2(cgi_output[1], STDOUT_FILENO);
            close(cgi_input[1]); close(cgi_output[0]);
            setenv("REQUEST_METHOD", req.method.c_str(), 1);
            if (req.method == "GET") setenv("QUERY_STRING", req.query.c_str(), 1);
            else if (req.method == "POST") {
                setenv("CONTENT_LENGTH", to_string(req.content_length).c_str(), 1);
                setenv("CONTENT_TYPE", "application/x-www-form-urlencoded", 1);
            }
            execl(script_path.c_str(), script_path.c_str(), nullptr);
            exit(1);
        } else {
            close(cgi_input[0]); close(cgi_output[1]);
            if (req.method == "POST" && req.content_length > 0) {
                write(cgi_input[1], req.body.c_str(), req.body.size());
            }
            close(cgi_input[1]);
            // 先发送HTTP状态行
            string status_line = "HTTP/1.1 200 OK\r\n";
            send(client_socket, status_line.c_str(), status_line.size(), 0);
            // 再转发CGI输出
            char c;
            while (read(cgi_output[0], &c, 1) > 0) {
                send(client_socket, &c, 1, 0);
            }
            close(cgi_output[0]);
            waitpid(pid, nullptr, 0);
        }
        return true;
    }
};

// ==================== 静态文件处理器 ====================
class StaticFileHandler {
    string doc_root;
    MimeTypeManager mime_mgr;
public:
    StaticFileHandler(const string& root): doc_root(root) {}
    bool handle(const HttpRequest& req, int client_socket) {
        string file_path = doc_root + req.path;
        ifstream file(file_path, ios::binary);
        if (!file.is_open()) {
            send_404(client_socket);
            return false;
        }
        file.seekg(0, ios::end);
        size_t file_size = file.tellg();
        file.seekg(0, ios::beg);
        string mime_type = mime_mgr.get(file_path);
        string header = "HTTP/1.1 200 OK\r\nServer: MyPoorWebServer\r\nContent-Type: " + mime_type + "\r\nContent-Length: " + to_string(file_size) + "\r\n\r\n";
        send(client_socket, header.c_str(), header.size(), 0);
        char buf[4096];
        while (!file.eof()) {
            file.read(buf, sizeof(buf));
            streamsize n = file.gcount();
            if (n > 0) send(client_socket, buf, n, 0);
        }
        return true;
    }
    void send_404(int client_socket) {
        string msg = "HTTP/1.1 404 Not Found\r\nServer: MyPoorWebServer\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n<html><head><title>Not Found</title></head><body><p>HTTP request file not found.</p></body></html>\r\n";
        send(client_socket, msg.c_str(), msg.size(), 0);
    }
};

// ==================== Web服务器主类 ====================
class WebServer {
    int server_socket;
    unsigned short port;
    string doc_root;
    StaticFileHandler static_handler;
    CgiHandler cgi_handler;
public:
    WebServer(unsigned short p = 8080, const string& root = "httpdocs")
        : port(p), doc_root(root), static_handler(root), cgi_handler(root) {}
    bool start() {
        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket < 0) return false;
        int opt = 1;
        setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;
        if (::bind(server_socket, (sockaddr*)&addr, sizeof(addr)) < 0) return false;
        if (listen(server_socket, 10) < 0) return false;
        cout << "服务器启动成功，端口:" << port << " 根目录:" << doc_root << endl;
        return true;
    }
    void run() {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        while (true) {
            int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_len);
            if (client_socket < 0) continue;
            thread(&WebServer::handle_client, this, client_socket).detach();
        }
    }
private:
    void handle_client(int client_socket) {
        HttpRequest req;
        if (!req.parse(client_socket)) {
            static_handler.send_404(client_socket);
            close(client_socket);
            return;
        }
        cout << "method: " << req.method << " url: " << req.url << " query: " << req.query << " path: " << req.path << endl;
        bool ok = false;
        if (req.is_cgi) ok = cgi_handler.handle(req, client_socket);
        else ok = static_handler.handle(req, client_socket);
        shutdown(client_socket, SHUT_WR);
        close(client_socket);
        cout << "connection close....client: " << client_socket << endl;
    }
};

// ==================== 主函数 ====================
int main() {
    WebServer server(8080, "httpdocs");
    if (!server.start()) {
        cerr << "服务器启动失败" << endl;
        return 1;
    }
    server.run();
    return 0;
}
