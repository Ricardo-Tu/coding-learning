#pragma once
#include <memory>
#include <vulkan/vulkan.hpp>
#include <SDL2/SDL.h>
#include <functional>
#include "tool.hpp"
#include "render.hpp"
namespace toy2d
{
    extern SDL_Window *window;
    extern bool framebufferResizeFlag;
    extern uint32_t WindowWidth;
    extern uint32_t WindowHeight;

    void Init(std::vector<const char *> extensions, std::function<vk::SurfaceKHR(vk::Instance)> retsurface, int width, int height);
    void Quit();
    void reCreateSwapChain();
    render &GetRenderInstance();
}