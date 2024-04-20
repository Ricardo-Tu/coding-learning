#include "../toy2d/shader.hpp"
#include "../toy2d/context.hpp"
namespace toy2d
{
    std::unique_ptr<Shader> Shader::instance_ = nullptr;
    void Shader::Init(const std::string &vertexShaderSource, const std::string &fragmentShaderSource)
    {
        instance_.reset(new Shader(vertexShaderSource, fragmentShaderSource));
        instance_->initStage();
    }

    void Shader::Quit()
    {
        instance_.reset();
    }

    Shader &Shader::GetInstance()
    {
        assert(instance_ != nullptr);
        return *instance_;
    }

    void Shader::initStage()
    {
        stage_.resize(2);
        stage_[0].setStage(vk::ShaderStageFlagBits::eVertex).setModule(vertexModule).setPName("main");
        stage_[1].setStage(vk::ShaderStageFlagBits::eFragment).setModule(fragmentModule).setPName("main");
    }

    std::vector<vk::PipelineShaderStageCreateInfo> Shader::GetStage()
    {
        return stage_;
    }

    Shader::Shader(const std::string &vertexShaderSource, const std::string &fragmentShaderSource)
    {
        vk::ShaderModuleCreateInfo createInfo;
        createInfo.codeSize = vertexShaderSource.size();
        createInfo.pCode = (uint32_t *)vertexShaderSource.data();
        vertexModule = Context::GetInstance().logicaldevice.createShaderModule(createInfo);

        createInfo.codeSize = fragmentShaderSource.size();
        createInfo.pCode = (uint32_t *)fragmentShaderSource.data();
        fragmentModule = Context::GetInstance().logicaldevice.createShaderModule(createInfo);
    }

    Shader::~Shader()
    {
        auto &device = Context::GetInstance().logicaldevice;
        device.destroyShaderModule(vertexModule);
        device.destroyShaderModule(fragmentModule);
    }
}