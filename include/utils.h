# ifndef UTILS_H
#define UTILS_H

#include <string>
#include <ctime>

using namespace std;

// 发送HTTP1.0请求头
void header(int client_socket);

// 获取一行HTTP报文，存放在buffer中，返回读取的字符数
int getHttpLine(int client_socket, string& buffer);

int getTime();
// 获取文件的MIME类型，用于发送正确的Content-Type头
string get_mime_type(const string& filename);

#endif // UTILS_H