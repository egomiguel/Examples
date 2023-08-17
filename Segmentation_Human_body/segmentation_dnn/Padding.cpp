#include "Padding.hpp"
#include <stdio.h>

Padding::Padding(const Padding& pObj)
{
    mTop = pObj.mTop;
    mBottom = pObj.mBottom;
    mLeft = pObj.mLeft;
    mRight = pObj.mRight;
}

Padding::Padding(int pTop, int pBottom, int pLeft, int pRight)
{
    mTop = pTop;
    mBottom = pBottom;
    mLeft = pLeft;
    mRight = pRight;
}

void Padding::show()
{
    printf("Padding: Top: %d, Bottom: %d, Left: %d, Right: %d \n", mTop, mBottom, mLeft, mRight);
}
