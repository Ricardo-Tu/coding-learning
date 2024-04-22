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
            .setCommandBufferCount(maxFlightCount_)
            .setLevel(vk::CommandBufferLevel::ePrimary);

        cmdbuffer_ = Context::GetInstance().logicaldevice.allocateCommandBuffers(allocInfo);
    }

    void render::DrawColorTriangle()
    {
        auto &logicaldevice = Context::GetInstance().logicaldevice;
        auto res = Context::GetInstance().logicaldevice.waitForFences(fence_[currentFrame_], vk::True, UINT64_MAX);
        if (res != vk::Result::eSuccess)
            throw std::runtime_error("failed to wait for fence");

        logicaldevice.resetFences(fence_[currentFrame_]);
        auto result = logicaldevice.acquireNextImageKHR(Context::GetInstance().swapchain_->swapChain, UINT64_MAX, imageAvaliable_[currentFrame_]);
        if (result.result != vk::Result::eSuccess)
            throw std::runtime_error("failed to acquire next image");

        auto imageIndex = result.value;

        cmdbuffer_[currentFrame_].reset();

        vk::CommandBufferBeginInfo begin;
        begin.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        cmdbuffer_[currentFrame_].begin(begin);
        {
            cmdbuffer_[currentFrame_].bindPipeline(vk::PipelineBindPoint::eGraphics, Context::GetInstance().renderprocess_->pipeline);
            vk::RenderPassBeginInfo beginInfo;
            vk::Rect2D renderArea;
            vk::ClearValue clearValue;
            clearValue.color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});
            renderArea.setOffset({0, 0})
                .setExtent(Context::GetInstance().swapchain_->swapchaininfo.extent);
            beginInfo.setRenderPass(Context::GetInstance().renderprocess_->renderPass)
                .setFramebuffer(Context::GetInstance().swapchain_->framebuffers[imageIndex])
                .setRenderArea(renderArea)
                .setClearValueCount(1)
                .setClearValues(clearValue);

            cmdbuffer_[currentFrame_].bindVertexBuffers(0, {Context::GetInstance().renderprocess_->vertexBuffer}, {0});

            cmdbuffer_[currentFrame_].beginRenderPass(beginInfo, vk::SubpassContents::eInline);
            {
                cmdbuffer_[currentFrame_].draw(3, 1, 0, 0);
            }
            cmdbuffer_[currentFrame_].endRenderPass();
        }
        cmdbuffer_[currentFrame_].end();

        vk::SubmitInfo submit;
        std::array<vk::PipelineStageFlags, 1> flag = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        submit.setCommandBuffers(cmdbuffer_[currentFrame_])
            .setWaitSemaphores(imageAvaliable_[currentFrame_])
            .setWaitDstStageMask(flag)
            .setSignalSemaphores(imageDrawFinsh_[currentFrame_])
            .setWaitSemaphoreCount(1)
            .setSignalSemaphoreCount(1);

        Context::GetInstance().graphicQueue.submit(submit, fence_[currentFrame_]);

        vk::PresentInfoKHR present;
        present.setImageIndices(imageIndex)
            .setSwapchains(Context::GetInstance().swapchain_->swapChain)
            .setWaitSemaphores(imageDrawFinsh_[currentFrame_]);

        res = Context::GetInstance().presentQueue.presentKHR(present);
        if (res != vk::Result::eSuccess)
            throw std::runtime_error("failed to present");

        currentFrame_ = (currentFrame_ + 1) % maxFlightCount_;
    }

    render::render(uint32_t maxFlightCount) : maxFlightCount_(maxFlightCount), currentFrame_(0)
    {
        vk::Device logicaldevice = Context::GetInstance().logicaldevice;
        vk::FenceCreateInfo fenceInfo;
        vk::SemaphoreCreateInfo semaphoreInfo;
        fence_.resize(maxFlightCount_);
        fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
        imageAvaliable_.resize(maxFlightCount_);
        imageDrawFinsh_.resize(maxFlightCount_);
        for (uint32_t i = 0; i < maxFlightCount_; i++)
        {
            fence_[i] = logicaldevice.createFence(fenceInfo);
            imageAvaliable_[i] = logicaldevice.createSemaphore(semaphoreInfo);
            imageDrawFinsh_[i] = logicaldevice.createSemaphore(semaphoreInfo);
        }
        InitCmdPool();
        InitCmdBuffer();
    }

    render::~render()
    {
        vk::Device logicaldevice = Context::GetInstance().logicaldevice;
        for (auto &semaphore : imageAvaliable_)
            logicaldevice.destroySemaphore(semaphore);
        for (auto &semaphore : imageDrawFinsh_)
            logicaldevice.destroySemaphore(semaphore);
        for (auto &fence : fence_)
            logicaldevice.destroyFence(fence);
        logicaldevice.freeCommandBuffers(cmdpool_, cmdbuffer_);
        logicaldevice.destroyCommandPool(cmdpool_);
    }
}