#include <iostream>
#include <thread>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "include/webserverSet.h"
#include "include/requestHandler.h"
#include "include/route.h"

using namespace std;

int main() {
    int server_socket = -1;
    unsigned short port = 8080;
    int client_socket = -1;
    sockaddr_in client_name;
    socklen_t client_name_len = sizeof(client_name);

    server_socket = startServer(port);

    routeInit(); // 初始化路由表

    while (1) {
        client_socket = accept(server_socket, (sockaddr*)&client_name, &client_name_len);

        cout << "New connection from ip:" << inet_ntoa(client_name.sin_addr) << " and port:" << ntohs(client_name.sin_port) << endl;
        if (client_socket < 0) {
            error_exit("accept error");
        }
        thread new_thread(accept_request, client_socket);
        new_thread.detach();
    }

    return 0;
}