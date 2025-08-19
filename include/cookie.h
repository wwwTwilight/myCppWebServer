#ifndef COOKIE_H
#define COOKIE_H

#include <string>
#include <ctime>
#include <vector>
#include <unordered_map>

using namespace std;

// 用于设置Cookie的类
class setCookie {
public:
    vector<string> cookies;

    setCookie(string username);

    void addCookie(const string& cookie);

    string getCookie();
};

// 用于获取Cookie具体的每一个name=val的类
class Cookie {
public:
    string originalCookie; // 原始Cookie字符串
    unordered_map<string, string> cookies;

    Cookie(const string& cookie);

    string parseCookie(const string& name);
};

#endif // COOKIE_H