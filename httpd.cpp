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

	if(bind(server_socket, (sockaddr*)&name, name_len) < 0) {
		error_exit("bind error");
	}
	// 设置端口复用
	int opt = 1;
	if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(&opt)) < 0) {
		error_exit("setsockopt error");
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
	thread new_thread;
	server_socket = startSever(port);

	while(1) {
	    client_socket = accept(server_socket, (sockaddr*)&client_name, &client_name_len);

		cout << "New connection from ip:" << inet_ntoa(client_name.sin_addr) << " and port:" << ntohs(client_name.sin_port) << endl;
		if(client_socket < 0) {
			error_exit("accept error");
		}
		
	}

    return 0;
}