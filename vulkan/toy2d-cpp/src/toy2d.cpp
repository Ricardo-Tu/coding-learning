#include "../toy2d/toy2d.hpp"
#include "../toy2d/context.hpp"
#include "../toy2d/shader.hpp"

namespace toy2d
{
    void Init(std::vector<const char *> extensions, std::function<vk::SurfaceKHR(vk::Instance)> retsurface, int width, int height)
    {
        Context::Init(extensions, retsurface);
        Context::GetInstance().InitSwapchain(width, height);
        Shader::Init(ReadWholeFile("vert.spv"), ReadWholeFile("frag.spv"));
        Context::GetInstance().renderprocess->InitRenderPass();
        Context::GetInstance().renderprocess->InitRenderPassLayout();
        Context::GetInstance().swapchain->createFramebuffers(width, height);
        Context::GetInstance().renderprocess->InitPipeline(width, height);
    }

    void Quit()
    {
        Shader::Quit();
        Context::Quit();
    }
}