// 写在前面，虽然是使用cpp重写，但是并没有使用面向对象的思想，毕竟这一份主要在于理解webserver在cpp中的实现，后面会使用面向对象的思想来重写
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstdint>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <cctype>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
using namespace std;

void error_exit(const string& message) {
    cerr << message << endl;
    exit(0);
}

void method_not_supported(int client_socket) {
    vector<char> buffer(1024);
	sprintf(buffer.data(), "HTTP/1.0 501 Not Implemented\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);
	sprintf(buffer.data(), "Server: MyPoorWebServer\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);
	sprintf(buffer.data(), "Content-Type: text/html\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);
	sprintf(buffer.data(), "\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);
	sprintf(buffer.data(), "<html><hea><title>Method Not Implemented</title></head>\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);
	sprintf(buffer.data(), "<body><p>HTTP request method not supported.</p></body></html>\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);
	cout << "client: " << client_socket << " 501 Method Not Implemented" << endl;
}

// 获取一行HTTP报文
int getHttpLine(int client_socket, string& buffer) {
    buffer.clear();
    char c = '\0';
    int n = 0;
    while (c != '\n') {
        n = recv(client_socket, &c, 1, 0);
        if (n > 0) {
            if (c == '\r') {
                char next;
                n = recv(client_socket, &next, 1, MSG_PEEK);
                if (n > 0 && next == '\n') {
                    recv(client_socket, &c, 1, 0);
                } else {
                    c = '\n';
                }
            }
            buffer += c;
        } else {
            c = '\n';
        }
    }
    return buffer.size();
}

// 发送HTTP1.0请求头
void header(int client_socket) {
	vector<char> buffer(1024);
	sprintf(buffer.data(), "HTTP/1.0 200 OK\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);

	sprintf(buffer.data(), "Server: MyPoorWebServer\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);

	sprintf(buffer.data(), "Content-Type: text/html\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);

	sprintf(buffer.data(), "\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);
}

void not_found(int client_socket) {
    vector<char> buffer(1024);
	sprintf(buffer.data(), "HTTP/1.0 404 Not Found\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);
	sprintf(buffer.data(), "Server: MyPoorWebServer\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);
	sprintf(buffer.data(), "Content-Type: text/html\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);
	sprintf(buffer.data(), "\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);
	sprintf(buffer.data(), "<html><head><title>Not Found</title></head>\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);
	sprintf(buffer.data(), "<body><p>HTTP request file not found.</p></body></html>\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);

	cout << "client: " << client_socket << " 404 Not Found" << endl;
}

// 打开文件并发送内容，自带请求头
void open_file(int& client_socket, const string& filename) {
    string fullpath = "httpdocs" + filename;
    ifstream ifile(fullpath.data(), ios::binary);
    vector<char> buffer(1024);
    if (!ifile) {
        not_found(client_socket);
        return;
    }
    ifile.seekg(0, ios::end);
    size_t filesize = ifile.tellg();
    ifile.seekg(0, ios::beg);

    // 发送带 Content-Length 的 header，1.1协议特色
    vector<char> headbuf(256);
    snprintf(headbuf.data(), headbuf.size(),
        "HTTP/1.1 200 OK\r\nServer: MyPoorWebServer\r\nContent-Type: text/html\r\nContent-Length: %zu\r\n\r\n", filesize);
    send(client_socket, headbuf.data(), strlen(headbuf.data()), 0);

    while (ifile) {
        ifile.read(buffer.data(), buffer.size());
        streamsize count = ifile.gcount();
        if (count > 0) {
            send(client_socket, buffer.data(), count, 0);
        }
    }
}

void* accept_request(int client_socket) {
    string buffer;
    string method, url, query, path;
    int cgi = 0;

    getHttpLine(client_socket, buffer); // 获取第一行HTTP报文

    // 获取请求方法
    size_t j = 0;
    while (j < buffer.size() && !isspace(buffer[j])) {
        method += buffer[j++];
    }

    if (!(strcasecmp(method.data(), "GET") == 0 || strcasecmp(method.data(), "POST") == 0)) {
        method_not_supported(client_socket);
        close(client_socket);
        return NULL;
    }

    if (strcasecmp(method.data(), "POST") == 0) cgi = 1;

    // 跳过空白字符
    while (j < buffer.size() && isspace(buffer[j])) {
        j++;
    }

    // 获取url
    while (j < buffer.size() && !isspace(buffer[j])) {
        url += buffer[j++];
    }

    // 如果是GET请求，获取query
    if (strcasecmp(method.data(), "GET") == 0) {
        size_t pos = url.find('?');
        if (pos != string::npos && pos + 1 < url.size()) {
			cgi = 1;
            query = url.substr(pos + 1);
            path = url.substr(0, pos);
        }
		else {
            path = url;
		}
    }

	cout << "method: " << method << " url: " << url << " query: " << query;

	if (path[path.size() - 1] == '/') // 判断请求的是不是目录，如果是，就拼接上默认的文件名
	{
		path += "index.html";
	}

	cout << " path: " << path << endl;

	if(!cgi) {
		open_file(client_socket, path);
	}

    close(client_socket);
	cout << "connection close....client: " << client_socket << endl;
    return NULL;
}

int startSever(unsigned short& port) {
    if (port == 0) {
        port = 8080;
    }
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in name;
    socklen_t name_len = sizeof(name);
    memset(&name, 0, sizeof(name));
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        error_exit("setsockopt error");
    }

    if (bind(server_socket, (sockaddr*)&name, name_len) < 0) {
        error_exit("bind error");
    }

    if (listen(server_socket, 10) < 0) {
        error_exit("listen error");
    }

    cout << "server start on socket " << server_socket << endl;
    cout << "server start on address " << inet_ntoa(name.sin_addr) << endl;
    cout << "server start on port " << port << endl;

    return server_socket;
}

int main() {
    int server_socket = -1;
    unsigned short port = 8080;
    int client_socket = -1;
    sockaddr_in client_name;
    socklen_t client_name_len = sizeof(client_name);

    server_socket = startSever(port);

    while (1) {
        client_socket = accept(server_socket, (sockaddr*)&client_name, &client_name_len);

        cout << "New connection from ip:" << inet_ntoa(client_name.sin_addr) << " and port:" << ntohs(client_name.sin_port) << endl;
        if (client_socket < 0) {
            error_exit("accept error");
        }
        thread new_thread(accept_request, client_socket);
        new_thread.detach();
    }

    return 0;
}