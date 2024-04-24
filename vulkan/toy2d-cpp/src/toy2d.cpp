#include "../toy2d/toy2d.hpp"
#include "../toy2d/render.hpp"
#include "../toy2d/context.hpp"
#include "../toy2d/shader.hpp"

namespace toy2d
{
    void Init(std::vector<const char *> extensions, std::function<vk::SurfaceKHR(vk::Instance)> retsurface, int width, int height)
    {
        // Context::Init(extensions, retsurface);
        Context::GetInstance().instance_.reset(new Context(extensions, retsurface));
        Context::GetInstance().swapchain_.reset(new Swapchain(width, height));
        Context::GetInstance().renderprocess_.reset(new RenderProcess());
        Shader::Init(ReadWholeFile("vert.spv"), ReadWholeFile("frag.spv"));
        Context::GetInstance().renderprocess_->InitRenderPass();
        Context::GetInstance().renderprocess_->InitDescriptorSet(Context::GetInstance().renderprocess_->maxFramesCount_);
        Context::GetInstance().renderprocess_->CreateUniformBuffer(Context::GetInstance().renderprocess_->maxFramesCount_);
        Context::GetInstance().renderprocess_->CreateCommandDescriptorSets();
        Context::GetInstance().renderprocess_->InitRenderPassLayout();
        Context::GetInstance().swapchain_->createFramebuffers(width, height);
        Context::GetInstance().renderprocess_->InitPipeline(width, height);
        Context::GetInstance().render_.reset(new render());
        Context::GetInstance().renderprocess_->CreateVertexBuffer();
    }

    void Quit()
    {
        Context::GetInstance().logicaldevice.waitIdle();
        Shader::Quit();
        Context::Quit();
    }

    render &GetRenderInstance()
    {
        return *Context::GetInstance().render_;
    }
}