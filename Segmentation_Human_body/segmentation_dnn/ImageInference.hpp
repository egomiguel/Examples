#ifndef IMAGE_INFERENCE_H
#define IMAGE_INFERENCE_H

#include <opencv2/opencv.hpp>

class ImageSize;
class Padding;

class ImageInferenceProcess
{
public:
    ImageInferenceProcess(const cv::Mat& pBodyFullSegment, const cv::Mat& pBodyPartSegment, const ImageSize& pImageSize, const ImageSize& pModelInput, const Padding& pPadding);
    cv::Mat get_Mask(float threshold = 0.75);
    cv::Mat get_Parts_segmentation(const cv::Mat& mask);
private:
    cv::Mat mBody_full_segment, mBody_part_segment;

    ImageSize* mImageSize;

    ImageSize* mModelInput;

    Padding* mPadding;

    ImageSize get_image_size(const cv::Mat& pImage);
    
    cv::Mat resize_image_to(const cv::Mat& pImage, const ImageSize& pImageSize);
    
    void get_sigmoid(cv::Mat& pImage);
    
    cv::Mat remove_padding_and_resize_back(const cv::Mat& pImage, int original_height, int original_width, const Padding& padding);
    
    cv::Mat crop_and_resize_batch(const cv::Mat& pImage, std::vector<float>& pBox, const ImageSize& crop_size);
    
    cv::Mat scale_and_crop_to_input_tensor_shape(cv::Mat& pImage, bool apply_sigmoid_activation = false);

    cv::Mat get_arg_max_last(const cv::Mat& pImage);
};

#endif
