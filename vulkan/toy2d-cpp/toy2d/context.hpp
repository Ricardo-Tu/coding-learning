#pragma once
#include <memory>
#include <iostream>
#include <optional>
#include <functional>
#include <vector>
#include <array>
#include <vulkan/vulkan.hpp>

namespace toy2d{
    class Context final
    {
    public:
        static void Init();
        static void Quit();
        static Context &GetInstance();
        ~Context();

        vk::Instance instance;

    private:
        Context();
        static std::unique_ptr<Context> instance_;
    };


}