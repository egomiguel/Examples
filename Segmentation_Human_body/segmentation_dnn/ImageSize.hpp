#ifndef IMAGE_SIZE_H
#define IMAGE_SIZE_H

class ImageSize
{
public:
    int mHeight, mWidth;

    ImageSize(const ImageSize& pObj);

    ImageSize(int pHeight = 0, int pWidth = 0);

    bool operator==(const ImageSize& other);

    void show();
};


#endif