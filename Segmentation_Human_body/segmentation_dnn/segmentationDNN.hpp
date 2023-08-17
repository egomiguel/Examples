#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include <string>
#include<opencv2/opencv.hpp>

class PrivateData;

class SegmentationDNN
{
public:
    SegmentationDNN(const std::string& pModelPB);
    
    ~SegmentationDNN();

    bool Execute(const cv::Mat& pImg);

    cv::Mat getBodyMask();

    cv::Mat getBodyParts();

    static void getVersion();

private:
    PrivateData* mData;
    cv::Mat mMask, mParts;
    cv::Mat GetPrediction(const std::string& pInputLayer, const std::string& pOutputLayer, const std::vector<std::int64_t>& pInput_dims, const float* pData, size_t pSize);
};

#endif
