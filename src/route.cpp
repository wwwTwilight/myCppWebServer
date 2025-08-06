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
    get_routes["/upload.html"] = get_page;
    get_routes["/files.html"] = get_page;
    get_routes["/api/files"] = list_uploads_json;
    get_routes["/api/download"] = handle_download;

    post_routes["/post.html"] = login_page;
    post_routes["/upload"] = file_upload;
    post_routes["/api/delete"] = file_upload;
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
        method_not_supported(http_message.client_socket);
        return 0;
    }
}

int getRoute(HttpMessage& http_message) {
    if(get_routes.contains(http_message.path)) {
        return get_routes[http_message.path](http_message);
    } else {
        return get_file(http_message);
    }
}

int postRoute(HttpMessage& http_message) {
    if(post_routes.contains(http_message.path)) {
        return post_routes[http_message.path](http_message);
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
    return 1;
}

// 泛用获取文件内容
int get_file(HttpMessage& http_message) {
    int client_socket = http_message.client_socket;
    
    // URL 解码路径
    string decoded_path = urlDecode(http_message.path);
    string fullpath = decoded_path[0] == '/' ? decoded_path.substr(1) : decoded_path;

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
    return 1;
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

    return 1;
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
        snprintf(headbuf.data(), headbuf.size(), "HTTP/1.1 200 OK\r\n");
        send(client_socket, headbuf.data(), strlen(headbuf.data()), 0);

        open_http_file(client_socket, "/login_failed.html");
        
        return 0;
    }
}

int file_upload(HttpMessage& http_message) {
    if(http_message.headers.find("Content-Length") == http_message.headers.end()) {
        error_message("Content-Length not found");
        return 0;
    }

    if(http_message.headers.find("Content-Type") == http_message.headers.end()) {
        error_message("Content-Type not found");
        return 0;
    }

    string boundary = http_message.headers["Content-Type"];
    size_t pos = boundary.find("boundary=");
    if (pos == string::npos) {
        error_message("Boundary not found in Content-Type");
        return 0;
    }
    boundary = "--" + boundary.substr(pos + 9); // 边界字符串，带上开头的“--”

    if(boundary[boundary.size() - 1] == '\r' || boundary[boundary.size() - 1] == '\n') {
        boundary = boundary.substr(0, boundary.size() - 1); // 去掉结尾的\r或\n
    }

    // cout << "Boundary: " << boundary << endl;

    // 创建upload目录
    struct stat st{};
    if (stat("upload", &st) == -1) {
        mkdir("upload", 0755);
    }

    string fileContent;

    string &body = http_message.body;

    // cout << "body: " << body << endl;

    size_t boundary_start = body.find(boundary);
    if (boundary_start == string::npos) {
        error_message("Boundary not found in body");
        return 0;
    }

    size_t header_start = boundary_start + boundary.size() + 2;

    Content_Disposition content_disposition;

    size_t left = body.find("name=", header_start);

    if (left == string::npos) {
        error_message("Content-Disposition not found");
        return 0;
    }

    size_t right = body.find(";", left);

    content_disposition.name = body.substr(left + 5 + 1, right - left - 5 - 2); // 这个突兀的 +1 -2 是为了跳过引号

    // 其实这一段没有什么意义，因为本项目上传的表单只有一个文件，name字段只有file

    left = body.find("filename=", right);
    if (left == string::npos) {
        error_message("Content-Disposition not found");
        return 0;
    }

    right = body.find("\r\n", left);

    content_disposition.filename = body.substr(left + 9 + 1, right - left - 9 - 2);

    string filename = file_name_secure(content_disposition.filename);

    left = body.find("\r\n\r\n", right);
    left += 4; // 跳过\r\n\r\n

    right = body.find(boundary + "--", left);

    fileContent = body.substr(left, right - left);

    ofstream ofile("upload/" + filename, ios::binary);
    if (!ofile) {
        error_message("Failed to open file for writing");
        return 0;
    }

    ofile.write(fileContent.data(), fileContent.size());
    ofile.close();

    int client_socket = http_message.client_socket;
    string response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Connection: close\r\n\r\n"
        "<!DOCTYPE html>"
        "<html><head><meta charset='utf-8'></head>"
        "<body><h1>文件上传成功！</h1>"
        "<p>文件名: " + filename + "</p>"
        "<p>大小: " + to_string(fileContent.size()) + " 字节</p>"
        "</body></html>";
    
    send(client_socket, response.c_str(), response.length(), 0);

    return 1;
}

int list_uploads_json(HttpMessage& http_message) {
    int client_socket = http_message.client_socket;
    
    string command = "cd upload && ls -1";
    string files_output = executeCommand(command);
    
    // 简单的JSON格式
    string files_json = "[";
    if (!files_output.empty()) {
        istringstream iss(files_output);
        string filename;
        bool first = true;
        
        while (getline(iss, filename)) {
            if (filename.empty()) continue;
            
            if (!first) files_json += ",";
            files_json += "\"" + filename + "\"";  // 只返回文件名字符串
            first = false;
        }
    }
    files_json += "]";
    
    string response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json; charset=utf-8\r\n"
        "Connection: close\r\n\r\n" + files_json;
    
    send(client_socket, response.data(), response.length(), 0);
    return 1;
}

int handle_download(HttpMessage& http_message) {
    string queryKey = "file=";
    size_t pos = http_message.query.find(queryKey) + queryKey.size();
    if (pos == string::npos || pos >= http_message.query.size()) {
        not_found(http_message.client_socket);
        return 0;
    }

    string filename = urlDecode(http_message.query.substr(pos));

    string fullpath = "upload/" + filename;

    cout << "Downloading file: " << fullpath << endl;

    ifstream ifile(fullpath.data(), ios::binary);
    if (!ifile) {
        not_found(http_message.client_socket);
        return 0;
    }
    
    ifile.seekg(0, ios::end);
    size_t filesize = ifile.tellg();
    ifile.seekg(0, ios::beg);

    string mime_type = get_mime_type(fullpath);

    vector<char> headbuf(512);
    snprintf(headbuf.data(), headbuf.size(),
        "HTTP/1.1 200 OK\r\n"
        "Server: MyPoorWebServer\r\n"
        "Content-Type: %s\r\n"
        "Content-Disposition: attachment; filename=\"%s\"\r\n"
        "Content-Length: %zu\r\n\r\n", mime_type.data(), filename.data(), filesize);
    
    send(http_message.client_socket, headbuf.data(), strlen(headbuf.data()), 0);

    vector<char> buffer(4096);
    while (!ifile.eof()) {
        ifile.read(buffer.data(), buffer.size());
        streamsize count = ifile.gcount();
        if (count > 0) {
            send(http_message.client_socket, buffer.data(), count, 0);
        }
    }
    
    return 1;
}

int handle_delete(HttpMessage& http_message) {

}