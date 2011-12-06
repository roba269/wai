#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include <set>
#include <map>

class Scheduler {
public:
    virtual ~Scheduler() {};
    int InitScheduler();
    virtual int ArrangeMatch(int &a, int &b);
private:
    int _build_player_list();
    int _build_matrix();
    void _add_pair(int a, int b);
    std::map<int, std::set<int> > m_mat;
    std::vector<int> m_plist;
};

#endif
