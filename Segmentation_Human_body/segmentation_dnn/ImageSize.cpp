#include "ImageSize.hpp"
#include <stdio.h>

ImageSize::ImageSize(const ImageSize& pObj)
{
    mHeight = pObj.mHeight;
    mWidth = pObj.mWidth;
}

ImageSize::ImageSize(int pHeight, int pWidth)
{
    mHeight = pHeight;
    mWidth = pWidth;
}

bool ImageSize::operator==(const ImageSize& other)
{
    if (mHeight == other.mHeight && mWidth == other.mWidth)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void ImageSize::show()
{
    printf("Image Size: Height: %d, Width: %d \n", mHeight, mWidth);
}


