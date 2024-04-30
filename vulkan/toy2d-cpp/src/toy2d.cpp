#include "../toy2d/toy2d.hpp"
#include "../toy2d/render.hpp"
#include "../toy2d/context.hpp"
#include "../toy2d/shader.hpp"
#include "../toy2d/texture.hpp"

namespace toy2d
{
    SDL_Window *window = nullptr;
    bool framebufferResizeFlag = false;
    uint32_t WindowWidth = 1024;
    uint32_t WindowHeight = 720;

    void Init(std::vector<const char *> extensions, std::function<vk::SurfaceKHR(vk::Instance)> retsurface, int width, int height)
    {
        // Context::Init(extensions, retsurface);
        Context::GetInstance().instance_.reset(new Context(extensions, retsurface));
        Context::GetInstance().swapchain_.reset(new Swapchain(width, height));
        Context::GetInstance().renderprocess_.reset(new RenderProcess());
        Shader::Init(ReadWholeFile("vert.spv"), ReadWholeFile("frag.spv"));
        Context::GetInstance().renderprocess_->InitRenderPass();
        Context::GetInstance().renderprocess_->CreateUniformBuffer(Context::GetInstance().renderprocess_->maxFramesCount_);
        Context::GetInstance().renderprocess_->InitLayoutDescriptorSet(Context::GetInstance().renderprocess_->maxFramesCount_);
        // Context::GetInstance().renderprocess_->CreateCommandDescriptorSets();
        Context::GetInstance().renderprocess_->InitRenderPassLayout();
        Context::GetInstance().swapchain_->createFramebuffers();
        Context::GetInstance().renderprocess_->InitPipeline();
        Context::GetInstance().render_.reset(new render());
        Context::GetInstance().renderprocess_->CreateVertexBuffer();
        Context::GetInstance().renderprocess_->CreateIndexBuffer();
        Context::GetInstance().texture_.reset(new texture("./texture.jpg", Context::GetInstance().renderprocess_->maxFramesCount_));
        Context::GetInstance().texture_->createDescriptorPool();
        Context::GetInstance().texture_->createDescriptorSets();
    }

    void Quit()
    {
        Context::GetInstance().logicaldevice.waitIdle();
        Shader::Quit();
        Context::Quit();
    }

    void reCreateSwapChain()
    {
        Context::GetInstance().logicaldevice.waitIdle();
        Context::GetInstance().swapchain_.reset(new Swapchain(toy2d::WindowWidth, toy2d::WindowHeight));
        Context::GetInstance().swapchain_->createFramebuffers();
        toy2d::framebufferResizeFlag = false;
    }

    render &GetRenderInstance()
    {
        return *Context::GetInstance().render_;
    }
}