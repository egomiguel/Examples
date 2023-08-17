#include "segmentationDNN.hpp"
#include "PrivateData.hpp"
#include "tensorflow/c/c_api.h"
#include <stdio.h>
#include <functional> 
#include <algorithm>
#include "ImageProcessing.hpp"
#include "ImageInference.hpp"
#include <fstream>
#include "SegmentationException.hpp"

std::string LAYER_INPUT = "sub_2";
std::string LAYER_OUTPUT_MASK = "float_segments";
std::string LAYER_OUTPUT_PART = "float_part_heatmaps";


SegmentationDNN::SegmentationDNN(const std::string& pModelPB)
{
    mData = new PrivateData();

    bool result = mData->init(pModelPB);

    if (result == false)
    {
        throw SegmentationException("Could not load or read model parameters.");
    }
}

SegmentationDNN::~SegmentationDNN()
{
    mData->~PrivateData();

    mData = NULL;
    delete mData;
}


cv::Mat SegmentationDNN::getBodyMask()
{
    return mMask;
}

cv::Mat SegmentationDNN::getBodyParts()
{
    return mParts;
}

bool SegmentationDNN::Execute(const cv::Mat& pImg)
{
    ImageSize originalSize(pImg.rows, pImg.cols);
    ImageSize modelInputSize;

    ImageProcessing process;
    std::pair<cv::Mat, Padding> result = process.get_processed_image(pImg, modelInputSize);

    int height = result.first.rows;
    int width = result.first.cols;
    int channel = result.first.channels();

    const std::vector<std::int64_t> input_dims = { 1, height, width, channel };

    float* buffer = (float*)result.first.data;
    int64 tSize = height * width * channel;

    cv::Mat outputMask = GetPrediction(LAYER_INPUT, LAYER_OUTPUT_MASK, input_dims, buffer, sizeof(float) * tSize);

    cv::Mat outputPart = GetPrediction(LAYER_INPUT, LAYER_OUTPUT_PART, input_dims, buffer, sizeof(float) * tSize);

    ImageInferenceProcess inference(outputMask, outputPart, originalSize, modelInputSize, result.second);

    mMask = inference.get_Mask();

    mParts = inference.get_Parts_segmentation(mMask);

    return true;
}

cv::Mat SegmentationDNN::GetPrediction(const std::string& pInputLayer, const std::string& pOutputLayer, const std::vector<std::int64_t>& pInput_dims, const float* pData, size_t pSize)
{

    TF_Output t0 = { TF_GraphOperationByName(mData->getGraph(), pInputLayer.c_str()), 0 };
    TF_Output t1 = { TF_GraphOperationByName(mData->getGraph(), pOutputLayer.c_str()), 0 };

    if (t0.oper == NULL)
    {
        throw SegmentationException("Failed TF_GraphOperationByName Input.");
    }

    if (t1.oper == NULL)
    {
        throw SegmentationException("Failed TF_GraphOperationByName Output.");
    }

    const std::vector<TF_Output> input_ops = { t0 };
    const std::vector<TF_Tensor*> input_tensors = { mData->CreateTensor(TF_FLOAT, pInput_dims, pData, pSize) };

    const std::vector<TF_Output> out_ops = { t1 };
    std::vector<TF_Tensor*> output_tensors = { nullptr };

    auto code = mData->Execute(input_ops, input_tensors, out_ops, output_tensors);

    int64_t dim1 = TF_Dim(output_tensors[0], 1);
    int64_t dim2 = TF_Dim(output_tensors[0], 2);
    int64_t dim3 = TF_Dim(output_tensors[0], 3);
    int64_t totalLenght = dim1 * dim2 * dim3;

    auto result = mData->GetTensorData<float>(output_tensors[0]);
    float * rawData = result.data();

    cv::Mat outputMatrix;

    std::vector<cv::Mat> channelsList;
    int cont = 0;
    int64_t tSize = dim1 * dim2;

    for (int64_t i = 0; i < dim3; i++)
    {
        float* arrayTemp = new float[tSize];
        cont = 0;

        for (int64_t j = i; j < totalLenght; j += dim3)
        {
            arrayTemp[cont] = rawData[j];
            cont++;
        }
        channelsList.push_back(cv::Mat(dim1, dim2, CV_32F, arrayTemp));
    }

    cv::merge(channelsList, outputMatrix);

    return outputMatrix;
}

void SegmentationDNN::getVersion()
{
    printf("Hello from TensorFlow C library version %s\n", TF_Version());
}