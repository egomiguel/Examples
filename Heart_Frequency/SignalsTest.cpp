#include <iostream>
#include <fstream>
#include "Filters/Filters.hpp"


int main()
{
    Filters filter;

    double frequency = filter.getHeartRatePPGFromVideo("video.mp4");

    std::cout << "Heart frequency is about " << frequency << std::endl;

    return 0;
}