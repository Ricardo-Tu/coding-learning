#include "../toy2d/context.hpp"
#include "../toy2d/shader.hpp"
#include "../toy2d/renderpass.hpp"

namespace toy2d
{
    std::unique_ptr<RenderProcess> RenderProcess::renderprocess = nullptr;
    RenderProcess &RenderProcess::GetInstance()
    {
        return *rendprocess;
    }

    void RenderProcess::Init()
    {
        instance_.reset(new RenderProcess());
    }

    void RenderProcess::InitRenderPass()
    {
        vk::RenderPassCreateInfo renderPassCreateInfo;

        // 1. color attachment description
        vk::AttachmentDescription colorAttachment;
        colorAttachment.setFormat(Context::GetInstance().swapchain->info.format)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setSamples(vk::SampleCountFlagBits::e1);

        // 2. color attachment reference
        vk::AttachmentReference colorAttachmentRef;
        colorAttachmentRef.setAttachment(0)
            .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

        // 3. subpass
        vk::SubpassDescription subpass;
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.setColorAttachmentCount(1)
            .setColorAttachments(&colorAttachment);

        // 3. renderpass
        renderPassCreateInfo.setSubpasses(subpass)
            .setAttachmentCount(1);

        // 4. subpass dependency
        vk::SubpassDependency dependency;
        dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
            .setDstSubpass(0)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
        renderPassCreateInfo.setDependencies(dependency);

        // 5. create renderpass
        renderPass = Context::GetInstance().logicaldevice.createRenderPass(renderPassCreateInfo);
    }

    void RenderProcess::InitRenderPassLayout()
    {
        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayout = Context::GetInstance().logicaldevice.createPipelineLayout(pipelineLayoutCreateInfo);
    }

    void RenderProcess::InitPipeline()
    {
        vk::GraphicsPipelineCreateInfo createinfo;

        // 1. vertex info
        vk::PipelineVertexInputStateCreateInfo inputstate;
        createinfo.setPVertexInputState(&inputstate);

        // 2. vertex assembly
        vk::PipelineInputAssemblyStateCreateInfo inputAsm;
        inputAsm.setPrimitiveRestartEnable(false)
            .setTopology(vk::PrimitiveTopology::eTriangleList);
        createinfo.setPInputAssemblyState(&inputAsm);        

        // 3. shader
        auto stages = Shader::GetInstance().GetStage();
    }

    RenderProcess::RenderProcess()
    {
    }

    RenderProcess::~RenderProcess()
    {
        auto &logicaldevice = Context::GetInstance().logicaldevice;
        logicaldevice.destroyRenderPass(renderPass);
        logicaldevice.destroyPipelineLayout(pipelineLayout);
        logicaldevice.destroyPipeline(pipline);
    }
}