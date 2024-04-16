#include "../toy2d/context.hpp"
#include "../toy2d/shader.hpp"
#include "../toy2d/renderprocess.hpp"

namespace toy2d
{

    void RenderProcess::InitRenderPass()
    {
        vk::RenderPassCreateInfo renderPassCreateInfo;

        // 1. color attachment description
        vk::AttachmentDescription colorAttachment;
        colorAttachment.setFormat(Context::GetInstance().swapchain->info.format.format)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setSamples(vk::SampleCountFlagBits::e1);
        renderPassCreateInfo.setPAttachments(&colorAttachment);

        // 2. color attachment reference
        vk::AttachmentReference colorAttachmentRef;
        colorAttachmentRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setAttachment(0);

        // 3. subpass
        vk::SubpassDescription subpass;
        subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setColorAttachments(colorAttachmentRef);
        renderPassCreateInfo.setSubpasses(subpass);

        // 4. subpass dependency
        vk::SubpassDependency dependency;
        dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
            .setDstSubpass(0)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
        renderPassCreateInfo.setDependencies(dependency);

        // 5. create renderpass
        renderPass = Context::GetInstance().logicaldevice.createRenderPass(renderPassCreateInfo);
    }

    void RenderProcess::InitRenderPassLayout()
    {
        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayout = Context::GetInstance().logicaldevice.createPipelineLayout(pipelineLayoutCreateInfo);
    }

    void RenderProcess::InitPipeline(int width, int height)
    {
        vk::GraphicsPipelineCreateInfo createinfo;

        // 1. vertex info
        vk::PipelineVertexInputStateCreateInfo inputstate;
        createinfo.setPVertexInputState(&inputstate);

        // 2. vertex assembly
        vk::PipelineInputAssemblyStateCreateInfo inputAsm;
        inputAsm.setPrimitiveRestartEnable(false)
            .setTopology(vk::PrimitiveTopology::eTriangleList);
        createinfo.setPInputAssemblyState(&inputAsm);

        // 3. shader
        auto stages = Shader::GetInstance().GetStage();
        createinfo.setStages(stages);

        // 4. viewport
        vk::PipelineViewportStateCreateInfo viewportState;
        vk::Viewport viewport(0, 0, width, height, 0, 1);
        viewportState.setViewports(viewport);
        vk::Rect2D rect({0, 0}, {static_cast<uint32_t>(width), static_cast<uint32_t>(height)});
        viewportState.setScissors(rect);
        createinfo.setPViewportState(&viewportState);

        // 4. Rasterization
        vk::PipelineRasterizationStateCreateInfo rastinfo;
        rastinfo.setRasterizerDiscardEnable(false)
            .setCullMode(vk::CullModeFlagBits::eBack)
            .setFrontFace(vk::FrontFace::eCounterClockwise)
            .setPolygonMode(vk::PolygonMode::eFill)
            .setLineWidth(1.0f);
        createinfo.setPRasterizationState(&rastinfo);

        // 5. multisample
        vk::PipelineMultisampleStateCreateInfo sample;
        sample.setSampleShadingEnable(false)
            .setRasterizationSamples(vk::SampleCountFlagBits::e1);
        createinfo.setPMultisampleState(&sample);

        // 6. test  pass

        // 7. color blend
        vk::PipelineColorBlendStateCreateInfo blend;
        vk::PipelineColorBlendAttachmentState blendAttachment;
        blendAttachment.setBlendEnable(false)
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
        blend.setLogicOpEnable(false)
            .setAttachments(blendAttachment);
        createinfo.setPColorBlendState(&blend);
        auto result = Context::GetInstance().logicaldevice.createGraphicsPipeline(nullptr, createinfo);
        if (result.result != vk::Result::eSuccess)
        {
            std::cout << "create pipeline failed" << std::endl;
            exit(2);
        }
        pipeline = result.value;
    }

    RenderProcess::RenderProcess()
    {
    }

    RenderProcess::~RenderProcess()
    {
        auto &logicaldevice = Context::GetInstance().logicaldevice;
        logicaldevice.destroyRenderPass(renderPass);
        logicaldevice.destroyPipelineLayout(pipelineLayout);
        logicaldevice.destroyPipeline(pipeline);
    }
}