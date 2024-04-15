#pragma once
#include <memory>
#include <vulkan/vulkan.hpp>

namespace toy2d
{
    class RenderProcess final
    {
    public:
        vk::RenderPass renderPass;
        vk::Pipeline pipline;
        vk::PipelineLayout pipelineLayout;
        static void Init();
        void InitRenderPass();
        void InitRenderPassLayout();
        void InitPipeline();
        static RenderProcess &GetInstance();
        ~RenderProcess();

    private:
        RenderProcess();
        static std::unique_ptr<RenderProcess> rendprocess;
    };
}