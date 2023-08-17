#ifndef PRIVATE_FILTERS_H
#define PRIVATE_FILTERS_H

#include <Eigen/Dense>
#include <vector>

class PrivateFilters
{
public:
    PrivateFilters();

    Eigen::MatrixXd ICA(const Eigen::MatrixXd& pX, int maxIter = 250, double tol = 0.0001);

    std::vector<double> filterFFT(const std::vector<double>& pData);

    std::vector<double> getPositiveFrequencyFFT(int pSiganlSize, double pSamplingRate);

    Eigen::MatrixXd makeDiagonalMatrix(Eigen::VectorXd pVector, int k);

    std::pair<std::vector<double>, std::vector<double>> filterHP(const std::vector<double>& pData, double pSmoothing = 14400);

    void writeVector(const Eigen::VectorXd& pX, const std::string& path);

    void writeVector(const std::vector<double>& pX, const std::string& path);

    bool getMaxValueRangeXY(const std::vector<double>& pX, const std::vector<double>& pY, double pMinX, double pMaxX, std::pair<double, double>& result);

private:
    Eigen::MatrixXd sym_decorrelation(const Eigen::MatrixXd& pW);

    std::pair<Eigen::MatrixXd, Eigen::RowVectorXd> logcosh(const Eigen::MatrixXd& pX);

    Eigen::MatrixXd ICA_Par(const Eigen::MatrixXd& pX, double tol, int maxIter, const Eigen::MatrixXd& pInitW);

    Eigen::MatrixXd getEigenVectorsPCA(const Eigen::MatrixXd& pMatrix);
};




#endif