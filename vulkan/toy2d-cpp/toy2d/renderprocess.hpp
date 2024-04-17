#pragma once
#include <memory>
#include <vulkan/vulkan.hpp>

namespace toy2d
{
    class RenderProcess final
    {
    public:
        vk::RenderPass renderPass;
        vk::Pipeline pipeline;
        vk::PipelineLayout pipelineLayout;
        void InitRenderPass();
        void InitRenderPassLayout();
        void InitPipeline(int width, int height);
        RenderProcess();
        ~RenderProcess();

    private:
    };
}