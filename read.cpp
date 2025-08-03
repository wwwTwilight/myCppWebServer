#include <sys/stat.h>
#include <fstream>
#include <sstream>

// 提取 boundary
string extract_boundary(const string& content_type) {
    size_t pos = content_type.find("boundary=");
    if (pos == string::npos) return "";
    
    string boundary = content_type.substr(pos + 9);
    // 移除可能的引号
    if (boundary.front() == '"' && boundary.back() == '"') {
        boundary = boundary.substr(1, boundary.length() - 2);
    }
    return boundary;
}

// 解析单个 multipart 部分
struct MultipartPart {
    string name;
    string filename;
    string content_type;
    vector<char> data;
};

vector<MultipartPart> parse_multipart_data(const string& body, const string& boundary) {
    vector<MultipartPart> parts;
    
    string delimiter = "--" + boundary;
    string end_delimiter = delimiter + "--";
    
    size_t pos = 0;
    
    while (true) {
        // 查找下一个边界
        size_t start = body.find(delimiter, pos);
        if (start == string::npos) break;
        
        // 跳过边界
        start += delimiter.length();
        
        // 检查是否是结束边界
        if (body.substr(start, 2) == "--") break;
        
        // 跳过 CRLF
        if (body.substr(start, 2) == "\r\n") start += 2;
        
        // 查找下一个边界（这部分的结束）
        size_t end = body.find(delimiter, start);
        if (end == string::npos) break;
        
        // 提取这一部分
        string part_data = body.substr(start, end - start);
        
        // 解析这一部分
        MultipartPart part = parse_single_part(part_data);
        if (!part.name.empty()) {
            parts.push_back(part);
        }
        
        pos = end;
    }
    
    return parts;
}

MultipartPart parse_single_part(const string& part_data) {
    MultipartPart part;
    
    // 查找空行（分隔 headers 和 data）
    size_t header_end = part_data.find("\r\n\r\n");
    if (header_end == string::npos) return part;
    
    string headers = part_data.substr(0, header_end);
    string data = part_data.substr(header_end + 4);
    
    // 移除末尾的 CRLF
    if (data.length() >= 2 && data.substr(data.length() - 2) == "\r\n") {
        data = data.substr(0, data.length() - 2);
    }
    
    // 解析 headers
    istringstream header_stream(headers);
    string line;
    
    while (getline(header_stream, line)) {
        // 移除 CRLF
        if (!line.empty() && line.back() == '\r') line.pop_back();
        
        if (line.find("Content-Disposition:") == 0) {
            // 解析 Content-Disposition
            size_t name_pos = line.find("name=\"");
            if (name_pos != string::npos) {
                name_pos += 6;
                size_t name_end = line.find("\"", name_pos);
                if (name_end != string::npos) {
                    part.name = line.substr(name_pos, name_end - name_pos);
                }
            }
            
            size_t filename_pos = line.find("filename=\"");
            if (filename_pos != string::npos) {
                filename_pos += 10;
                size_t filename_end = line.find("\"", filename_pos);
                if (filename_end != string::npos) {
                    part.filename = line.substr(filename_pos, filename_end - filename_pos);
                }
            }
        }
        else if (line.find("Content-Type:") == 0) {
            part.content_type = line.substr(14); // 跳过 "Content-Type: "
        }
    }
    
    // 存储数据
    part.data.assign(data.begin(), data.end());
    
    return part;
}

// 生成安全的文件名
string generate_safe_filename(const string& original_filename) {
    if (original_filename.empty()) return "unnamed_file";
    
    string safe_name = original_filename;
    
    // 替换危险字符
    for (char& c : safe_name) {
        if (c == '/' || c == '\\' || c == '..' || c < 32 || c == ':' || c == '*' || c == '?' || c == '<' || c == '>' || c == '|') {
            c = '_';
        }
    }
    
    // 避免重名，添加时间戳
    time_t now = time(nullptr);
    string timestamp = to_string(now);
    
    size_t dot_pos = safe_name.find_last_of('.');
    if (dot_pos != string::npos) {
        safe_name.insert(dot_pos, "_" + timestamp);
    } else {
        safe_name += "_" + timestamp;
    }
    
    return safe_name;
}

// 检查并创建目录
bool ensure_upload_directory() {
    struct stat st;
    if (stat("httpdocs/upload", &st) != 0) {
        // 目录不存在，创建它
        if (mkdir("httpdocs/upload", 0755) != 0) {
            return false;
        }
    }
    return true;
}

