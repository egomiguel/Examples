#include <Eigen/Dense>
#include "opencv2/opencv.hpp"
#include "Filters.hpp"
#include "FiltersException.hpp"
#include "PrivateFilters.hpp"
#include <iostream>

Filters::Filters()
{
    mFilter = new PrivateFilters();
}

Filters::~Filters()
{
    mFilter = NULL;
    delete mFilter;
}

double Filters::getHeartRatePPGFromVideo(const std::string& pPath)
{
    cv::VideoCapture capture(pPath);

    if (!capture.isOpened())
    {
        throw FiltersException("Error opening video stream or file.");
    }

    double fps = capture.get(cv::CAP_PROP_FPS);

    cv::Mat frame;

    std::vector<double> Red, Green, Blue;

    while (true)
    {
        capture.read(frame);

        if (frame.empty())
            break;

        cv::Scalar tMean = cv::mean(frame);

        Blue.push_back(tMean[0]);
        Green.push_back(tMean[1]);
        Red.push_back(tMean[2]);
    }

    capture.release();

    Eigen::VectorXd signalRed = Eigen::Map<Eigen::VectorXd>(Red.data(), Red.size());
    Eigen::VectorXd signalGreen = Eigen::Map<Eigen::VectorXd>(Green.data(), Green.size());
    Eigen::VectorXd signalBlue = Eigen::Map<Eigen::VectorXd>(Blue.data(), Blue.size());

    Eigen::MatrixXd signal(Red.size(), 3);
    signal << signalRed, signalGreen, signalBlue;
    Eigen::MatrixXd result = mFilter->ICA(signal);

    std::vector<double> positiveFrequencies = mFilter->getPositiveFrequencyFFT(Red.size(), fps);
    std::vector<double> frequencies;
    double minFreq = 0.5;
    double maxFreq = 3.5;

    for (int i = 0; i < result.cols(); i++)
    {
        Eigen::VectorXd vector = result.col(i);
        std::vector<double> vectorSTD(vector.data(), vector.data() + vector.size());
        std::pair<std::vector<double>, std::vector<double>> hp = mFilter->filterHP(vectorSTD);
        std::vector<double> fft = mFilter->filterFFT(hp.second);
        std::pair<double, double> heartFreq(0, 0);
        if (mFilter->getMaxValueRangeXY(positiveFrequencies, fft, minFreq, maxFreq, heartFreq))
        {
            frequencies.push_back(heartFreq.first * 60.0);
        }
        else
        {
            throw FiltersException("Heart rate could not be calculated.");
        }
    }

    if (frequencies.size() == 1)
    {
        return frequencies[0];
    }
    else if (frequencies.size() == 2)
    {
        return (frequencies[0] + frequencies[1]) / 2.0;
    }
    else
    {
        std::sort(frequencies.begin(), frequencies.end());
        double diff = abs(frequencies[1] - frequencies[0]);
        int pos = 0;

        for (int i = 0; i < frequencies.size() - 1; i++)
        {
            if (abs(frequencies[i] - frequencies[i + 1]) < diff)
            {
                diff = abs(frequencies[i] - frequencies[i + 1]);
                pos = i;
            }
        }

        return (frequencies[pos] + frequencies[pos + 1]) / 2.0;
    }
}
