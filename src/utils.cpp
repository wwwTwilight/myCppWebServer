#include "../include/utils.h"
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include <iostream>
#include <vector>

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
