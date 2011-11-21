#ifndef MOVE_H
#define MOVE_H

class Move {
public:
    Move() {};
    virtual ~Move() {};
};

class MoveRenju : Move {
public:
    int x, y, flg;
};

#endif