// 主要的文件上传处理函数
void handle_file_upload(int client_socket, const HttpMessage& http_message) {
    cout << "DEBUG: 开始处理文件上传" << endl;
    
    // 获取 Content-Type
    string content_type;
    for (const auto& header : http_message.headers) {
        if (strcasecmp(header.first.c_str(), "Content-Type") == 0) {
            content_type = header.second;
            break;
        }
    }
    
    cout << "DEBUG: Content-Type = " << content_type << endl;
    
    // 检查是否是 multipart/form-data
    if (content_type.find("multipart/form-data") == string::npos) {
        send_error_response(client_socket, "400 Bad Request", "不是有效的文件上传请求");
        return;
    }
    
    // 提取 boundary
    string boundary = extract_boundary(content_type);
    if (boundary.empty()) {
        send_error_response(client_socket, "400 Bad Request", "无法解析 boundary");
        return;
    }
    
    cout << "DEBUG: boundary = " << boundary << endl;
    
    // 确保上传目录存在
    if (!ensure_upload_directory()) {
        send_error_response(client_socket, "500 Internal Server Error", "无法创建上传目录");
        return;
    }
    
    // 解析 multipart 数据
    vector<MultipartPart> parts = parse_multipart_data(http_message.body, boundary);
    
    cout << "DEBUG: 解析到 " << parts.size() << " 个部分" << endl;
    
    // 查找文件部分
    MultipartPart* file_part = nullptr;
    for (auto& part : parts) {
        cout << "DEBUG: 部分名称=" << part.name << ", 文件名=" << part.filename << ", 大小=" << part.data.size() << endl;
        
        if (!part.filename.empty()) {
            file_part = &part;
            break;
        }
    }
    
    if (!file_part) {
        send_error_response(client_socket, "400 Bad Request", "没有找到文件数据");
        return;
    }
    
    // 检查文件大小（限制 10MB）
    const size_t MAX_FILE_SIZE = 10 * 1024 * 1024;
    if (file_part->data.size() > MAX_FILE_SIZE) {
        send_error_response(client_socket, "413 Payload Too Large", "文件大小超过 10MB 限制");
        return;
    }
    
    // 生成安全的文件名
    string safe_filename = generate_safe_filename(file_part->filename);
    string filepath = "httpdocs/upload/" + safe_filename;
    
    cout << "DEBUG: 保存文件到 " << filepath << endl;
    
    // 保存文件
    ofstream outfile(filepath, ios::binary);
    if (!outfile) {
        send_error_response(client_socket, "500 Internal Server Error", "无法保存文件");
        return;
    }
    
    outfile.write(file_part->data.data(), file_part->data.size());
    outfile.close();
    
    cout << "DEBUG: 文件保存成功" << endl;
    
    // 发送成功响应
    send_upload_success_response(client_socket, safe_filename, file_part->data.size());
}

// 发送错误响应
void send_error_response(int client_socket, const string& status, const string& message) {
    string response = 
        "HTTP/1.1 " + status + "\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "\r\n"
        "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>上传失败</title></head>"
        "<body style='font-family: Arial; text-align: center; padding: 50px;'>"
        "<h1>❌ 上传失败</h1>"
        "<p>" + message + "</p>"
        "<button onclick='history.back()'>返回</button>"
        "</body></html>";
    
    send(client_socket, response.c_str(), response.length(), 0);
}

// 发送成功响应
void send_upload_success_response(int client_socket, const string& filename, size_t filesize) {
    string response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "\r\n"
        "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>上传成功</title>"
        "<style>body{font-family:Arial;text-align:center;padding:50px;background:#f0f8ff;}"
        ".success{background:white;padding:30px;border-radius:10px;display:inline-block;box-shadow:0 2px 10px rgba(0,0,0,0.1);}"
        "button{padding:10px 20px;margin:5px;border:none;border-radius:5px;cursor:pointer;}"
        ".btn-primary{background:#4CAF50;color:white;}"
        ".btn-secondary{background:#f0f0f0;color:#333;}</style></head>"
        "<body><div class='success'>"
        "<h1>✅ 文件上传成功！</h1>"
        "<p><strong>文件名：</strong>" + filename + "</p>"
        "<p><strong>大小：</strong>" + to_string(filesize) + " 字节</p>"
        "<p><strong>保存位置：</strong>httpdocs/upload/" + filename + "</p>"
        "<br>"
        "<button class='btn-primary' onclick='location.href=\"upload.html\"'>继续上传</button>"
        "<button class='btn-secondary' onclick='location.href=\"index.html\"'>返回主页</button>"
        "</div></body></html>";
    
    send(client_socket, response.c_str(), response.length(), 0);
}