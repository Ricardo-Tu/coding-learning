#pragma once
#include <iostream>
#include <vector>
#include <chrono>
#include <vulkan/vulkan.hpp>

namespace toy2d
{
    class render final
    {
    public:
        void UpdateUniformBuffer(uint32_t CurrentUniformBufIndex);
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
        void createSampler();
        std::vector<vk::CommandBuffer> CreateCommandBuffer(uint32_t CommandBufferCount);
        uint32_t maxFramesCount_ = 2;

    private:
        uint32_t currentFrame_ = 0;
    };
}