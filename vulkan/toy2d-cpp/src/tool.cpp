#include <iostream>
#include "../toy2d/tool.hpp"

namespace toy2d
{
    std::string ReadWholeFile(const std::string &filepath)
    {
        std::ifstream file(filepath, std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            std::cout << "read " << filepath << "failed" << std::endl;
            return std::string{};
        }

        auto size = file.tellg();
        std::string content;
        content.resize(size);
        file.seekg(0);
        file.read(content.data(), size);
        return content;
    }
}