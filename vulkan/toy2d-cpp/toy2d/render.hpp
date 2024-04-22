#pragma once
#include <iostream>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace toy2d
{
    class render final
    {
    public:
        void DrawColorTriangle();
        render(uint32_t maxFlightCount = 2);
        ~render();

        vk::CommandPool cmdpool_;
        std::vector<vk::CommandBuffer> cmdbuffer_;
        std::vector<vk::Semaphore> imageAvaliable_;
        std::vector<vk::Semaphore> imageDrawFinsh_;
        std::vector<vk::Fence> fence_;
        void InitCmdPool();
        void InitCmdBuffer();

    private:
        uint32_t maxFlightCount_;
        uint32_t currentFrame_ = 0;
    };
}