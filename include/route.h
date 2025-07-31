#ifndef ROUTER_H
#define ROUTER_H

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include "httpMes.h"

using namespace std;

class route {
public:

    unordered_map<string, function<int(HttpMessage&)>> get_routes;
    unordered_map<string, function<int(HttpMessage&)>> post_routes;

    route();

    void routeWork(HttpMessage& http_message);
};

int get_page(HttpMessage& http_message);
int post_page(HttpMessage& http_message);

#endif // ROUTER_H