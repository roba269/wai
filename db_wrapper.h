#ifndef DB_WRAPPER_H
#define DB_WRAPPER_H

#include <cassert>
#include "mysql.h"

class DBWrapper {
public:
    /*
    static DBWrapper *GetInstance() {
        if (s_wrapper == NULL) {
            s_wrapper = new DBWrapper();
            s_wrapper->InitInstance();
        }
        return s_wrapper;
    }
    */
    DBWrapper() {
        InitInstance();
    }
    ~DBWrapper() {
        DestroyInstance();
    }
    static void DestroyInstance() {
        mysql_close(s_mysql_handle);
        s_mysql_handle = NULL;
        // if (s_wrapper)
        //     delete s_wrapper;
    }
    /*
    int Query(const char *str);
    MYSQL_ROW FetchRow();
    int FreeResult();
    */
    static MYSQL *GetHandle() {
        // FIXME: mutex?
        if (s_mysql_handle == NULL) {
            InitInstance();
        }
        return s_mysql_handle;
    }
private:
    static void InitInstance();
    static MYSQL *s_mysql_handle;
};

#endif
