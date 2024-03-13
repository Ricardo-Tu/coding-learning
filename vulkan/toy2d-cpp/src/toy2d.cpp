#include "../toy2d/toy2d.hpp"

namespace toy2d{
    void Context::Init(){
        instance_.reset(new Context);
    }

    void Context::Quit(){
        instance_.reset();
    }

    Context& Context::GetInstance(){
        return *instance_;
    }

    Context::Context(){
        vk::InstanceCreateInfo createInfo;
        instance = vk::createInstance(createInfo);
    }

    Context::~Context(){
        instance.destroy();
    }
}