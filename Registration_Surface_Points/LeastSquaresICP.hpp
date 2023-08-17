#ifndef REGISTRATION_ICPLS_H
#define REGISTRATION_ICPLS_H

#include <opencv2/calib3d/calib3d.hpp>
#include "vtkSmartPointer.h"
#include "vtkImplicitPolyDataDistance.h"
#include "vtkPolyData.h"
#include "Types.hpp"

namespace TKA
{
	namespace REGISTRATION
	{
		class LeastSquaresICP
		{
		private:
			struct GaussNewton
			{
				cv::Mat A, B;
				double totalError;
				double localError;
				GaussNewton(const cv::Mat& A, const cv::Mat& B, double totalError, double localError)
				{
					this->A = A;
					this->B = B;
					this->totalError = totalError;
					this->localError = localError;
				}
			};

			double chi2;

			double maxError;

			cv::Mat Rx(double angle);

			cv::Mat Ry(double angle);

			cv::Mat Rz(double angle);

			cv::Mat DRx(double angle);

			cv::Mat DRy(double angle);

			cv::Mat DRz(double angle);

			cv::Mat CreatePoint(double x, double y, double z);

			cv::Mat CreatePoint(cv::Point3d Point);

			cv::Mat DF_Rx(double angleX, double angleY, double angleZ, const cv::Mat& Point);

			cv::Mat DF_Ry(double angleX, double angleY, double angleZ, const cv::Mat& Point);

			cv::Mat DF_Rz(double angleX, double angleY, double angleZ, const cv::Mat& Point);

			cv::Mat Jacobian(const cv::Mat& data, const cv::Mat& Point);

			cv::Mat JacobianScale(const cv::Mat& data, const cv::Mat& Point);

			cv::Mat SquareError(const cv::Mat& data, const cv::Mat& source, const cv::Mat& target);

			cv::Mat SquareErrorScale(const cv::Mat& data, const cv::Mat& source, const cv::Mat& target);

			void GetScale(const std::vector<cv::Point3d>& target, cv::Mat& data);

			GaussNewton GetSystem(const std::vector<cv::Point3d>& target, const cv::Mat& data, int posBegin, int posEnd, double lambda = 0);

			GaussNewton GetSystemScale(const std::vector<cv::Point3d>& target, const cv::Mat& data, int posBegin, int posEnd, double lambda = 0);

			GaussNewton GetSystem(const std::vector<PointTypeITK>& target, const cv::Mat& data, int posBegin, int posEnd, double lambda = 0);

			cv::Point3d ClosestPoint(const vtkSmartPointer<vtkPolyData>& surface, double point[3]);

			std::vector<cv::Point3d> GetCorrespondence(const vtkSmartPointer<vtkImplicitPolyDataDistance> implicitPolyDataDistance, const cv::Mat& data);

			std::vector<cv::Point3d> GetCorrespondenceScale(const vtkSmartPointer<vtkImplicitPolyDataDistance> implicitPolyDataDistance, const cv::Mat& data);

			void shuffleSource();

			void shuffleCenterSource();

			std::vector<cv::Point3d> source;
			std::vector<cv::Point3d> centerSource;
			cv::Point3d aveSource;
		public:
			LeastSquaresICP(const std::vector<PointTypeITK>& sourcePoints);

			LeastSquaresICP(const std::vector<cv::Point3d>& sourcePoints);

			double LeastSquares(const vtkSmartPointer<vtkPolyData>& surface, cv::Mat& data, int iterations = 200);

			double LeastSquaresScale(const vtkSmartPointer<vtkPolyData>& surface, cv::Mat& data, int iterations = 200);

			double LeastSquaresRandomInit(const vtkSmartPointer<vtkPolyData>& surface, cv::Mat& data, int iterations = 200);

			double LeastSquaresScaleRandomInit(const vtkSmartPointer<vtkPolyData>& surface, cv::Mat& data, int iterations = 200);

			double LeastSquaresTest(const vtkSmartPointer<vtkPolyData>& surface, cv::Mat& data, int iterations = 100);

			cv::Mat GetRotationMatrix(double angleX, double angleY, double angleZ);

			void setChi2(double pChi2);

			void setMaxError(double pMaxError);

			double getChi2() const;

			double getMaxError() const;

			static cv::Mat GetRotationAnglesXYZ(const std::vector<cv::Point3d>& threeVectorsSource, const std::vector<cv::Point3d>& threeVectorstarget, cv::Mat& data);
		};
	}
}

#endif