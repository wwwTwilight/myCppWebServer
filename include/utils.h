# ifndef UTILS_H
#define UTILS_H

#include <string>
#include <ctime>
#include "httpMes.h"

using namespace std;

struct Content_Disposition {
    string name; // 名称
    string filename; // 文件名
};

// 发送HTTP1.0请求头
void header(int client_socket);

// 获取一行HTTP报文，存放在buffer中，返回读取的字符数
int getHttpLine(int client_socket, string& buffer);

int getTime();
// 获取文件的MIME类型，用于发送正确的Content-Type头
string get_mime_type(const string& filename);

// 发送HTTP文件内容，自带请求头
void open_http_file(int& client_socket, const string& filename);

// 将文件名转换为安全的格式，避免路径穿越等问题，同时将时间戳作为结尾，确保唯一性
string file_name_secure(const string& filename);

string executeCommand(const string& command);

string urlDecode(const string& encoded);

int verifyUser(HttpMessage& http_message);

#endif // UTILS_H