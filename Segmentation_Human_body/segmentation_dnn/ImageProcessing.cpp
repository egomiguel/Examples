#include "ImageProcessing.hpp"
#include "math.h"

float MOVILNET_SCALE = 127.5;
float MOVILNET_RESOLUTION = 0.5;
int MOVILNET_STRIDE = 16;


ImageProcessing::ImageProcessing()
{
}

std::pair<cv::Mat, Padding> ImageProcessing::get_processed_image(const cv::Mat& pImage, ImageSize& modelInputSize)
{
    cv::Mat rightImage;
    pImage.convertTo(rightImage, CV_32F);

    int height = rightImage.rows;
    int width = rightImage.cols;

    modelInputSize = get_input_resolution_height_and_width(MOVILNET_RESOLUTION, MOVILNET_STRIDE, height, width);

    std::pair<cv::Mat, Padding> info = pad_and_resize_to(rightImage, modelInputSize.mHeight, modelInputSize.mWidth);

    cv::Mat processed = get_mobileNet_processed_image(info.first);

    return std::make_pair(processed, info.second);
}

bool ImageProcessing::is_valid_input_resolution(float pResolution, int pOutputStride)
{
    return (int(pResolution - 1.0) % pOutputStride == 0);
}

int ImageProcessing::to_valid_input_resolution(float pResolution, int pOutputStride)
{
    if (is_valid_input_resolution(pResolution, pOutputStride) == true)
    {
        return int(pResolution);
    }
    
    return int(floor(pResolution / float(pOutputStride)) * pOutputStride + 1);
}

ImageSize ImageProcessing::get_input_resolution_height_and_width(float pInternal_resolution, int pOutput_stride, int pInput_height, int pInput_width)
{
    int height = to_valid_input_resolution(float(pInput_height) * pInternal_resolution, pOutput_stride);
    int width = to_valid_input_resolution(float(pInput_width) * pInternal_resolution, pOutput_stride);
    return ImageSize(height, width);
}

std::pair<cv::Mat, Padding> ImageProcessing::pad_and_resize_to(const cv::Mat& pImage, int pTarget_height, int pTarget_width)
{
    int input_height = pImage.rows;
    int input_width = pImage.cols;
    float target_aspect = float(pTarget_width) / float(pTarget_height);
    float aspect = float(input_width) / float(input_height);
    Padding padding;

    if (aspect < target_aspect)
    {
        padding = Padding(0, 0, round(MOVILNET_RESOLUTION * (target_aspect * input_height - input_width)),
            round(MOVILNET_RESOLUTION * (target_aspect * input_height - input_width)));
    }
    else
    {
        padding = Padding(round(MOVILNET_RESOLUTION * ((1.0 / target_aspect) * input_width - input_height)),
                          round(MOVILNET_RESOLUTION * ((1.0 / target_aspect) * input_width - input_height)),
                          0, 0);
    }

    cv::Mat paddedTemp, resized;
    cv::copyMakeBorder(pImage, paddedTemp, padding.mTop, padding.mBottom, padding.mLeft, padding.mRight, cv::BORDER_CONSTANT);

    cv::Mat padded;
    paddedTemp.convertTo(padded, CV_32F);

    cv::resize(padded, resized, cv::Size(pTarget_width, pTarget_height));

    return std::make_pair(resized, padding);
}

cv::Mat ImageProcessing::get_mobileNet_processed_image(const cv::Mat& pImage)
{
    cv::Mat result = pImage / MOVILNET_SCALE;
    result -= cv::Scalar(1, 1, 1);
    return result;
}