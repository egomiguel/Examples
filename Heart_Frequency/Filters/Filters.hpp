#ifndef FILTERS_H
#define FILTERS_H

#include <vector>
#include "HeartRatePPG_export.h"

class PrivateFilters;

class HEARTRATEPPG_EXPORT Filters
{
public:
    Filters();

    ~Filters();

    double getHeartRatePPGFromVideo(const std::string& pPath);

private:
    PrivateFilters * mFilter;
};

#endif