#include "../include/httpMes.h"
#include "../include/utils.h"
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <cerrno>
#include <unistd.h>

// 处理请求头
void HttpMessage::requestHead() {
    string buffer;
    int n = 1;
    while (n > 0) {
        n = getHttpLine(client_socket, buffer);

        // 要把空行读了，这之后的所有内容都是请求体相关的内容
        if (buffer == "\r\n" || buffer == "\n" || buffer.length() <= 2) {
            break;
        }

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
    cout << "\n=== READING REQUEST BODY ===" << endl;
    
    if (strcasecmp(method.data(), "POST") != 0) {
        cout << "Not a POST request, skipping body read" << endl;
        return;
    }
    
    if (headers.find("Content-Length") == headers.end()) {
        cout << "No Content-Length header found" << endl;
        return;
    }
    
    int content_length = stoi(headers["Content-Length"]);
    cout << "Expected Content-Length: " << content_length << " bytes" << endl;
    
    if (content_length <= 0) {
        cout << "Invalid Content-Length: " << content_length << endl;
        return;
    }
    
    body.clear();
    body.reserve(content_length + 100); // 预留一些额外空间
    
    int total_received = 0;
    char buffer[4096]; // 增大缓冲区
    int consecutive_failures = 0; // 连续失败计数
    
    cout << "Starting to read body data..." << endl;
    
    while (total_received < content_length) {
        int remaining = content_length - total_received;
        int to_read = min(remaining, (int)sizeof(buffer));
        
        cout << "Attempting to read " << to_read << " bytes (progress: " 
             << total_received << "/" << content_length << ")" << endl;
        
        int bytes_received = recv(client_socket, buffer, to_read, 0);
        
        if (bytes_received > 0) {
            body.append(buffer, bytes_received);
            total_received += bytes_received;
            consecutive_failures = 0; // 重置失败计数
            cout << "Successfully read " << bytes_received << " bytes" << endl;
        } else if (bytes_received == 0) {
            cout << "Connection closed by client" << endl;
            break;
        } else {
            consecutive_failures++;
            cout << "recv() failed, error: " << strerror(errno) 
                 << " (attempt " << consecutive_failures << ")" << endl;
            
            if (consecutive_failures >= 3) {
                cout << "Too many consecutive failures, giving up" << endl;
                break;
            }

        }
    }
    
    cout << "Body read completed: " << total_received << "/" << content_length 
         << " bytes (success: " << (total_received == content_length ? "YES" : "NO") << ")" << endl;
    cout << "Actual body size: " << body.size() << " bytes" << endl;
    cout << "=== REQUEST BODY READ END ===\n" << endl;
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