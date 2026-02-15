#include "ConversionHelpers.hpp"

#include <algorithm>
#include <iterator>

std::vector<const char*> BAV::ConversionHelpers::StringVectorToCharVector(const std::vector<std::string>& stringVector)
{
    std::vector<const char*> charVector(stringVector.size());

    std::ranges::transform(
        stringVector,
        charVector.begin(),
        [](const std::string& s) { return s.c_str(); });

    return charVector;
}
