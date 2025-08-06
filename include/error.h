// http错误处理相关的内容
#ifndef ERROR_H
#define ERROR_H

#include <iostream>
#include <string>

using namespace std;

// 发送方法不支持的信息
void method_not_supported(int client_socket);
// 错误信息报告
void error_message(const string& message);
// 发送404 Not Found响应
void not_found(int client_socket);
// 不允许访问的响应
void forbidden(int client_socket);

#endif // ERROR_H