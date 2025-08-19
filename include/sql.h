#ifndef SQL_H
#define SQL_H

#include <iostream>
#include <stdexcept>
#include <memory>
#include <string>
#include <iomanip>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <cppconn/prepared_statement.h>
#include "httpMes.h"

using namespace std;

extern sql::mysql::MySQL_Driver *driver;
extern unique_ptr<sql::Connection> con;

int SQLinit();
int verifyAccount(HttpMessage& http_message);

#endif // SQL_H