#include "PrivateFilters.hpp"
#include <EigenRand/EigenRand>
#include <iostream>
#include <opencv2/core.hpp>
#include <unsupported/Eigen/FFT>

PrivateFilters::PrivateFilters()
{
}

void showMatrix(Eigen::MatrixXd M)
{
    std::cout << M << std::endl;
}

Eigen::MatrixXd PrivateFilters::ICA(const Eigen::MatrixXd& pX, int maxIter, double tol)
{
    Eigen::MatrixXd XT = pX.transpose();
    int n_features = XT.rows();
    int n_samples = XT.cols();

    int components = std::min(n_features, n_samples);

    XT = XT.colwise() - XT.rowwise().mean();
    
    Eigen::JacobiSVD<Eigen::MatrixXd, Eigen::FullPivHouseholderQRPreconditioner> svd(XT, Eigen::ComputeThinU);

    Eigen::MatrixXd U = svd.matrixU();
    Eigen::RowVectorXd D = svd.singularValues();

    Eigen::MatrixXd K = (U.array().rowwise() / D.array()).transpose();

    Eigen::MatrixXd X1 = K * XT;

    X1 = X1 * sqrt(double(n_samples));

    Eigen::Rand::Vmt19937_64 urng; 

    Eigen::MatrixXd w_init = Eigen::Rand::normal<Eigen::MatrixXd>(components, components, urng, 0, 1.0);

    Eigen::MatrixXd W = ICA_Par(X1, tol, maxIter, w_init);

    Eigen::MatrixXd result = (W * K * XT).transpose();

    return result;
}

Eigen::MatrixXd PrivateFilters::sym_decorrelation(const Eigen::MatrixXd& pW)
{
    Eigen::MatrixXd tMatrix = pW * pW.transpose();
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eig(tMatrix);
    
    Eigen::MatrixXd eigVectors = eig.eigenvectors();
    Eigen::VectorXd eigValuesPros = (1.0 / eig.eigenvalues().array().sqrt());
    Eigen::MatrixXd B = eigVectors.array().rowwise() * eigValuesPros.transpose().array();
    Eigen::MatrixXd A = B * eigVectors.transpose() * pW;

    return A;
}

std::pair<Eigen::MatrixXd, Eigen::RowVectorXd> PrivateFilters::logcosh(const Eigen::MatrixXd& pX)
{
    Eigen::MatrixXd gx = Eigen::tanh(pX.array());
    int rows = gx.rows();
    Eigen::VectorXd vect(rows);
    for (int i = 0; i < rows; i++)
    {
        vect(i) = (1.0 - gx.row(i).array().pow(2)).mean();
    }

    return std::make_pair(gx, vect);
}

Eigen::MatrixXd PrivateFilters::ICA_Par(const Eigen::MatrixXd& pX, double tol, int maxIter, const Eigen::MatrixXd& pInitW)
{
    Eigen::MatrixXd W = sym_decorrelation(pInitW);

    float p = float(pX.cols());
    float lim;
    for (int i = 0; i < maxIter; i++)
    {
        std::pair<Eigen::MatrixXd, Eigen::RowVectorXd> tResult = logcosh(W * pX);
        Eigen::MatrixXd gwtx = tResult.first;
        Eigen::VectorXd g_wtx = tResult.second;

        Eigen::MatrixXd tCal1 = W.array().colwise() * g_wtx.array();

        Eigen::MatrixXd tCal2 = (gwtx * pX.transpose()) / p - tCal1;

        Eigen::MatrixXd W1 = sym_decorrelation(tCal2);

        lim = (Eigen::abs(Eigen::abs((W1 * W.transpose()).diagonal().array()) - 1)).maxCoeff();
        W = W1;

        if (lim <= tol)
        {
            break;
        }
    }

    if (lim > tol)
    {
        printf("FastICA did not converge. Consider increasing tolerance or the maximum number of iterations.");
    }

    return W;
}

std::vector<double> PrivateFilters::filterFFT(const std::vector<double>& pData)
{
    int64_t tSize = pData.size();
    std::vector<double> resultFFT;

    if (tSize < 2)
    {
        return resultFFT;
    }

    std::vector<double> temp = pData;
    Eigen::VectorXd signal = Eigen::Map<Eigen::VectorXd>(temp.data(), temp.size());

    Eigen::FFT<double> fft;

    Eigen::VectorXcd result;

    fft.fwd(result, signal);

    for (int64_t i = 0; i < tSize; i++)
    {
        double val = pow(result(i).real(), 2) + pow(result(i).imag(), 2);
        resultFFT.push_back(sqrt(val));
    }

    return resultFFT;
}

Eigen::MatrixXd PrivateFilters::makeDiagonalMatrix(Eigen::VectorXd pVector, int k)
{
    int absK = abs(k);
    int64_t tSize = pVector.size();
    Eigen::MatrixXd newMatrix = Eigen::MatrixXd::Zero(tSize + absK, tSize + absK);
    int64_t row, col;

    if (k >= 0)
    {
        row = 0;
        col = absK;
    }
    else
    {
        row = absK;
        col = 0;
    }

    for (int64_t i = row; i < tSize + row; i++)
    {
        newMatrix(i, col) = pVector(i - row);
        col++;
    }
    return newMatrix;
}

