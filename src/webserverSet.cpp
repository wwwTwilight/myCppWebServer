#include "../include/webserverSet.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

void error_exit(const string& message) {
    cerr << message << endl;
    exit(0);
}

void error_message(const string& message) {
	cout << "Error: " << message << endl;
}

int startServer(unsigned short& port) {
    if (port == 0) {
        port = 8080;
    }
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in name;
    socklen_t name_len = sizeof(name);
    memset(&name, 0, sizeof(name));
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        error_exit("setsockopt error");
    }

    if (::bind(server_socket, (sockaddr*)&name, name_len) < 0) {
        error_exit("bind error");
    }

    if (listen(server_socket, 10) < 0) {
        error_exit("listen error");
    }

    cout << "server start on socket " << server_socket << endl;
    cout << "server start on address " << inet_ntoa(name.sin_addr) << endl;
    cout << "server start on port " << port << endl;

    return server_socket;
}