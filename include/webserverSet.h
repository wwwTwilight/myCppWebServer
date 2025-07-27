#ifndef WEBSERVER_SET_H
#define WEBSERVER_SET_H

#include <iostream>
#include <string>
#include <vector>

using namespace std;

int startServer(unsigned short& port);
void error_exit(const string& message);
void error_message(const string& message);

#endif // WEBSERVER_SET_H