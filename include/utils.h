# ifndef UTILS_H
#define UTILS_H

#include <string>

using namespace std;

// 发送HTTP1.0请求头
void header(int client_socket);

// 获取一行HTTP报文，存放在buffer中，返回读取的字符数
int getHttpLine(int client_socket, string& buffer);

#endif // UTILS_H