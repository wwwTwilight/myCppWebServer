#!/usr/bin/python3
# upload.cgi: 处理文件上传并保存到 file 文件夹
import sys, os, cgi

UPLOAD_DIR = '../file/'  # 根据你的目录结构调整

print("Content-type:text/html\n")

form = cgi.FieldStorage()

if 'uploadfile' in form:
    fileitem = form['uploadfile']
    if fileitem.filename:
        # 获取安全的文件名
        filename = os.path.basename(fileitem.filename)
        save_path = os.path.join(UPLOAD_DIR, filename)
        try:
            with open(save_path, 'wb') as f:
                f.write(fileitem.file.read())
            print(f"<html><head><meta charset='UTF-8'><title>上传成功</title></head><body>")
            print(f"<h2>文件 {filename} 上传成功！</h2>")
            print("<a href='upload.html'>返回上传页</a>")
            print("</body></html>")
        except Exception as e:
            print(f"<html><body><h2>保存失败: {e}</h2></body></html>")
    else:
        print("<html><body><h2>未选择文件</h2></body></html>")
else:
    print("<html><body><h2>没有收到文件</h2></body></html>")
