#include "../include/sql.h"

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

