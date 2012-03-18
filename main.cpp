#include <cstdio>
#include "compiler.h"
#include "scheduler.h"
#include "simple_match.h"
#include "sandbox.h"

int main() {
    char fname[128];
    Compiler::InitInstance("/home/roba/wai/submit/");
    Scheduler *sched = Scheduler::GetInstance();
    sched->InitScheduler(MATCH_RENJU);
    while (1) {
        int u1, u2, sub1, sub2;
        fprintf(stderr, "try to arrange new match\n");
        if (sched->ArrangeMatch(MATCH_RENJU, u1, u2, sub1, sub2)) {
            fprintf(stderr, "Scheduler arranged an new match: (uid:%d sid:%d) v.s. (uid:%d sid:%d)", u1, sub1, u2, sub2);
            sched->RegNewMatch(MATCH_RENJU, u1, u2, sub1, sub2);
            SimpleMatch *match = new SimpleMatch;
            match->InitMatch(MATCH_RENJU, 0, "Renju Test Match");
            
            snprintf(fname, 128, "/home/roba/wai/submit/%d.exe", sub1);
            Sandbox p1(fname);
            snprintf(fname, 128, "/home/roba/wai/submit/%d.exe", sub2);
            Sandbox p2(fname);
            snprintf(fname, 128, "/home/roba/wai/trunk/test_judge/renju_judge.exe");
            Sandbox judge(fname);
            match->SetJudge(&judge);
            match->AddPlayer(&p1, sub1);
            match->AddPlayer(&p2, sub2);
            match->Start();
        } else {
            sleep(1);
        }
    }
    return 0;
}

