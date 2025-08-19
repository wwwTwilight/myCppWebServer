#include "../include/utils.h"
#include "../include/cookie.h"
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <cstdio>
#include <ctime>

using namespace std;

setCookie::setCookie(string username) {
    addCookie("time=" + to_string(getTime())); // 添加当前时间戳
    addCookie("max-age=300");
    addCookie("username=" + username); // 添加用户名
}

// 注意cookie是name=val的形式
void setCookie::addCookie(const string& cookie) {
    cookies.push_back(cookie);
}

// 获得所有的Cookie字符串，自带前缀和换行，用于设置cookie
string setCookie::getCookie() {
    string cookieStr;
    for (const auto& cookie : cookies) {
        cookieStr += "Set-Cookie: " + cookie + "\r\n";
    }
    return cookieStr;
}

Cookie::Cookie(const string& cookie) {
    originalCookie = cookie; // 保存原始Cookie字符串
    size_t pos = 0;
    while(originalCookie[pos] == ' ') {
        pos++;
    }
    originalCookie = originalCookie.substr(pos); // 去掉前面的空格
    parseCookie(originalCookie);
}
string Cookie::parseCookie(const string& cookie) {
    size_t left = 0, right = 0;
    while (right < cookie.size()) {
        while (right < cookie.size() && cookie[right] != ';') {
            right++;
        }
        string pair = cookie.substr(left, right - left);
        size_t equalPos = pair.find('=');
        if (equalPos != string::npos) {
            string name = pair.substr(0, equalPos);
            string value = pair.substr(equalPos + 1);
            cookies[name] = value; // 存储name=val对
        }
        left = right + 1; // 跳过分号
        right++;
        while(left < cookie.size() && cookie[left] == ' ') {
            left++; // 跳过空格
            right++;
        }
    }
    return "";
}