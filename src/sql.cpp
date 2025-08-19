#include "../include/sql.h"
#include "../include/httpMes.h"

sql::mysql::MySQL_Driver *driver;
unique_ptr<sql::Connection> con;

int SQLinit() {
    try {
        const string host = "tcp://127.0.0.1:3306";
        const string user = "root";
        const string password = "Password123#@!";  // 密码记得改
        const string database = "netDisk"; // 数据库名要匹配

        driver = sql::mysql::get_mysql_driver_instance();

        con.reset(driver->connect(host, user, password));

        con->setSchema(database);

    } catch (sql::SQLException &e) {
        cerr << "SQL 错误: " << e.what() << endl;
        cerr << "错误代码: " << e.getErrorCode() << endl;
        cerr << "SQL 状态: " << e.getSQLState() << endl;
        return 1;
    } catch (...) {
        cerr << "未知错误" << endl;
        return 1;
    }

    return 0;
}

// 验证用户账号密码是否匹配，匹配返回1，不匹配返回0
int verifyAccount(HttpMessage& http_message) {
    if (http_message.body.find("username") == string::npos || http_message.body.find("password") == string::npos) {
        return 0;
    }

    size_t left = http_message.body.find("username") + 9; // 9是"username="的长度
    size_t right = http_message.body.find('&', left);
    string username = http_message.body.substr(left, right - left);

    left = http_message.body.find("password") + 9; // 9是"password="的长度
    right = http_message.body.find('&', left);
    string password = http_message.body.substr(left, right - left);

    try {
        unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("SELECT COUNT(*) FROM users WHERE username = ? AND password = ?"));
        pstmt->setString(1, username);
        pstmt->setString(2, password);

        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if (res->next() && res->getInt(1) > 0) {
            cout << "用户验证成功: " << username << endl;
            return 1;
        }
        else {
            cout << "用户验证失败: " << username << endl;
            return 0;
        }
    } catch (sql::SQLException &e) {
        cerr << "SQL 错误: " << e.what() << endl;
        return 0; // 验证失败
    }
    catch (...) {
        cerr << "未知错误" << endl;
        return 0; // 验证失败
    }

    return 0; // 验证失败
}