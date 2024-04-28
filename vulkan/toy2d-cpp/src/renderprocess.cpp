#include "../toy2d/context.hpp"
#include "../toy2d/shader.hpp"
#include "../toy2d/renderprocess.hpp"

namespace toy2d
{
    const std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}}};

    std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

    MVP mvp = {};

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
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setSrcAccessMask(vk::AccessFlagBits::eNone);

        renderPassCreateInfo.setSubpasses(subpass)
            .setSubpassCount(1)
            .setPAttachments(&colorAttachmentDescription)
            .setAttachmentCount(1)
            .setDependencies(dependency)
            .setDependencyCount(1);

        // 5. create renderpass
        assert(Context::GetInstance().logicaldevice != nullptr);
        renderPass = Context::GetInstance().logicaldevice.createRenderPass(renderPassCreateInfo);
    }

    void RenderProcess::InitDescriptorSet(uint32_t count)
    {
        vk::DescriptorPoolCreateInfo poolCreateInfo;
        vk::DescriptorPoolSize poolSize;
        vk::DescriptorSetLayoutBinding layoutBinding;
        poolSize.setType(vk::DescriptorType::eUniformBuffer)
            .setDescriptorCount(count);
        poolCreateInfo.setMaxSets(count)
            .setPoolSizeCount(1)
            .setPoolSizes(poolSize);
        descriptorPool = Context::GetInstance().logicaldevice.createDescriptorPool(poolCreateInfo);

        layoutBinding.setBinding(0)
            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setDescriptorCount(1)
            .setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
            .setPImmutableSamplers(nullptr);

        vk::DescriptorSetLayoutCreateInfo layoutCreateInfo;
        layoutCreateInfo.setBindings(layoutBinding)
            .setBindingCount(1);

        // descriptorSetLayouts.resize(count);
        // for (uint32_t i = 0; i < count; i++)
        // {
        // descriptorSetLayouts[i] = Context::GetInstance().logicaldevice.createDescriptorSetLayout(layoutCreateInfo);
        // }
        descriptorSetLayouts.resize(count, Context::GetInstance().logicaldevice.createDescriptorSetLayout(layoutCreateInfo));

        vk::DescriptorSetAllocateInfo allocateInfo;
        allocateInfo.setDescriptorPool(descriptorPool)
            .setDescriptorSetCount(count)
            .setSetLayouts(descriptorSetLayouts);
        descriptorSets.resize(count);
        descriptorSets = Context::GetInstance().logicaldevice.allocateDescriptorSets(allocateInfo);
    }

    void RenderProcess::CreateCommandDescriptorSets()
    {
        for (uint32_t i = 0; i < descriptorSets.size(); i++)
        {
            vk::DescriptorBufferInfo bufferInfo;
            bufferInfo.setBuffer(hostUniformBuffer[i])
                .setOffset(0)
                .setRange(sizeof(MVP));

            vk::WriteDescriptorSet descriptorWrite;
            descriptorWrite.setDstSet(descriptorSets[i])
                .setDstBinding(0)
                .setDstArrayElement(0)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setDescriptorCount(1)
                .setPBufferInfo(&bufferInfo);

            Context::GetInstance().logicaldevice.updateDescriptorSets({descriptorWrite}, {});
        }
    }

    void RenderProcess::InitRenderPassLayout()
    {
        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo.setSetLayoutCount(1)
            .setSetLayouts(descriptorSetLayouts);
        pipelineLayout = Context::GetInstance().logicaldevice.createPipelineLayout(pipelineLayoutCreateInfo);
        assert(pipelineLayout != nullptr);
    }

    void RenderProcess::InitPipeline()
    {

        vk::GraphicsPipelineCreateInfo pipelinecreateinfo;

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
        pipelinecreateinfo.setPVertexInputState(&vertexInput);

        // 2. vertex assembly
        vk::PipelineInputAssemblyStateCreateInfo inputAsm;
        inputAsm.setPrimitiveRestartEnable(false)
            .setTopology(vk::PrimitiveTopology::eTriangleList);
        pipelinecreateinfo.setPInputAssemblyState(&inputAsm);

        // 3. shader
        auto stages = Shader::GetInstance().GetStage();
        pipelinecreateinfo.setStages(stages);

        // 4. viewport
        vk::PipelineViewportStateCreateInfo viewportState;
        vk::Viewport viewport(0, 0, Context::GetInstance().swapchain_->swapchaininfo.extent.width, Context::GetInstance().swapchain_->swapchaininfo.extent.height, 0, 1);
        viewportState.setViewports(viewport);
        vk::Rect2D rect({0, 0}, {static_cast<uint32_t>(Context::GetInstance().swapchain_->swapchaininfo.extent.width), static_cast<uint32_t>(Context::GetInstance().swapchain_->swapchaininfo.extent.height)});
        viewportState.setScissors(rect);
        viewportState.setViewportCount(1)
            .setScissorCount(1);
        // .setScissors(rect);
        pipelinecreateinfo.setPViewportState(&viewportState);

        // 4. Rasterization
        vk::PipelineRasterizationStateCreateInfo rastinfo;
        rastinfo.setRasterizerDiscardEnable(false)
            .setDepthClampEnable(vk::False)
            .setCullMode(vk::CullModeFlagBits::eBack)
            .setFrontFace(vk::FrontFace::eClockwise)
            .setPolygonMode(vk::PolygonMode::eFill)
            .setLineWidth(1.0f)
            .setDepthBiasEnable(vk::False);
        pipelinecreateinfo.setPRasterizationState(&rastinfo);

        // 5. multisample
        vk::PipelineMultisampleStateCreateInfo sample;
        sample.setSampleShadingEnable(false)
            .setRasterizationSamples(vk::SampleCountFlagBits::e1);
        pipelinecreateinfo.setPMultisampleState(&sample);

        // 6. test  pass

        // 7. color blend
        vk::PipelineColorBlendStateCreateInfo blend;
        vk::PipelineColorBlendAttachmentState blendAttachment;
        blendAttachment.setBlendEnable(false)
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
        blend.setLogicOpEnable(false)
            // .setLogicOp(vk::LogicOp::eCopy)
            .setAttachments(blendAttachment);

        // 8. dynamic state
        std::vector<vk::DynamicState> dynamicStates = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor};
        vk::PipelineDynamicStateCreateInfo dynmaicCreateInfo;
        dynmaicCreateInfo.setDynamicStateCount(dynamicStates.size())
            .setDynamicStates(dynamicStates);

        pipelinecreateinfo.setPColorBlendState(&blend)
            .setRenderPass(renderPass)
            .setLayout(pipelineLayout)
            .setPDynamicState(&dynmaicCreateInfo)
            .setSubpass(0)
            .setBasePipelineHandle(nullptr);

        auto result = Context::GetInstance().logicaldevice.createGraphicsPipeline(nullptr, pipelinecreateinfo);
        if (result.result != vk::Result::eSuccess)
        {
            std::cout << "create pipeline failed" << std::endl;
            exit(2);
        }
        pipeline = result.value;
    }

    vk::DeviceMemory RenderProcess::CreateDeviceMemory(vk::Buffer buf, vk::MemoryPropertyFlags flags)
    {
        vk::Device logicaldevice = Context::GetInstance().logicaldevice;
        vk::MemoryRequirements memoryRequirements = logicaldevice.getBufferMemoryRequirements(buf);
        vk::MemoryAllocateInfo memoryAllocateInfo;
        vk::PhysicalDeviceMemoryProperties properties = Context::GetInstance().physicalDevice.getMemoryProperties();
        uint32_t memorytypeIndex = 0;

        for (uint32_t i = 0; i < properties.memoryTypeCount; i++)
        {
            if ((memoryRequirements.memoryTypeBits & (1 << i)) && (properties.memoryTypes[i].propertyFlags & flags))
            {
                memorytypeIndex = i;
                break;
            }
        }

        memoryAllocateInfo.setAllocationSize(memoryRequirements.size)
            .setMemoryTypeIndex(memorytypeIndex);
        vk::DeviceMemory retMemory = logicaldevice.allocateMemory(memoryAllocateInfo);
        assert(retMemory != nullptr);
        return retMemory;
    }

    vk::Buffer RenderProcess::CreateVkBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage)
    {
        vk::BufferCreateInfo bufferCreateInfo;
        bufferCreateInfo.setSize(size)
            .setUsage(usage)
            .setSharingMode(vk::SharingMode::eExclusive);
        vk::Buffer retBuf = Context::GetInstance().logicaldevice.createBuffer(bufferCreateInfo);
        assert(retBuf != nullptr);
        return retBuf;
    }

    void RenderProcess::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
    {
        vk::CommandBuffer cmdbuf = Context::GetInstance().render_->CreateCommandBuffer(1)[0];
        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        cmdbuf.begin(beginInfo);
        {
            vk::BufferCopy copy;
            copy.setSize(size)
                .setSrcOffset(0)
                .setDstOffset(0);
            cmdbuf.copyBuffer(srcBuffer, dstBuffer, copy);
        }
        cmdbuf.end();
        vk::SubmitInfo submit;
        submit.setCommandBuffers(cmdbuf);
        Context::GetInstance().graphicQueue.submit(submit, nullptr);
        Context::GetInstance().logicaldevice.waitIdle();
        Context::GetInstance().logicaldevice.freeCommandBuffers(Context::GetInstance().render_->cmdpool_, cmdbuf);
    }

    void RenderProcess::CreateVertexBuffer()
    {
        vk::Device logicaldevice = Context::GetInstance().logicaldevice;
        hostVertexBuffer = CreateVkBuffer(vertices.size() * sizeof(Vertex), vk::BufferUsageFlagBits::eTransferSrc);
        hostBufferMemory = CreateDeviceMemory(hostVertexBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        logicaldevice.bindBufferMemory(hostVertexBuffer, hostBufferMemory, 0);

        void *ptr = logicaldevice.mapMemory(hostBufferMemory, 0, vertices.size() * sizeof(Vertex)); // map memory
        memcpy(ptr, vertices.data(), vertices.size() * sizeof(Vertex));
        // logicaldevice.unmapMemory(hostBufferMemory); // unmap memory when you don't need it forever,otherwise don't do it.

        gpuVertexBuffer = CreateVkBuffer(vertices.size() * sizeof(Vertex), vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer);
        gpuBufferMemory = CreateDeviceMemory(gpuVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);
        logicaldevice.bindBufferMemory(gpuVertexBuffer, gpuBufferMemory, 0);

        CopyBuffer(hostVertexBuffer, gpuVertexBuffer, vertices.size() * sizeof(Vertex));
    }

    void RenderProcess::CreateIndexBuffer()
    {
        vk::Device logicaldevice = Context::GetInstance().logicaldevice;
        vk::Buffer stageBuffer;
        vk::DeviceMemory stageBufferMemory;
        stageBuffer = CreateVkBuffer(sizeof(indices[0]) * indices.size(), vk::BufferUsageFlagBits::eTransferSrc);
        stageBufferMemory = CreateDeviceMemory(stageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        logicaldevice.bindBufferMemory(stageBuffer, stageBufferMemory, 0);
        void *stageBufferPtr = logicaldevice.mapMemory(stageBufferMemory, 0, sizeof(indices[0]) * indices.size());
        memcpy(stageBufferPtr, indices.data(), sizeof(indices[0]) * indices.size());

        indexBuffer = CreateVkBuffer(sizeof(indices[0]) * indices.size(), vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer);
        indexBufferMemory = CreateDeviceMemory(indexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);
        logicaldevice.bindBufferMemory(indexBuffer, indexBufferMemory, 0);

        CopyBuffer(stageBuffer, indexBuffer, sizeof(indices[0]) * indices.size());
        logicaldevice.unmapMemory(stageBufferMemory);
        logicaldevice.destroyBuffer(stageBuffer);
        logicaldevice.freeMemory(stageBufferMemory);
    }

    void RenderProcess::CreateUniformBuffer(uint32_t count)
    {
        hostUniformBuffer.resize(count);
        hostUniformBufferMemory.resize(count);
        hostUniformBufferMemoryPtr.resize(count);
        vk::Device logicaldevice = Context::GetInstance().logicaldevice;
        for (uint32_t index = 0; index < count; index++)
        {
            hostUniformBuffer[index] = CreateVkBuffer(sizeof(MVP), vk::BufferUsageFlagBits::eUniformBuffer);
            hostUniformBufferMemory[index] = CreateDeviceMemory(hostUniformBuffer[index], vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
            logicaldevice.bindBufferMemory(hostUniformBuffer[index], hostUniformBufferMemory[index], 0);
            hostUniformBufferMemoryPtr[index] = logicaldevice.mapMemory(hostUniformBufferMemory[index], 0, sizeof(MVP));
        }
    }

    RenderProcess::RenderProcess()
    {
    }

    RenderProcess::~RenderProcess()
    {
        std::cout << "Entry and leave ~RenderProcess." << std::endl;
    }
}