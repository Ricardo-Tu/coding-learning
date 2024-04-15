#include "../toy2d/shader.hpp"
#include "../toy2d/context.hpp"
namespace toy2d
{
    std::unique_ptr<Shader> Shader::instance_ = nullptr;
    void Shader::Init(const std::string &vertexShaderSource, const std::string &fragmentShaderSource)
    {
        instance_.reset(new Shader(vertexShaderSource, fragmentShaderSource));
    }

    void Shader::Quit()
    {
        instance_.release();
    }

    Shader &Shader::GetInstance()
    {
        return *instance_;
    }

    void Shader::GetStage()
    {
        vk::PipelineShaderStageCreateInfo vertexStageInfo;
        vertexStageInfo.setFlags(vk::PipelineShaderStageCreateFlags())
            .setModule(vertexModule)
            .setPName("main")
            
    }

    Shader::~Shader()
    {
        auto &device = Context::GetInstance().logicaldevice;
        device.destroyShaderModule(vertexModule);
        device.destroyShaderModule(fragmentModule);
    }

    Shader::Shader(const std::string &vertexShaderSource, const std::string &fragmentShaderSource)
    {
        vk::ShaderModuleCreateInfo createInfo;
        createInfo.codeSize = vertexShaderSource.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(vertexShaderSource.data());
        vertexModule = Context::GetInstance().logicaldevice.createShaderModule(createInfo);

        createInfo.codeSize = fragmentShaderSource.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(fragmentShaderSource.data());
        fragmentModule = Context::GetInstance().logicaldevice.createShaderModule(createInfo);
    }
}