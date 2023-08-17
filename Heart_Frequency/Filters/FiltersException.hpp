#ifndef EXCEPTION_FILTERS_H
#define EXCEPTION_FILTERS_H

#include <exception>
#include <string>

class FiltersException : public std::exception
{
    std::string msg_;
public:
    FiltersException(const std::string& msg) : msg_(msg) {}

    virtual const char* what() const noexcept override
    {
        return msg_.c_str();
    }
};

#endif