std::pair<std::vector<double>, std::vector<double>> PrivateFilters::filterHP(const std::vector<double>& pData, double pSmoothing)
{
    const int64_t tSize = pData.size();

    if (tSize < 3)
    {
        std::vector<double> cyclical(tSize, 0);
        return std::make_pair(pData, cyclical);
    }

    std::vector<double> tSignal = pData;
    Eigen::VectorXd signal = Eigen::Map<Eigen::VectorXd>(tSignal.data(), tSignal.size());

    double a = 6. * pSmoothing + 1.;
    double b = -4. * pSmoothing;
    double c = pSmoothing;

    Eigen::MatrixXd temp(1, 3);
    temp(0, 0) = c;
    temp(0, 1) = b;
    temp(0, 2) = a;

    temp = Eigen::MatrixXd::Ones(tSize, 1) * temp;

    Eigen::MatrixXd matrix = makeDiagonalMatrix(temp.col(2), 0) + makeDiagonalMatrix(temp.block(0, 1, tSize - 1, 1), 1);

    matrix = matrix + makeDiagonalMatrix(temp.block(0, 1, tSize - 1, 1), -1);

    matrix = matrix + makeDiagonalMatrix(temp.block(0, 0, tSize - 2, 1), 2);

    matrix = matrix + makeDiagonalMatrix(temp.block(0, 0, tSize - 2, 1), -2);

    matrix(0, 0) = 1. + pSmoothing;
    matrix(1, 0) = -2. * pSmoothing;

    matrix(0, 1) = -2. * pSmoothing;
    matrix(1, 1) = 5. * pSmoothing + 1.;

    matrix(tSize - 2, tSize - 2) = 5. * pSmoothing + 1.;
    matrix(tSize - 1, tSize - 2) = -2. * pSmoothing;

    matrix(tSize - 2, tSize - 1) = -2. * pSmoothing;
    matrix(tSize - 1, tSize - 1) = 1. + pSmoothing;

    Eigen::MatrixXd result = matrix.inverse() * signal;
    Eigen::MatrixXd tCyclical = signal - result;

    std::vector<double> trend(result.data(), result.data() + result.rows() * result.cols());
    std::vector<double> cyclical(tCyclical.data(), tCyclical.data() + tCyclical.rows() * tCyclical.cols());

    return std::make_pair(trend, cyclical);
}

Eigen::MatrixXd PrivateFilters::getEigenVectorsPCA(const Eigen::MatrixXd& pMatrix)
{
    Eigen::MatrixXd centered = pMatrix.rowwise() - pMatrix.colwise().mean();
    Eigen::MatrixXd cov = centered.adjoint() * centered;
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eig(cov);
    return eig.eigenvectors();
}

std::vector<double> PrivateFilters::getPositiveFrequencyFFT(int pSiganlSize, double pSamplingRate)
{
    std::vector<double> result;
    double sampleSpacing = 1.0 / pSamplingRate;
    double val = 1.0 / (pSiganlSize * sampleSpacing);

    int positiveSize = ((pSiganlSize - 1) / 2) + 1;
    int negativeSize = (pSiganlSize / 2);

    for (int i = 0; i < positiveSize; i++)
    {
        double freq = double(i) * val;
        result.push_back(freq);
    }

    /*for (int i = -negativeSize; i < 0; i++)
    {
        double freq = double(i) * val;
        result.push_back(freq);
    }*/

    return result;
}

bool PrivateFilters::getMaxValueRangeXY(const std::vector<double>& pX, const std::vector<double>& pY, double pMinX, double pMaxX, std::pair<double, double>& result)
{
    bool isFine = false;
    int64_t tSize1 = pX.size();
    int64_t tSize2 = pY.size();
    double temp = 0;
    for (int i = 0; (i < tSize1) && (i < tSize2); i++)
    {
        if (pX[i] >= pMinX && pX[i] <= pMaxX)
        {
            if (isFine == false || temp < pY[i])
            {
                temp = pY[i];
                result.first = pX[i];
                result.second = pY[i];
                isFine = true;
            }
        }
    }

    return isFine;
}

void PrivateFilters::writeVector(const Eigen::VectorXd& pX, const std::string& path)
{
    std::ofstream MyFile(path);
    int64_t tSize = pX.size();
    for (int i = 0; i < tSize; i++)
    {
        MyFile << pX[i] << "\n";
    }
    MyFile.close();
}

void PrivateFilters::writeVector(const std::vector<double>& pX, const std::string& path)
{
    std::ofstream MyFile(path);
    int64_t tSize = pX.size();
    for (int i = 0; i < tSize; i++)
    {
        MyFile << pX[i] << "\n";
    }
    MyFile.close();
}