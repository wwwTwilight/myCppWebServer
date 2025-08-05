#include "../include/utils.h"
#include "../include/error.h"
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <cstdio>
#include <ctime>
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <cstdio>
#include <ctime>
#include <fstream>

// 发送HTTP1.0请求头
void header(int client_socket) {
	vector<char> buffer(1024);

	snprintf(buffer.data(), buffer.size(), "HTTP/1.0 200 OK\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);

	snprintf(buffer.data(), buffer.size(), "Server: MyPoorWebServer\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);

	snprintf(buffer.data(), buffer.size(), "Content-Type: text/html\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);

	snprintf(buffer.data(), buffer.size(), "charset=UTF-8\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);

	snprintf(buffer.data(), buffer.size(), "\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);
}

// 获取一行HTTP报文，存放在buffer中，返回读取的字符数
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

// 获取当前时间的时间戳
int getTime() {
    return time(nullptr);
}

// 获取文件的MIME类型，用于发送正确的Content-Type头
string get_mime_type(const string& filename) {
	string ext;
	size_t pos = filename.find_last_of('.');
	if (pos != string::npos) {
		ext = filename.substr(pos);
		// 统一转换为小写
        for (char& c : ext) {
            c = tolower(c);
        }
	}
	if (ext == ".html" || ext == ".htm") return "text/html";
    else if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    else if (ext == ".png") return "image/png";
    else if (ext == ".gif") return "image/gif";
    else if (ext == ".pdf") return "application/pdf";
    else if (ext == ".doc") return "application/msword";
    else if (ext == ".docx") return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    else if (ext == ".zip") return "application/zip";
    else if (ext == ".txt") return "text/plain; charset=UTF-8";
    else if (ext == ".mp4") return "video/mp4";
    else if (ext == ".mp3") return "audio/mpeg";
    else if (ext == ".css") return "text/css";
    else if (ext == ".js") return "application/javascript";
    else return "application/octet-stream"; // 默认二进制下载
}

// 打开文件并发送内容，无请求行
void open_http_file(int& client_socket, const string& filename) {
    string fullpath = "httpdocs" + filename;
    ifstream ifile(fullpath.data(), ios::binary);
    vector<char> buffer(4096);
    if (!ifile) {
        not_found(client_socket);
        return;
    }
    ifile.seekg(0, ios::end);
    size_t filesize = ifile.tellg();
    ifile.seekg(0, ios::beg);

	string mime_type = get_mime_type(fullpath);

    // 发送带 Content-Length 的 header，1.1协议特色
    vector<char> headbuf(256);
    snprintf(headbuf.data(), headbuf.size(),
		"Server: MyPoorWebServer\r\n"
		"Content-Type: %s; charset=UTF-8\r\n"
		"Content-Length: %zu\r\n\r\n", mime_type.data(), filesize);
    send(client_socket, headbuf.data(), strlen(headbuf.data()), 0);

    while (!ifile.eof()) {
        ifile.read(buffer.data(), buffer.size());
        streamsize count = ifile.gcount();
        if (count > 0) {
            send(client_socket, buffer.data(), count, 0);
        }
    }
}

string file_name_secure(const string& filename) {
    string secure_name = filename;
    // 替换文件名中的不安全字符
    for (char& c : secure_name) {
        if (c == '/') c = '_';
    }
    size_t pos = secure_name.find_last_of('.');
    string name = secure_name.substr(0, pos);
    string ext = secure_name.substr(pos);
    return name + "__" + to_string(getTime()) + ext;
}