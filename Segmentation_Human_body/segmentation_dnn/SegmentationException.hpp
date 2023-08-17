#ifndef EXCEPTION_SEGMENTATION_H
#define EXCEPTION_SEGMENTATION_H

#include <exception>
#include <string>

class SegmentationException : public std::exception
{
    std::string msg_;
public:
    SegmentationException(const std::string& msg) : msg_(msg) {}

    virtual const char* what() const noexcept override
    {
        return msg_.c_str();
    }
};

#endif