#include "../toy2d/context.hpp"

namespace toy2d
{
    std::unique_ptr<Context> Context::instance_ = nullptr;
    void Context::Init()
    {
        Context::instance_.reset(new Context);
    }

    void Context::Quit()
    {
        instance_.reset();
    }

    Context &Context::GetInstance()
    {
        return *instance_;
    }

    Context::Context()
    {
        vk::InstanceCreateInfo info;
        vk::ApplicationInfo appInfo;
        std::vector<const char *> layers = {"VK_LAYER_KHRONOS_validation"};
        appInfo.setApiVersion(VK_API_VERSION_1_3);
        info.setPApplicationInfo(&appInfo)
            .setPEnabledLayerNames(layers);
        instance = vk::createInstance(info);
    }
    Context::~Context()
    {
        instance.destroy();
    }
}