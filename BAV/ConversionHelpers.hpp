#pragma once
#include <string>
#include <vector>

namespace BAV
{
    class ConversionHelpers
    {
    public:
        static std::vector<const char*> StringVectorToCharVector(const std::vector<std::string>& stringVector);

    };
}
