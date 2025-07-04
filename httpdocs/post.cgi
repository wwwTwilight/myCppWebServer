#!/usr/bin/python3
# CGI脚本：处理POST请求并展示提交的数据
# 服务器通过环境变量和标准输入传递请求数据，通过标准输出返回HTML响应
#coding:utf-8
# 设置字符编码为UTF-8，确保中文正常显示
import sys,os
# 导入必要模块：
# sys - 用于读取标准输入
# os - 用于获取环境变量
import urllib
# 从环境变量获取POST数据长度
# 这是服务器通过CGI协议传递的关键参数
length = os.getenv('CONTENT_LENGTH')

# 判断是否存在POST数据
if length:
    # 读取指定长度的POST数据（从标准输入）
    # 标准输入已被服务器重定向自cgi_input管道
    postdata = sys.stdin.read(int(length))
    # 输出HTTP响应头（必须以空行结束）
    # 这是CGI协议要求的响应格式
    # 输出HTTP响应头
    print("Content-type:text/html\n")
    # 生成HTML响应体
    print('<html>') 
    print('<head>') 
    print('<title>POST</title>') 
    print('</head>') 
    print('<body>') 
    print('<h2> Your POST data: </h2>')
    print('<ul>')
    # 解析POST数据（格式：key1=value1&key2=value2）
    for data in postdata.split('&'):
        print('<li>' + data + '</li>')
    print('</ul>')
    print('</body>')
    print('</html>')

else:
# 无POST数据时的处理
    print("Content-type:text/html\n")
    # 输出无数据提示
    print('no found')
