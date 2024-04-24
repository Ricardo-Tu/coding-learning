#include "../toy2d/context.hpp"
#include "../toy2d/shader.hpp"
#include "../toy2d/renderprocess.hpp"

namespace toy2d
{
    const std::vector<Vertex> vertices = {
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.0f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}};

    MVP mvp = {
        .model = glm::mat4(1.0f),
        .view = glm::lookAt(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)),
        .project = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 10.0f)};

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
            .setStageFlags(vk::ShaderStageFlagBits::eVertex)
            .setPImmutableSamplers(nullptr);

        vk::DescriptorSetLayoutCreateInfo layoutCreateInfo;
        layoutCreateInfo.setBindings(layoutBinding)
            .setBindingCount(1);

        descriptorSetLayouts.resize(count, Context::GetInstance().logicaldevice.createDescriptorSetLayout(layoutCreateInfo));

        vk::DescriptorSetAllocateInfo allocateInfo;
        allocateInfo.setDescriptorPool(descriptorPool)
            .setDescriptorSetCount(count)
            .setSetLayouts(descriptorSetLayouts);
        descriptorSets = Context::GetInstance().logicaldevice.allocateDescriptorSets(allocateInfo);
    }

    void RenderProcess::InitRenderPassLayout()
    {
        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo.setSetLayoutCount(1)
            .setSetLayouts(descriptorSetLayouts);
        pipelineLayout = Context::GetInstance().logicaldevice.createPipelineLayout(pipelineLayoutCreateInfo);
        assert(pipelineLayout != nullptr);
    }

    void RenderProcess::InitPipeline(int width, int height)
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
        vk::Viewport viewport(0, 0, width, height, 0, 1);
        viewportState.setViewports(viewport);
        vk::Rect2D rect({0, 0}, {static_cast<uint32_t>(width), static_cast<uint32_t>(height)});
        viewportState.setScissors(rect);
        viewportState.setViewportCount(1)
            .setScissorCount(1)
            .setScissors(rect);
        pipelinecreateinfo.setPViewportState(&viewportState);

        // 4. Rasterization
        vk::PipelineRasterizationStateCreateInfo rastinfo;
        rastinfo.setRasterizerDiscardEnable(false)
            .setCullMode(vk::CullModeFlagBits::eBack)
            .setFrontFace(vk::FrontFace::eClockwise)
            .setPolygonMode(vk::PolygonMode::eFill)
            .setLineWidth(1.0f);
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
            .setPDynamicState(&dynmaicCreateInfo);

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

    void RenderProcess::CreateCommandDescriptorSets()
    {
        for (uint32_t i = 0; i < descriptorSets.size(); i++)
        {
            auto &set = descriptorSets[i];
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

    void RenderProcess::CreateVertexBuffer()
    {
        vk::Device logicaldevice = Context::GetInstance().logicaldevice;
        hostVertexBuffer = CreateVkBuffer(vertices.size() * sizeof(Vertex), vk::BufferUsageFlagBits::eTransferSrc);
        hostBufferMemory = CreateDeviceMemory(hostVertexBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        logicaldevice.bindBufferMemory(hostVertexBuffer, hostBufferMemory, 0);

        void *ptr = logicaldevice.mapMemory(hostBufferMemory, 0, sizeof(vertices)); // map memory
        memcpy(ptr, vertices.data(), vertices.size() * sizeof(Vertex));
        // logicaldevice.unmapMemory(hostBufferMemory); // unmap memory when you don't need it forever,otherwise don't do it.

        gpuVertexBuffer = CreateVkBuffer(vertices.size() * sizeof(Vertex), vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer);
        gpuBufferMemory = CreateDeviceMemory(gpuVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);
        logicaldevice.bindBufferMemory(gpuVertexBuffer, gpuBufferMemory, 0);

        vk::CommandBuffer cmdbuf = Context::GetInstance().render_->CreateCommandBuffer(1)[0];
        vk::CommandBufferBeginInfo begin;
        begin.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        cmdbuf.begin(begin);
        {
            vk::BufferCopy copy;
            copy.setSize(vertices.size() * sizeof(Vertex))
                .setSrcOffset(0)
                .setDstOffset(0);
            cmdbuf.copyBuffer(hostVertexBuffer, gpuVertexBuffer, copy);
        }
        cmdbuf.end();
        vk::SubmitInfo submit;
        submit.setCommandBuffers(cmdbuf);
        Context::GetInstance().graphicQueue.submit(submit, nullptr);
        Context::GetInstance().logicaldevice.waitIdle();
        Context::GetInstance().logicaldevice.freeCommandBuffers(Context::GetInstance().render_->cmdpool_, cmdbuf);
    }

    void RenderProcess::CreateUniformBuffer(uint32_t count)
    {
        hostUniformBuffer.resize(count);
        hostUniformBufferMemory.resize(count);
        hostUniformBufferMemoryPtr.resize(count);
        for (uint32_t index = 0; index < count; index++)
        {
            vk::Device logicaldevice = Context::GetInstance().logicaldevice;
            hostUniformBuffer[index] = CreateVkBuffer(sizeof(glm::mat4) * 3, vk::BufferUsageFlagBits::eUniformBuffer);
            hostUniformBufferMemory[index] = CreateDeviceMemory(hostUniformBuffer[index], vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
            logicaldevice.bindBufferMemory(hostUniformBuffer[index], hostUniformBufferMemory[index], 0);
            hostUniformBufferMemoryPtr[index] = logicaldevice.mapMemory(hostUniformBufferMemory[index], 0, sizeof(glm::mat4));
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