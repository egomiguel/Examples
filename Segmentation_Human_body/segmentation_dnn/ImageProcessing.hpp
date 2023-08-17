#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include <opencv2/opencv.hpp>
#include "ImageSize.hpp"
#include "Padding.hpp"

class ImageProcessing
{
public:
    ImageProcessing();

    std::pair<cv::Mat, Padding> get_processed_image(const cv::Mat& pImage, ImageSize& modelInputSize);

private:
    bool is_valid_input_resolution(float pResolution, int pOutputStride);

    int to_valid_input_resolution(float pResolution, int pOutputStride);

    ImageSize get_input_resolution_height_and_width(float pInternal_resolution, int pOutput_stride, int pInput_height, int pInput_width);
    
    std::pair<cv::Mat, Padding> pad_and_resize_to(const cv::Mat& pImage, int pTarget_height, int pTarget_width);
    
    cv::Mat get_mobileNet_processed_image(const cv::Mat& pImage);
};

#endif