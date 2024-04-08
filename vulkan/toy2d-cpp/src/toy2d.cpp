#include "../toy2d/toy2d.hpp"
#include "../toy2d/context.hpp"

namespace toy2d
{
    void Init(std::vector<const char *> extensions, std::function<vk::SurfaceKHR(vk::Instance)> retsurface)
    {
        Context::Init(extensions, retsurface);
    }

    void Quit()
    {
        Context::Quit();
    }
}