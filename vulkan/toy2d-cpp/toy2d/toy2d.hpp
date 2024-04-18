#pragma once
#include <memory>
#include <vulkan/vulkan.hpp>
#include "tool.hpp"
#include "render.hpp"
namespace toy2d
{
    void Init(std::vector<const char *> extensions, std::function<vk::SurfaceKHR(vk::Instance)> retsurface, int width, int height);
    void Quit();
    render &GetRenderInstance();
}