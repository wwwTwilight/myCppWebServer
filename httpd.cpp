// 写在前面，虽然是使用cpp重写，但是并没有使用面向对象的思想，毕竟这一份主要在于理解webserver在cpp中的实现，后面会使用面向对象的思想来重写
#include <iostream>         // 输入输出操作
#include <string>           // 字符串处理
#include <vector>           // 动态数组
#include <memory>           // 智能指针（unique_ptr/shared_ptr）
#include <mutex>            // 互斥锁
#include <thread>           // 线程支持
#include <fstream>          // 文件操作
#include <sstream>          // 字符串流
#include <stdexcept>        // 异常处理
#include <cstdint>          // 固定大小整数类型
#include <sys/socket.h>     // socket函数
#include <netinet/in.h>     // 网络地址结构
#include <arpa/inet.h>      // IP地址转换
#include <unistd.h>         // close/read/write等系统调用
#include <sys/stat.h>       // 文件状态查询
#include <fcntl.h>          // 文件控制
#include <pthread.h>        // POSIX线程（如需兼容旧代码）
#include <cctype>           // 字符处理（如isspace）
#include <cstring>          // C风格字符串函数
#include <cstdio>           // C风格输入输出
#include <cstdlib>          // 标准库函数
#include <stdio.h>			//标准输入输出
#include <sys/socket.h>		//套接字函数
#include <sys/types.h>		//套接字类型
#include <netinet/in.h>		//网络地址结构
#include <arpa/inet.h>		//IP地址转换
#include <unistd.h>			//close/read/write等系统调用
#include <ctype.h>			//字符处理函数
#include <strings.h>		//字符串处理函数
#include <string.h>			//字符串处理函数
#include <sys/stat.h>		//文件状态查询
#include <pthread.h>		//POSIX线程（如需兼容旧代码）
#include <sys/wait.h>		//进程等待
#include <stdlib.h>			//标准库函数
using namespace std;

void error_exit(string message) {
    cerr << message << endl;
    exit(0);
}

// 获取一行HTTP报文
int getHttpLine(int client_socket, vector<char> &buffer, int buffer_size = -1) {
	if(buffer_size == -1) {
		buffer_size = buffer.size();
	}
	
	int i = 0;
	char c = '\0';
	int n = 0;

	while(i < buffer_size && c != '\0') {
		n = recv(client_socket, &c, 1, 0);

		if(n > 0) {
			if(c == '\r') {
				n = recv(client_socket, &c, 1, MSG_PEEK);
				if(n > 0 && c == '\n') {
					n = recv(client_socket, &c, 1, 0);
				}
				else {
					c = '\0';
				}
			}
		}
	}

}

void header(int client_socket) {
	vector<char> buffer(1024);
	sprintf(buffer.data(), "HTTP/1.1 200 OK\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);

	sprintf(buffer.data(), "Server: MyPoorWebServer\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);

	sprintf(buffer.data(), "Content-Type: text/html\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);

	sprintf(buffer.data(), "\r\n");
	send(client_socket, buffer.data(), strlen(buffer.data()), 0);
}

void *accept_request(int client_socket) {
	vector<char> buffer(1024);
	int n = 1;
	while(n > 0) {
		n = recv(client_socket, buffer.data(), sizeof(buffer), 0); // 这个是阻塞的，读完就卡住了
		cout << buffer.data();
		
	}

	header(client_socket);

	ifstream ifile;

	ifile.open("httpdocs/test.html", ios::binary);
	if(!ifile) {
		error_exit("file error");
	}
	ifile.seekg(0, ios::beg);

	while(!ifile.eof()) {
		ifile.read(buffer.data(), sizeof(buffer));
		send(client_socket, buffer.data(), ifile.gcount(), 0);
	}

    close(client_socket);
    return NULL;
}

int startSever(unsigned short &port) {
	if(port == 0) {
		port = 8080;
	}
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	// 现把端口和地址的信息填入name中，然后bind
    sockaddr_in name;
	socklen_t name_len = sizeof(name);
	memset(&name, 0, sizeof(name));
	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);

	// 设置端口复用，这个要放在绑定前面
	int opt = 1;
	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		error_exit("setsockopt error");
	}

	if(bind(server_socket, (sockaddr*)&name, name_len) < 0) {
		error_exit("bind error");
	}

	// 开始监听
	if(listen(server_socket, 10) < 0) {
		error_exit("listen error");
	}

	cout << "server start on socket " << server_socket << endl;
	cout << "server start on address " << inet_ntoa(name.sin_addr) << endl;
	cout << "server start on port " << port << endl;

	return server_socket;
}

int main() {
    int server_socket = -1;
	unsigned short port = 8080;
	int client_socket = -1;
	sockaddr_in client_name;
	socklen_t client_name_len = sizeof(client_name);
	// thread new_thread;
	server_socket = startSever(port);

	while(1) {
	    client_socket = accept(server_socket, (sockaddr*)&client_name, &client_name_len);

		cout << "New connection from ip:" << inet_ntoa(client_name.sin_addr) << " and port:" << ntohs(client_name.sin_port) << endl;
		if(client_socket < 0) {
			error_exit("accept error");
		}
		thread new_thread(accept_request, client_socket);
		new_thread.detach();
	}

    return 0;
}