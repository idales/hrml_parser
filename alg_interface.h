#pragma once
#include <string>

struct ITagValue
{
    virtual std::string get_value(const std::string& ) = 0;
    virtual ~ITagValue() = default;
};
