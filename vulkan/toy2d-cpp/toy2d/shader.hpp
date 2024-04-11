#pragma once
#include <string>
#include <memory>
#include <vulkan/vulkan.hpp>
#include "context.hpp"

namespace toy2d
{
    class Shader final
    {
    public:
        static void Init(const std::string &vetexShaderSource,const std::string &fragmentShaderSource);
        static void Quit();
        static Shader &GetInstance();

        vk::ShaderModule vertexModule;
        vk::ShaderModule fragmentModule;
        ~Shader();

    private:
        Shader(const std::string &vertexShaderSource,const std::string &fragmentShaderSource);
        static std::unique_ptr<Shader> instance_;
    };
}