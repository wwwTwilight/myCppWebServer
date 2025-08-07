#include "../include/requestHandler.h"
#include "../include/utils.h"
#include "../include/webserverSet.h"
#include "../include/error.h"
#include "../include/httpMes.h"
#include "../include/route.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cctype>
#include <cstdio>
#include <cstdlib>

using namespace std;

void* accept_request(int client_socket) {
    string buffer;
    string method, url, query, path;

    getHttpLine(client_socket, buffer); // 获取第一行HTTP报文

    // 获取请求方法
    size_t j = 0;
    while (j < buffer.size() && !isspace(buffer[j])) {
        method += buffer[j++];
    }

    if (!(strcasecmp(method.data(), "GET") == 0 || strcasecmp(method.data(), "POST") == 0)) {
        method_not_supported(client_socket);
        close(client_socket);
        return NULL;
    }

    // 跳过空白字符
    while (j < buffer.size() && isspace(buffer[j])) {
        j++;
    }

    // 获取url
    while (j < buffer.size() && !isspace(buffer[j])) {
        url += buffer[j++];
    }

    // 如果是GET请求，获取query
    if (strcasecmp(method.data(), "GET") == 0) {
        size_t pos = url.find('?');
        if (pos != string::npos && pos + 1 < url.size()) {
            query = url.substr(pos + 1);
            path = url.substr(0, pos);
        }
		else {
			path = url;
		}
    }
	else if(strcasecmp(method.data(), "POST") == 0) {
		path = url; // POST请求直接使用url作为路径
	}

	if (path[path.size() - 1] == '/') // 判断请求的是不是目录，如果是，就拼接上默认的文件名
	{
		path += "index.html";
	}

	HttpMessage http_message(client_socket, method, url, query, path);
	cout << "method: " << method << " url: " << url << " query: " << query << " path: " << path << endl;

    routeWork(http_message);

	shutdown(client_socket, SHUT_WR); // 告诉客户端数据已发完
    close(client_socket);
	cout << "connection close....client: " << client_socket << endl;
    return NULL;
}