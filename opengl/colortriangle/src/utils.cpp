
#include "../colortriangle/utils.hpp"

std::string readfile(const std::string &filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        throw std::runtime_error("read shader failed");
    }

    std::stringstream sstr;
    sstr << file.rdbuf();
    std::string ret = sstr.str();
    return ret;
}