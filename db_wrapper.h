#ifndef DB_WRAPPER_H
#define DB_WRAPPER_H

#include <cassert>
#include "mysql.h"

class DBWrapper {
public:
    static int InitInstance();
    static DBWrapper *GetInstance() {
        assert(s_wrapper);
        return s_wrapper;
    }
    static void *DestroyInstance() {
        mysql_close(s_mysql_handle);
        delete s_wrapper;
    }
    static int Query(const char *str);
    static MYSQL_ROW FetchRow();
    static int FreeResult();

private:
    static DBWrapper *s_wrapper;
    static MYSQL *s_mysql_handle;
    static MYSQL_RES *s_mysql_res;
};

#endif
