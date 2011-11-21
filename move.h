#ifndef MOVE_H
#define MOVE_H

class Move {
public:
    Move() {};
    virtual ~Move() {};
};

class MoveRenju : public Move {
public:
    int x, y, flg;
};

#endif
