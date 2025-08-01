#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <sstream>
#include <string>

using namespace std;

class TimestampConverter {
public:
    // 将时间戳转换为可读时间字符串
    static string timestampToString(time_t timestamp, const string& format = "%Y-%m-%d %H:%M:%S") {
        struct tm* timeinfo = localtime(&timestamp);
        if (timeinfo == nullptr) {
            return "Invalid timestamp";
        }
        
        char buffer[100];
        strftime(buffer, sizeof(buffer), format.c_str(), timeinfo);
        return string(buffer);
    }
    
    // 将时间字符串转换为时间戳
    static time_t stringToTimestamp(const string& timeStr, const string& format = "%Y-%m-%d %H:%M:%S") {
        struct tm timeinfo = {};
        istringstream ss(timeStr);
        ss >> get_time(&timeinfo, format.c_str());
        
        if (ss.fail()) {
            return -1;  // 转换失败
        }
        
        return mktime(&timeinfo);
    }
    
    // 获取当前时间戳
    static time_t getCurrentTimestamp() {
        return time(nullptr);
    }
    
    // 获取高精度时间戳（毫秒）
    static long long getCurrentTimestampMs() {
        auto now = chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        return chrono::duration_cast<chrono::milliseconds>(duration).count();
    }
    
    // 将毫秒时间戳转换为可读时间
    static string timestampMsToString(long long timestampMs, const string& format = "%Y-%m-%d %H:%M:%S") {
        time_t timestamp = timestampMs / 1000;
        int milliseconds = timestampMs % 1000;
        
        string timeStr = timestampToString(timestamp, format);
        if (timeStr != "Invalid timestamp") {
            timeStr += "." + to_string(milliseconds);
        }
        return timeStr;
    }
    
    // 时间戳差值计算（返回秒数）
    static long long timeDifference(time_t timestamp1, time_t timestamp2) {
        return abs(timestamp1 - timestamp2);
    }
    
    // 格式化时间差
    static string formatTimeDifference(long long seconds) {
        long long days = seconds / (24 * 3600);
        seconds %= (24 * 3600);
        long long hours = seconds / 3600;
        seconds %= 3600;
        long long minutes = seconds / 60;
        seconds %= 60;
        
        stringstream ss;
        if (days > 0) ss << days << " 天 ";
        if (hours > 0) ss << hours << " 小时 ";
        if (minutes > 0) ss << minutes << " 分钟 ";
        if (seconds > 0) ss << seconds << " 秒";
        
        string result = ss.str();
        return result.empty() ? "0 秒" : result;
    }
};

// 交互式菜单
void showMenu() {
    cout << "\n=== 时间戳转换工具 ===" << endl;
    cout << "1. 时间戳 -> 可读时间" << endl;
    cout << "2. 可读时间 -> 时间戳" << endl;
    cout << "3. 获取当前时间戳" << endl;
    cout << "4. 获取当前毫秒时间戳" << endl;
    cout << "5. 毫秒时间戳 -> 可读时间" << endl;
    cout << "6. 计算时间差" << endl;
    cout << "7. 批量转换演示" << endl;
    cout << "0. 退出" << endl;
    cout << "请选择功能: ";
}

void timestampToReadable() {
    cout << "\n请输入时间戳: ";
    time_t timestamp;
    cin >> timestamp;
    
    cout << "转换结果:" << endl;
    cout << "标准格式: " << TimestampConverter::timestampToString(timestamp) << endl;
    cout << "日期格式: " << TimestampConverter::timestampToString(timestamp, "%Y年%m月%d日") << endl;
    cout << "时间格式: " << TimestampConverter::timestampToString(timestamp, "%H:%M:%S") << endl;
    cout << "ISO格式: " << TimestampConverter::timestampToString(timestamp, "%Y-%m-%dT%H:%M:%S") << endl;
}

void readableToTimestamp() {
    cout << "\n请输入时间字符串 (格式: YYYY-MM-DD HH:MM:SS): ";
    cin.ignore(); // 忽略之前的换行符
    string timeStr;
    getline(cin, timeStr);
    
    time_t timestamp = TimestampConverter::stringToTimestamp(timeStr);
    if (timestamp == -1) {
        cout << "时间格式错误！请使用 YYYY-MM-DD HH:MM:SS 格式" << endl;
    } else {
        cout << "转换结果: " << timestamp << endl;
    }
}

void showCurrentTimestamp() {
    time_t current = TimestampConverter::getCurrentTimestamp();
    cout << "\n当前时间戳: " << current << endl;
    cout << "当前时间: " << TimestampConverter::timestampToString(current) << endl;
}

void showCurrentTimestampMs() {
    long long currentMs = TimestampConverter::getCurrentTimestampMs();
    cout << "\n当前毫秒时间戳: " << currentMs << endl;
    cout << "当前时间: " << TimestampConverter::timestampMsToString(currentMs) << endl;
}

void msTimestampToReadable() {
    cout << "\n请输入毫秒时间戳: ";
    long long timestampMs;
    cin >> timestampMs;
    
    cout << "转换结果: " << TimestampConverter::timestampMsToString(timestampMs) << endl;
}

void calculateTimeDifference() {
    cout << "\n请输入第一个时间戳: ";
    time_t timestamp1;
    cin >> timestamp1;
    
    cout << "请输入第二个时间戳: ";
    time_t timestamp2;
    cin >> timestamp2;
    
    long long diff = TimestampConverter::timeDifference(timestamp1, timestamp2);
    cout << "\n时间差:" << endl;
    cout << "秒数: " << diff << endl;
    cout << "格式化: " << TimestampConverter::formatTimeDifference(diff) << endl;
    
    cout << "\n时间对比:" << endl;
    cout << "时间1: " << TimestampConverter::timestampToString(timestamp1) << endl;
    cout << "时间2: " << TimestampConverter::timestampToString(timestamp2) << endl;
}

void batchDemo() {
    cout << "\n=== 批量转换演示 ===" << endl;
    
    // 一些常见的时间戳
    time_t timestamps[] = {
        0,           // Unix 纪元
        946684800,   // 2000年1月1日
        1640995200,  // 2022年1月1日
        1672531200,  // 2023年1月1日
        1704067200,  // 2024年1月1日
        TimestampConverter::getCurrentTimestamp()  // 当前时间
    };
    
    string descriptions[] = {
        "Unix 纪元 (1970-01-01)",
        "千禧年 (2000-01-01)",
        "2022年元旦",
        "2023年元旦", 
        "2024年元旦",
        "当前时间"
    };
    
    for (int i = 0; i < 6; i++) {
        cout << descriptions[i] << ":" << endl;
        cout << "  时间戳: " << timestamps[i] << endl;
        cout << "  时间: " << TimestampConverter::timestampToString(timestamps[i]) << endl;
        cout << endl;
    }
}

int main() {
    cout << "欢迎使用时间戳转换工具！" << endl;
    
    int choice;
    do {
        showMenu();
        cin >> choice;
        
        switch (choice) {
            case 1:
                timestampToReadable();
                break;
            case 2:
                readableToTimestamp();
                break;
            case 3:
                showCurrentTimestamp();
                break;
            case 4:
                showCurrentTimestampMs();
                break;
            case 5:
                msTimestampToReadable();
                break;
            case 6:
                calculateTimeDifference();
                break;
            case 7:
                batchDemo();
                break;
            case 0:
                cout << "感谢使用！再见！" << endl;
                break;
            default:
                cout << "无效选择，请重新输入！" << endl;
        }
    } while (choice != 0);
    
    return 0;
}
