# include "../include/error.h"
# include <iostream>
# include <cstring>
# include <vector>
# include <sys/socket.h>

// 发送404 Not Found响应
void not_found(int client_socket) {
    vector<char> buffer(1024);

	snprintf(buffer.data(), buffer.size(), "HTTP/1.0 404 Not Found\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);

	snprintf(buffer.data(), buffer.size(), "Server: MyPoorWebServer\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);

	snprintf(buffer.data(), buffer.size(), "Content-Type: text/html\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);

	snprintf(buffer.data(), buffer.size(), "charset=UTF-8\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);

	snprintf(buffer.data(), buffer.size(), "\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);

	snprintf(buffer.data(), buffer.size(), "<html><head><title>Not Found</title></head>\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);

	snprintf(buffer.data(), buffer.size(), "<body><p>HTTP request file not found.</p></body></html>\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);

	cout << "client: " << client_socket << " 404 Not Found" << endl;
}

// 错误信息报告
void error_message(const string& message) {
	cout << "Error: " << message << endl;
}

// 发送方法不支持的信息
void method_not_supported(int client_socket) {
    vector<char> buffer(1024);

	snprintf(buffer.data(), buffer.size(), "HTTP/1.0 501 Not Implemented\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);

	snprintf(buffer.data(), buffer.size(), "Server: MyPoorWebServer\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);

	snprintf(buffer.data(), buffer.size(), "Content-Type: text/html\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);

	snprintf(buffer.data(), buffer.size(), "charset=UTF-8\r\n"); // 有中文，需要设置字符集，确保别的浏览器能正确显示
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);

	snprintf(buffer.data(), buffer.size(), "\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);

	snprintf(buffer.data(), buffer.size(), "<html><head><title>Method Not Implemented</title></head>\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);

	snprintf(buffer.data(), buffer.size(), "<body><p>HTTP request method not supported.</p></body></html>\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);

	cout << "client: " << client_socket << " 501 Method Not Implemented" << endl;
}