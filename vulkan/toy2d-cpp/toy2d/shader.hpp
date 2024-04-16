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
        static void Init(const std::string &vetexShaderSource, const std::string &fragmentShaderSource);
        static void Quit();
        static Shader &GetInstance();
        std::vector<vk::PipelineShaderStageCreateInfo> GetStage();
        void initStage();
        vk::ShaderModule vertexModule;
        vk::ShaderModule fragmentModule;
        ~Shader();

    private:
        static std::unique_ptr<Shader> instance_;
        std::vector<vk::PipelineShaderStageCreateInfo> stage_;
        Shader(const std::string &vertexShaderSource, const std::string &fragmentShaderSource);
    };
}