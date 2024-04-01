#pragma once
#include "vulkan/vulkan.hpp"
#include <memory>

namespace toy2d
{
    class Context final
    {
    public:
        vk::Instance instance;

        static void Init();
        static void Quit();
        static Context &GetInstance();
        Context();
        ~Context();

    private:
        static std::unique_ptr<Context> instance_;
    };

}