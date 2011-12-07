#ifndef COMPILER_H
#define COMPILER_H

#include <unistd.h>
#include <cstring>

class Compiler {
public:
    static void InitInstance(const char *prefix) {
        s_comp = new Compiler;
        strcpy(m_prefix, prefix);
        // TODO: change to multi-threaded
        if (fork() == 0) {
            while (1) {
                sleep(1);   // sleep 1 second
                PickSource();
            }
        }
    }
    static Compiler *GetInstance() {
        return s_comp;
    }
    static void DestoryInstance() {
        delete s_comp;
    }
private:
    static int PickSource();
    static int do_compile_cpp(int src_id);
    static int do_compile_java(int src_id);
    static Compiler *s_comp;
    static char m_prefix[128];
};

#endif
