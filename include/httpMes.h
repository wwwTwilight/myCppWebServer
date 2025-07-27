# ifndef HTTP_MES_H
# define HTTP_MES_H

#include <string>
#include <map>
#include <vector>
#include <unordered_map>

using namespace std;

class HttpMessage {
public:
    int client_socket; // 客户端套接字
    string method, url, query, path; // 请求方法、URL、查询字符串和路径
    unordered_map<string, string> headers; // 请求头
    string body; // 请求体

    HttpMessage(int client_socket, string method, string url, string query, string path);

    void requestHead();

    void requestBody();
};

#endif // HTTP_MES_H