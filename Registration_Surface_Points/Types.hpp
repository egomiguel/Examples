#ifndef REGISTRATION_TYPES_H
#define REGISTRATION_TYPES_H

#include "itkImage.h"

namespace TKA
{
	namespace REGISTRATION
	{
		using RegistrationPixelType = int16_t;
		using RegistrationImageType = itk::Image<RegistrationPixelType, 3>;
		using PointTypeITK = itk::Point<double, 3>;
	}
}

#endif