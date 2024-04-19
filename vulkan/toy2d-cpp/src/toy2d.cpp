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
        // Context::GetInstance().swapchain_.reset(new Swapchain(width, height));
        Context::GetInstance().swapchain_ = std::make_unique<Swapchain>(width, height);
        // Context::GetInstance().renderprocess_.reset(new RenderProcess());
        // Shader::Init(ReadWholeFile("vert.spv"), ReadWholeFile("frag.spv"));
        // Context::GetInstance().renderprocess_->InitRenderPass();
        // Context::GetInstance().renderprocess_->InitRenderPassLayout();
        // Context::GetInstance().swapchain->createFramebuffers(width, height);
        // Context::GetInstance().renderprocess_->InitPipeline(width, height);
        // Context::GetInstance().render_.reset(new render());
    }

    void Quit()
    {
        std::cout << "quit" << std::endl;
        // Context::GetInstance().logicaldevice.waitIdle();
        std::cout << "wait idle" << std::endl;
        std::cout << "shader quit" << std::endl;
        // Shader::Quit();
        Context::Quit();
    }

    render& GetRenderInstance()
    {
        return *Context::GetInstance().render_;
    }   
}