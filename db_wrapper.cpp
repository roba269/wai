#include <cassert>
#include <cstdio>
#include "mysql.h"
#include "db_wrapper.h"

// DBWrapper* DBWrapper::s_wrapper = NULL;
MYSQL* DBWrapper::s_mysql_handle = NULL;
// MYSQL_RES* DBWrapper::s_mysql_res = NULL; 

void DBWrapper::InitInstance()
{
    s_mysql_handle = mysql_init(NULL);
    if (!mysql_real_connect(s_mysql_handle, "localhost",
            "root", "passwd", "wai", 0, NULL, 0))
    {
        fprintf(stderr, "Mysql Connect Error\n");
    } 
}
/*
int DBWrapper::Query(const char *stmt)
{
    mysql_query(s_mysql_handle, stmt);
    s_mysql_res = mysql_store_result(s_mysql_handle);
    if (s_mysql_res == NULL) {
        fprintf(stderr, "%s\n", mysql_error(s_mysql_handle));
        return -1;
    }
    return 0;
}

MYSQL_ROW DBWrapper::FetchRow()
{
    return mysql_fetch_row(s_mysql_res);
}

int DBWrapper::FreeResult()
{
    mysql_free_result(s_mysql_res);
}
*/
