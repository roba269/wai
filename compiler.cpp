#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>
#include "mysql.h"
#include "compiler.h"
#include "common_define.h"
#include "db_wrapper.h"
#include "scheduler.h"

char Compiler::m_prefix[128];
Compiler* Compiler::s_comp;

int Compiler::do_compile_cpp(int src_id)
{
    char *cpp_cmd[] = {"g++", NULL, "-static", "-O2", "-o", NULL, NULL};
    char src_path[128], dst_path[128];
    sprintf(src_path, "%s/%d.cpp", m_prefix, src_id);
    sprintf(dst_path, "%s/%d.exe", m_prefix, src_id);
    cpp_cmd[1] = src_path;
    cpp_cmd[5] = dst_path;
    pid_t pid = fork();
    if (fork() == 0) {
        // child
        struct rlimit lim;
        lim.rlim_max = 60;
        lim.rlim_cur = 60;
        setrlimit(RLIMIT_CPU, &lim);
        // TODO: where the parameters stored?
        char filename[32];
        sprintf(filename, "%d.txt", src_id);
        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IRGRP);
        if (fd != STDERR_FILENO) {
            dup2(fd, STDERR_FILENO);
            close(fd);
        }
        execvp("g++", (char * const *)cpp_cmd);
    } else {
        int status = 0;
        waitpid(pid, &status, 0);
        return status;
    }
    assert(false);
    return 0;
}

int Compiler::do_compile_java(int src_id)
{
    return 0;
}

int Compiler::PickSource()
{
    MYSQL *handle = DBWrapper::GetHandle();
    mysql_query(handle, "SELECT id, lang, game_type, user_id FROM main_app_submit WHERE status = 0 ORDER BY sub_time LIMIT 1");
    MYSQL_RES *mysql_res = mysql_store_result(handle);
    MYSQL_ROW row;
    if ((row = mysql_fetch_row(mysql_res))) {
        int lang, id, user_id;
        char game_type_str[20];
        sscanf(row[0], "%d", &id);
        sscanf(row[1], "%d", &lang);
        sscanf(row[2], "%s", game_type_str);
        sscanf(row[3], "%d", &user_id);
        fprintf(stderr, "compile sub_id: %d lang: %d\n", id, lang);
        int status = 0;
        switch (lang) {
            case 0:
                status = do_compile_cpp(id);
                break;
            case 1:
                status = do_compile_java(id);
                break;
        }
        int err;
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            // compile successful
            err = 1;
            fprintf(stderr, "compile success\n");
        } else {
            // compile error
            err = 2;
            fprintf(stderr, "compile error\n");
        }
        // update database
        char cmd[128];
        sprintf(cmd, "UPDATE main_app_submit SET status = %d WHERE id = %d",
                err, id);
        mysql_query(handle, cmd);
        Scheduler *sched = Scheduler::GetInstance();
        sched->RegNewSub(str2type(game_type_str), user_id, id);
    }
    mysql_free_result(mysql_res);
    return 0;
}

