#include <cstdio>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "mysql.h"
#include "compiler.h"
#include "db_wrapper.h"

char Compiler::m_prefix[128];
Compiler* Compiler::s_comp;

int Compiler::do_compile_cpp(int src_id)
{
    char cpp_cmd[][20] = {"g++", "", "-static", "-O2", "-o", "", NULL};
    sprintf(cpp_cmd[1], "%s/%d.cpp", m_prefix, src_id);
    sprintf(cpp_cmd[5], "%s/%d.exe", m_prefix, src_id);
    pid_t pid = fork();
    if (fork() == 0) {
        // child
        struct rlimit lim;
        lim.rlim_max = 60;
        lim.rlim_cur = 60;
        setrlimit(RLIMIT_CPU, &lim);
        // TODO: where the parameters stored?    
        execvp("g++", (char * const *)cpp_cmd);
    } else {
        int status = 0;
        waitpid(pid, &status, 0);
        return status;
    }
}

int Compiler::do_compile_java(int src_id)
{

}

int Compiler::PickSource()
{
    DBWrapper *db = DBWrapper::GetInstance();
    db->Query("SELECT id, language FROM tab_submit WHERE stutus = 0 LIMIT = 1");
    MYSQL_ROW row;
    if (row = db->FetchRow()) {
        int lang, id;
        sscanf(row[0], "%d", &id);
        sscanf(row[1], "%d", &lang);
        int status = 0;
        switch (lang) {
            case 0:
                status = do_compile_cpp(id);
                break;
            case 1:
                status = do_compile_java(id);
                break;
        }
        // update database
    }
    return 0;
}

