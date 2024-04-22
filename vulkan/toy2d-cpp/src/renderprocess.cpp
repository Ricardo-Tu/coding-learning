#include "../toy2d/context.hpp"
#include "../toy2d/shader.hpp"
#include "../toy2d/renderprocess.hpp"

namespace toy2d
{
    const std::vector<Vertex> vertices = {
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.0f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}};

    void RenderProcess::InitRenderPass()
    {
        vk::RenderPassCreateInfo renderPassCreateInfo;

        // 1. color attachment description
        vk::AttachmentDescription colorAttachmentDescription;
        colorAttachmentDescription.setFormat(Context::GetInstance().swapchain_->swapchaininfo.format.format)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setSamples(vk::SampleCountFlagBits::e1);

        // 2. color attachment reference
        vk::AttachmentReference AttachmentRef;
        AttachmentRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setAttachment(0);

        // 3. subpass
        vk::SubpassDescription subpass;
        subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setPColorAttachments(&AttachmentRef)
            .setColorAttachmentCount(1);

        // 4. subpass dependency
        vk::SubpassDependency dependency;
        dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
            .setDstSubpass(0)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);

        renderPassCreateInfo.setSubpasses(subpass)
            .setPAttachments(&colorAttachmentDescription)
            .setDependencies(dependency)
            .setSubpassCount(1)
            .setAttachmentCount(1);

        // 5. create renderpass
        assert(Context::GetInstance().logicaldevice != nullptr);
        renderPass = Context::GetInstance().logicaldevice.createRenderPass(renderPassCreateInfo);
    }

    void RenderProcess::InitRenderPassLayout()
    {
        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo.setSetLayoutCount(0)
            .setPushConstantRangeCount(0);
        pipelineLayout = Context::GetInstance().logicaldevice.createPipelineLayout(pipelineLayoutCreateInfo);
        assert(pipelineLayout != nullptr);
    }

    void RenderProcess::InitPipeline(int width, int height)
    {

        vk::GraphicsPipelineCreateInfo createinfo;

        // 1. vertex info
        vk::PipelineVertexInputStateCreateInfo vertexInput;
        vk::VertexInputBindingDescription bindingdescrip;
        std::array<vk::VertexInputAttributeDescription, 2> attribute;

        attribute[0].setBinding(0).setFormat(vk::Format::eR32G32B32Sfloat).setLocation(0).setOffset(offsetof(Vertex, pos));
        attribute[1].setBinding(0).setFormat(vk::Format::eR32G32B32Sfloat).setLocation(1).setOffset(offsetof(Vertex, color));
        bindingdescrip.setBinding(0)
            .setStride(sizeof(Vertex))
            .setInputRate(vk::VertexInputRate::eVertex);
        vertexInput.setVertexBindingDescriptionCount(1)
            .setVertexBindingDescriptions(bindingdescrip)
            .setVertexAttributeDescriptionCount(attribute.size())
            .setVertexAttributeDescriptions(attribute);
        createinfo.setPVertexInputState(&vertexInput);

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
            .setFrontFace(vk::FrontFace::eClockwise)
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
        createinfo.setPColorBlendState(&blend)
            .setRenderPass(renderPass)
            .setLayout(pipelineLayout);
        auto result = Context::GetInstance().logicaldevice.createGraphicsPipeline(nullptr, createinfo);
        if (result.result != vk::Result::eSuccess)
        {
            std::cout << "create pipeline failed" << std::endl;
            exit(2);
        }
        pipeline = result.value;
    }

    void RenderProcess::CreateVertexBuffer()
    {
        vk::BufferCreateInfo bufferCreateInfo;
        vk::Device logicaldevice = Context::GetInstance().logicaldevice;
        bufferCreateInfo.setSize(vertices.size() * sizeof(Vertex))
            .setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
            .setSharingMode(vk::SharingMode::eExclusive);
        vertexBuffer = logicaldevice.createBuffer(bufferCreateInfo);
        assert(vertexBuffer != nullptr);

        vk::MemoryRequirements memoryRequirements = logicaldevice.getBufferMemoryRequirements(vertexBuffer);
        vk::MemoryAllocateInfo memoryAllocateInfo;
        vk::PhysicalDeviceMemoryProperties properties = Context::GetInstance().physicalDevice.getMemoryProperties();
        uint32_t memorytypeIndex = 0; // find memory type index
        for (uint32_t i = 0; i < properties.memoryTypeCount; i++)
        {
            if ((memoryRequirements.memoryTypeBits & (1 << i)) && (properties.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible))
            {
                memorytypeIndex = i;
                break;
            }
        }
        memoryAllocateInfo.setAllocationSize(memoryRequirements.size)
            .setMemoryTypeIndex(memorytypeIndex);
        vertexBufferMemory = logicaldevice.allocateMemory(memoryAllocateInfo);
        assert(vertexBufferMemory != nullptr);

        logicaldevice.bindBufferMemory(vertexBuffer, vertexBufferMemory, 0);

        void *data = logicaldevice.mapMemory(vertexBufferMemory, 0, sizeof(vertices)); // map memory
        memcpy(data, vertices.data(), vertices.size() * sizeof(Vertex));
        logicaldevice.unmapMemory(vertexBufferMemory);
    }

    RenderProcess::RenderProcess()
    {
    }

    RenderProcess::~RenderProcess()
    {
        std::cout << "Entry and leave ~RenderProcess." << std::endl;
    }
}