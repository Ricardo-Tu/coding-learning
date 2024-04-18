#include "../toy2d/context.hpp"
#include "../toy2d/render.hpp"

namespace toy2d
{
    void render::InitCmdPool()
    {
        vk::CommandPoolCreateInfo cmdcreateInfo;
        cmdcreateInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

        cmdpool_ = Context::GetInstance().logicaldevice.createCommandPool(cmdcreateInfo);
    }

    void render::InitCmdBuffer()
    {
        vk::CommandBufferAllocateInfo allocInfo;
        allocInfo.setCommandPool(cmdpool_)
            .setCommandBufferCount(1)
            .setLevel(vk::CommandBufferLevel::ePrimary);

        cmdbuffer_ = Context::GetInstance().logicaldevice.allocateCommandBuffers(allocInfo)[0];
    }

    void render::Render()
    {
        auto &logicaldevice = Context::GetInstance().logicaldevice;
        auto result = logicaldevice.acquireNextImageKHR(Context::GetInstance().swapchain->swapchain, UINT64_MAX, imageAvaliable_);
        if (result.result != vk::Result::eSuccess)
            std::throw_with_nested(std::runtime_error("failed to acquire next image"));

        auto imageIndex = result.value;

        cmdbuffer_.reset();

        vk::CommandBufferBeginInfo begin;
        begin.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        cmdbuffer_.begin(begin);
        {
            cmdbuffer_.bindPipeline(vk::PipelineBindPoint::eGraphics, Context::GetInstance().renderprocess->pipeline);
            vk::RenderPassBeginInfo beginInfo;
            vk::Rect2D renderArea;
            vk::ClearValue clearValue;
            clearValue.color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});
            renderArea.setOffset({0, 0})
                .setExtent(Context::GetInstance().swapchain->info.extent);
            beginInfo.setRenderPass(Context::GetInstance().renderprocess->renderPass)
                .setFramebuffer(Context::GetInstance().swapchain->framebuffers[imageIndex])
                .setRenderArea(renderArea)
                .setClearValueCount(1)
                .setClearValues(clearValue);

            cmdbuffer_.beginRenderPass(beginInfo, vk::SubpassContents::eInline);
            {
                cmdbuffer_.draw(3, 1, 0, 0);
            }
            cmdbuffer_.endRenderPass();
        }
        cmdbuffer_.end();

        vk::SubmitInfo submit;
        std::array<vk::PipelineStageFlags, 1> flag = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        submit.setCommandBuffers(cmdbuffer_)
            .setWaitSemaphores(imageAvaliable_)
            .setWaitDstStageMask(flag)
            .setSignalSemaphores(imageDrawFinsh_)
            .setWaitSemaphoreCount(1)
            .setSignalSemaphoreCount(1);

        Context::GetInstance().graphicQueue.submit(submit, fence_);

        vk::PresentInfoKHR present;
        present.setImageIndices(imageIndex)
            .setSwapchains(Context::GetInstance().swapchain->swapchain)
            .setWaitSemaphores(imageDrawFinsh_);

        auto res = Context::GetInstance().presentQueue.presentKHR(present);
        if (res != vk::Result::eSuccess)
            std::throw_with_nested(std::runtime_error("failed to present"));

        res = Context::GetInstance().logicaldevice.waitForFences(fence_, vk::True, UINT64_MAX);
        if (res != vk::Result::eSuccess)
            std::throw_with_nested(std::runtime_error("failed to wait for fence"));

        logicaldevice.resetFences(fence_);
    }

    render::render()
    {
        vk::SemaphoreCreateInfo semaphoreInfo;
        vk::FenceCreateInfo fenceInfo;
        vk::Device logicaldevice = Context::GetInstance().logicaldevice;
        imageAvaliable_ = logicaldevice.createSemaphore(semaphoreInfo);
        imageDrawFinsh_ = logicaldevice.createSemaphore(semaphoreInfo);
        fence_ = logicaldevice.createFence(fenceInfo);
        InitCmdPool();
        InitCmdBuffer();
    }

    render::~render()
    {
        vk::Device logicaldevice = Context::GetInstance().logicaldevice;
        // logicaldevice.destroyFence(fence_);
        // logicaldevice.destroyCommandPool(cmdpool_);
    }
}