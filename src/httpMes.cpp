#include "../include/httpMes.h"
#include "../include/utils.h"
#include <sys/socket.h>
#include <iostream>
#include <cstring>

// 处理请求头
void HttpMessage::requestHead() {
    string buffer;
    int n = 1;
    while (n > 0 && strcmp(buffer.data(), "\n")) {
        n = getHttpLine(client_socket, buffer);
        size_t pos = buffer.find(':');
        if (pos != string::npos) {
            string key = buffer.substr(0, pos);
            string value = buffer.substr(pos + 1);
            headers[key] = value;
        }
    }
    cookie = new Cookie(headers["Cookie"]);
}

// 处理请求体
void HttpMessage::requestBody() {
    if (strcasecmp(method.data(), "POST") == 0) {
        string content_length_str = headers["Content-Length"];
        if (!content_length_str.empty()) {
            int content_length = stoi(content_length_str);
            body.resize(content_length);
            recv(client_socket, body.data(), content_length, 0);
        } else {
            cout << "Content-Length not found" << endl;
        }
    }
}
HttpMessage::HttpMessage(int client_socket, string method, string url, string query, string path) {
    this->client_socket = client_socket;
    this->method = method;
    this->url = url;
    this->query = query;
    this->path = path;
    this->cookie = nullptr;
    requestHead(); // 处理请求头
    requestBody(); // 处理请求体
}