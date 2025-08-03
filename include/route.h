#ifndef ROUTER_H
#define ROUTER_H

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include "httpMes.h"

using namespace std;

extern unordered_map<string, function<int(HttpMessage&)>> get_routes;
extern unordered_map<string, function<int(HttpMessage&)>> post_routes;

void routeInit();

int routeWork(HttpMessage& http_message);

int getRoute(HttpMessage& http_message);

int postRoute(HttpMessage& http_message);

int get_page(HttpMessage& http_message);
int post_page(HttpMessage& http_message);
int login_page(HttpMessage& http_message);
int file_upload(HttpMessage& http_message);

#endif // ROUTER_H