#pragma once
#include <iostream>
#include <vulkan/vulkan.hpp>

namespace toy2d
{
    class render final
    {
    public:
        void Render();
        render();
        ~render();

        vk::CommandPool cmdpool_;
        vk::CommandBuffer cmdbuffer_;
        vk::Semaphore imageAvaliable_;
        vk::Semaphore imageDrawFinsh_;
        vk::Fence fence_;
        void InitCmdPool();
        void InitCmdBuffer();

    private:
    };
}