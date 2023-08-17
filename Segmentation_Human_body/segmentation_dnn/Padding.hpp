#ifndef PADDING_H
#define PADDING_H

class Padding
{
public:
    int mTop, mBottom, mLeft, mRight;

    Padding(const Padding& pObj);

    Padding(int pTop = 0, int pBottom = 0, int pLeft = 0, int pRight = 0);

    void show();
};

#endif