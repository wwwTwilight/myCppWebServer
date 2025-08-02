#include "../include/route.h"
#include "../include/utils.h"
#include "../include/error.h"
#include "../include/httpMes.h"
#include "../include/cookie.h"
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <cstdio>
#include <sys/socket.h>
#include <sys/wait.h>

using namespace std;

unordered_map<string, function<int(HttpMessage&)>> get_routes;
unordered_map<string, function<int(HttpMessage&)>> post_routes;

void routeInit() {
    get_routes["/index.html"] = get_page;
    get_routes["/contact.html"] = get_page;
    get_routes["/about.html"] = get_page;
    get_routes["/post.html"] = get_page;
    // get_routes["/login_success.html"] = get_page;

    post_routes["/post.cgi"] = login_page;
}

int routeWork(HttpMessage& http_message) {
    if (http_message.method == "GET") {
        getRoute(http_message);
        return 1;
    }
    else if (http_message.method == "POST") {
        postRoute(http_message);
        return 1;
    }
    else {
        getRoute(http_message);
        return 0;
    }
}

int getRoute(HttpMessage& http_message) {
    if(get_routes.contains(http_message.path)) {
        get_routes[http_message.path](http_message);
        return 1;
    } else {
        not_found(http_message.client_socket);
        return 0;
    }
}

int postRoute(HttpMessage& http_message) {
    if(post_routes.contains(http_message.path)) {
        post_routes[http_message.path](http_message);
        return 1;
    } else {
        not_found(http_message.client_socket);
        return 0;
    }
}

// 泛用get
int get_page(HttpMessage& http_message) {
    int client_socket = http_message.client_socket;
    string fullpath = "httpdocs" + http_message.path;
    ifstream ifile(fullpath.data(), ios::binary);
    vector<char> buffer(4096);
    if (!ifile) {
        not_found(client_socket);
        return 0;
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

// 泛用post
int post_page(HttpMessage& http_message) {
    int client_socket = http_message.client_socket;
	string buffer;

	string path = "httpdocs" + http_message.path;

	int cgi_input[2];
	int cgi_output[2];

	int content_length = -1;
	if (pipe(cgi_input) < 0 || pipe(cgi_output) < 0) {
		error_message("pipe error");
		return 0;
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
			return 0;
		}
	}

	vector<char> headbuf(256);
	snprintf(headbuf.data(), headbuf.size(), "HTTP/1.1 200 OK\r\n");
	send(client_socket, headbuf.data(), strlen(headbuf.data()), 0);

	int pid = fork();
	if (pid < 0) {
		error_message("fork error");
		return 0;
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

    return 0;
}

int login_page(HttpMessage& http_message) {
    int client_socket = http_message.client_socket;
	// string buffer;

	// string path = "httpdocs" + http_message.path;

    int content_length = -1;

    for(auto & header : http_message.headers) {
        if(strcasecmp(header.first.data(), "Content-Length") == 0) {
            content_length = atoi(header.second.data());
            break;
        }
    }

    if (content_length < 0) {
        error_message("Content-Length not found");
        return 0;
    }

    if(http_message.body.find("username=admin") != string::npos &&
       http_message.body.find("password=123456") != string::npos) {
        // 登录成功
        vector<char> headbuf(256);
        snprintf(headbuf.data(), headbuf.size(), "HTTP/1.1 200 OK\r\n");
        send(client_socket, headbuf.data(), strlen(headbuf.data()), 0);

        setCookie set_cookie;
        send(client_socket, set_cookie.getCookie().data(), set_cookie.getCookie().size(), 0);

        open_http_file(client_socket, "/login_success.html");

        return 1;
    } else {
        // 登录失败
        vector<char> headbuf(256);
        snprintf(headbuf.data(), headbuf.size(), "HTTP/1.1 401 Unauthorized\r\n");
        send(client_socket, headbuf.data(), strlen(headbuf.data()), 0);
        return 0;
    }
}