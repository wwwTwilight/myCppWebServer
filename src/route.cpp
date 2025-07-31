#include "../include/route.h"
#include "../include/utils.h"
#include "../include/error.h"
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <cstdio>
#include <sys/socket.h>

using namespace std;

route::route() {
    get_routes["/index.html"] = get_page;
    get_routes["/contact.html"] = get_page;
    get_routes["/about.html"] = get_page;
    get_routes["/post.html"] = get_page;
}

void route::routeWork(HttpMessage& http_message) {
    // 处理GET请求
    if (http_message.method == "GET") {
        
    }

    // 处理POST请求
    if (http_message.method == "POST") {
        
    }
}

int get_page(HttpMessage& http_message) {
    int client_socket = http_message.client_socket;
    string fullpath = "httpdocs" + http_message.path;
    ifstream ifile(fullpath.data(), ios::binary);
    vector<char> buffer(4096);
    if (!ifile) {
        not_found(client_socket);
        return;
    }
    ifile.seekg(0, ios::end);
    size_t filesize = ifile.tellg();
    ifile.seekg(0, ios::beg);

	string mime_type = get_mime_type(fullpath);

    // 发送带 Content-Length 的 header，1.1协议特色
    vector<char> headbuf(512);
    snprintf(headbuf.data(), headbuf.size(),
        "HTTP/1.1 200 OK\r\n"
		"Server: MyPoorWebServer\r\n"
		"Content-Type: %s\r\n"
		"Content-Length: %zu\r\n\r\n", mime_type.data(), filesize);
    send(client_socket, headbuf.data(), strlen(headbuf.data()), 0);

    while (!ifile.eof()) {
        ifile.read(buffer.data(), buffer.size());
        streamsize count = ifile.gcount();
        if (count > 0) {
            send(client_socket, buffer.data(), count, 0);
        }
    }
    return 0;
}