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

// 执行CGI脚本
void exec_cgi(const HttpMessage& http_message) {
	int client_socket = http_message.client_socket;
	string buffer;

	string path = "httpdocs" + http_message.path;

	int cgi_input[2];
	int cgi_output[2];

	int content_length = -1;
	if (pipe(cgi_input) < 0 || pipe(cgi_output) < 0) {
		error_message("pipe error");
		return;
	}

	if(strcasecmp(http_message.method.data(), "GET") == 0) {
        // 处理GET请求
	}
	else {
		for(auto & header : http_message.headers) {
			if(strcasecmp(header.first.data(), "Content-Length") == 0) {
				content_length = atoi(header.second.data());
				break;
			}
		}

		if (content_length < 0) {
			error_message("Content-Length not found");
			return;
		}
	}

	vector<char> headbuf(256);
	snprintf(headbuf.data(), headbuf.size(), "HTTP/1.1 200 OK\r\n");
	send(client_socket, headbuf.data(), strlen(headbuf.data()), 0);

	int pid = fork();
	if (pid < 0) {
		error_message("fork error");
		return;
	}
	if(pid == 0) {
		dup2(cgi_input[0], STDIN_FILENO);
		dup2(cgi_output[1], STDOUT_FILENO);
		close(cgi_input[1]);
		close(cgi_output[0]);

		static string method_env = "REQUEST_METHOD=" + http_message.method;
		putenv(method_env.data());

		if(strcasecmp(http_message.method.data(), "GET") == 0) {
			static string query_env = "QUERY_STRING=" + http_message.query;
			putenv(query_env.data());
		}
		else {
			static string content_length_env = "CONTENT_LENGTH=" + to_string(content_length);
			putenv(content_length_env.data());
		}

		if(execl(path.data(), path.data(), NULL) < 0) {
			error_message("execl error");
			close(cgi_input[0]);
			close(cgi_output[1]);

			exit(1);
		}
		close(cgi_input[0]);
		close(cgi_output[1]);

		exit(0);
	}
	else {
		close(cgi_input[0]);
		close(cgi_output[1]);

		if (strcasecmp(http_message.method.data(), "POST") == 0) {
			for(int i = 0; i < content_length; i++) {
				char c = http_message.body[i];
				write(cgi_input[1], &c, 1);
				
			}
		}

		while(read(cgi_output[0], buffer.data(), 1) > 0) {
			send(client_socket, buffer.data(), 1, 0);
		}

		close(cgi_input[1]);
		close(cgi_output[0]);

		waitpid(pid, NULL, 0);
	}
}

// 打开文件并发送内容，自带请求头
void open_http_file(const HttpMessage& http_message) {
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
}

void* accept_request(int client_socket) {
    string buffer;
    string method, url, query, path;
    int cgi = 0;

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

    if (strcasecmp(method.data(), "POST") == 0) cgi = 1;

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
			cgi = 1;
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

	cgi = cgi;

    routeWork(http_message);

	// if(!cgi) {
	// 	open_http_file(http_message);
	// }
	// else {
	//     exec_cgi(http_message);
	// }

	shutdown(client_socket, SHUT_WR); // 告诉客户端数据已发完
    close(client_socket);
	cout << "connection close....client: " << client_socket << endl;
    return NULL;
}