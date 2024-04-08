#pragma once
#include <memory>
#include <vulkan/vulkan.hpp>

namespace toy2d
{
    void Init(std::vector<const char *> extensions, std::function<vk::SurfaceKHR(vk::Instance)> retsurface);
    void Quit();
}