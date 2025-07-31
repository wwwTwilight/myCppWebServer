#include "../include/utils.h"
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <cstdio>
#include <ctime>

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
	if (ext == ".html" || ext == ".htm") return "text/html; charset=UTF-8";
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