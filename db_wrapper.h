#ifndef DB_WRAPPER_H
#define DB_WRAPPER_H

#include <cassert>
#include "mysql.h"

class DBWrapper {
public:
    static DBWrapper *GetInstance() {
        if (s_wrapper == NULL) {
            s_wrapper = new DBWrapper();
            s_wrapper->InitInstance();
        }
        return s_wrapper;
    }
    ~DBWrapper() {
        DestroyInstance();
    }
    void DestroyInstance() {
        mysql_close(s_mysql_handle);
        if (s_wrapper)
            delete s_wrapper;
    }
    int Query(const char *str);
    MYSQL_ROW FetchRow();
    int FreeResult();

private:
    void InitInstance();
    static DBWrapper *s_wrapper;
    MYSQL *s_mysql_handle;
    MYSQL_RES *s_mysql_res;
};

#endif
