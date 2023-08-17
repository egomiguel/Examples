#include "ImageInference.hpp"
#include "ImageSize.hpp"
#include "Padding.hpp"

const int labels = 24;

ImageInferenceProcess::ImageInferenceProcess(const cv::Mat& pBodyFullSegment, const cv::Mat& pBodyPartSegment, const ImageSize& pImageSize, const ImageSize& pModelInput, const Padding& pPadding)
{
    mBody_full_segment = pBodyFullSegment;
    mBody_part_segment = pBodyPartSegment;
    mImageSize = new ImageSize(pImageSize);
    mModelInput = new ImageSize(pModelInput);
    mPadding = new Padding(pPadding);
}

cv::Mat ImageInferenceProcess::get_Mask(float threshold)
{
    cv::Mat inference = scale_and_crop_to_input_tensor_shape(mBody_full_segment, true);
    cv::Mat result = (inference > threshold);
    return result;
}

cv::Mat ImageInferenceProcess::get_Parts_segmentation(const cv::Mat& mask)
{
    cv::Mat inference = scale_and_crop_to_input_tensor_shape(mBody_part_segment, true);
    
    int channel = inference.channels();

    if (channel == 1)
    {
        return inference;
    }

    cv::Mat rightImage = get_arg_max_last(inference);

    rightImage += 1;
    cv::Mat tempMat = mask / 255;

    cv::Mat part_segmentation = (rightImage.mul(tempMat)) * (255 / labels);

    return part_segmentation;
}

cv::Mat ImageInferenceProcess::get_arg_max_last(const cv::Mat& pImage)
{
    int rows = pImage.rows;
    int cols = pImage.cols;
    int channel = pImage.channels();
    int64_t tSize = rows * cols;

    float *input = (float*)(pImage.data);
    uchar* arrayTemp = new uchar[tSize];

    int64_t cont = 0;
    
    if (channel > 1)
    {
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                uchar pos = 0;
                cv::Vec<float, labels> elem = pImage.at<cv::Vec<float, labels>>(i, j);
                for (uchar k = 1; k < labels; k++)
                {
                    if (elem[k] > elem[pos])
                    {
                        pos = k;
                    }
                }

                arrayTemp[cont++] = pos;
            }
        }

        return cv::Mat(rows, cols, CV_8U, arrayTemp);
    }
    else
    {
        return pImage;
    }
    
}

ImageSize ImageInferenceProcess::get_image_size(const cv::Mat& pImage)
{
    int height = pImage.rows;
    int width = pImage.cols;
    return ImageSize(height, width);
}

cv::Mat ImageInferenceProcess::resize_image_to(const cv::Mat& pImage, const ImageSize& pImageSize)
{
    cv::Mat rightImage;
    pImage.convertTo(rightImage, CV_32F);

    if (get_image_size(pImage) == pImageSize)
    {
        return rightImage;
    }

    cv::Mat output;
    cv::resize(rightImage, output, cv::Size(pImageSize.mWidth, pImageSize.mHeight));
    return output;
}

void ImageInferenceProcess::get_sigmoid(cv::Mat& pImage)
{
    float *input = (float*)(pImage.data);
    int64_t tSize = pImage.cols * pImage.rows * pImage.channels();

    for (int64_t i = 0; i < tSize; i++)
    {
        float val = 1. / (1. + exp(-(input[i])));
        input[i] = val;
    } 
}

cv::Mat ImageInferenceProcess::remove_padding_and_resize_back(const cv::Mat& pImage, int original_height, int original_width, const Padding& padding)
{
    std::vector<float> box;
    box.push_back(padding.mTop / (original_height + padding.mTop + padding.mBottom - 1.0));
    box.push_back(padding.mLeft / (original_width + padding.mLeft + padding.mRight - 1.0));
    box.push_back((padding.mTop + original_height - 1.0) / (original_height + padding.mTop + padding.mBottom - 1.0));
    box.push_back((padding.mLeft + original_width - 1.0) / (original_width + padding.mLeft + padding.mRight - 1.0));

    return crop_and_resize_batch(pImage, box, ImageSize(original_height, original_width));
}

cv::Mat ImageInferenceProcess::crop_and_resize_batch(const cv::Mat& pImage, std::vector<float>& pBox, const ImageSize& crop_size)
{
    assert(pBox.size() == 4);

    float y1, x1, y2, x2;
    y1 = pBox[0];
    x1 = pBox[1];
    y2 = pBox[2];
    x2 = pBox[3];

    assert(std::max(pBox) <= 1);
    assert(std::min(pBox) >= 0);
    assert(y1 <= y2);
    assert(x1 <= x2);

    ImageSize image_size = get_image_size(pImage);
    int image_y1 = y1 * (float(image_size.mHeight) - 1.);
    int image_y2 = y2 * (float(image_size.mHeight) - 1.);
    int image_x1 = x1 * (float(image_size.mWidth) - 1.);
    int image_x2 = x2 * (float(image_size.mWidth) - 1.);

    cv::Mat cropped_image = pImage(cv::Range(image_y1, 1 + image_y2), cv::Range(image_x1, 1 + image_x2));

    cv::Mat resized_cropped_image = resize_image_to(cropped_image, ImageSize(crop_size.mHeight, crop_size.mWidth));

    return resized_cropped_image;
}

cv::Mat ImageInferenceProcess::scale_and_crop_to_input_tensor_shape(cv::Mat& pImage, bool apply_sigmoid_activation)
{
    cv::Mat resized_image = resize_image_to(pImage, *mModelInput);

    if (apply_sigmoid_activation == true)
    {
        get_sigmoid(resized_image);
    }

    return remove_padding_and_resize_back(resized_image, mImageSize->mHeight, mImageSize->mWidth, *mPadding);
}