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

    std::vector<vk::CommandBuffer> render::CreateCommandBuffer(uint32_t CommandBufferCount)
    {
        vk::CommandBufferAllocateInfo allocInfo;
        allocInfo.setCommandPool(cmdpool_)
            .setCommandBufferCount(CommandBufferCount)
            .setLevel(vk::CommandBufferLevel::ePrimary);

        return Context::GetInstance().logicaldevice.allocateCommandBuffers(allocInfo);
    }

    void render::InitCmdBuffer()
    {
        cmdbuffer_ = CreateCommandBuffer(maxFramesCount_);
    }

    void render::UpdateUniformBuffer(uint32_t CurrentUniformBufIndex)
    {
        MVP tmvp;
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        tmvp.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        tmvp.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        tmvp.project = glm::perspective(glm::radians(45.0f), Context::GetInstance().swapchain_->swapchaininfo.extent.width / (float)Context::GetInstance().swapchain_->swapchaininfo.extent.height, 0.1f, 10.0f);
        tmvp.project[1][1] *= -1;

        memcpy(Context::GetInstance().renderprocess_->hostUniformBufferMemoryPtr[CurrentUniformBufIndex], &tmvp, sizeof(mvp));
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

        UpdateUniformBuffer(currentFrame_);

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

            cmdbuffer_[currentFrame_].beginRenderPass(beginInfo, vk::SubpassContents::eInline);
            {
                // view port and scissor
                vk::Viewport viewport;
                vk::Rect2D scissor;
                viewport.setX(0.0f)
                    .setY(0.0f)
                    .setWidth(Context::GetInstance().swapchain_->swapchaininfo.extent.width)
                    .setHeight(Context::GetInstance().swapchain_->swapchaininfo.extent.height)
                    .setMinDepth(0.0f)
                    .setMaxDepth(1.0f);
                cmdbuffer_[currentFrame_].setViewport(0, viewport);

                vk::Rect2D scissorRect;
                scissor.setExtent(Context::GetInstance().swapchain_->swapchaininfo.extent)
                    .setOffset({0, 0});
                cmdbuffer_[currentFrame_].setScissor(0, scissor);
                cmdbuffer_[currentFrame_].bindVertexBuffers(0, {Context::GetInstance().renderprocess_->gpuVertexBuffer}, {0});
                cmdbuffer_[currentFrame_].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, Context::GetInstance().renderprocess_->pipelineLayout, 0, Context::GetInstance().renderprocess_->descriptorSets[currentFrame_], {});
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

        currentFrame_ = (currentFrame_ + 1) % maxFramesCount_;
    }

    render::render(uint32_t maxFramesCount_) : maxFramesCount_(maxFramesCount_), currentFrame_(0)
    {
        vk::Device logicaldevice = Context::GetInstance().logicaldevice;
        vk::FenceCreateInfo fenceInfo;
        vk::SemaphoreCreateInfo semaphoreInfo;
        fence_.resize(maxFramesCount_);
        fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
        imageAvaliable_.resize(maxFramesCount_);
        imageDrawFinsh_.resize(maxFramesCount_);
        for (uint32_t i = 0; i < maxFramesCount_; i++)
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