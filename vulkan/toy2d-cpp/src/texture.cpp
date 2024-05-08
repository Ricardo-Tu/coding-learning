#include "../toy2d/context.hpp"
#include "../toy2d/texture.hpp"
#include "../toy2d/buffer.hpp"
#include "../toy2d/renderprocess.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "../toy2d/stb_image.hpp"

namespace toy2d
{
    void texture::createTextureImage(uint32_t width, uint32_t height)
    {
        vk::ImageCreateInfo imageCreateInfo;
        imageCreateInfo.setImageType(vk::ImageType::e2D)
            .setArrayLayers(1)
            .setMipLevels(1)
            .setExtent(vk::Extent3D(width, height, 1))
            .setFormat(vk::Format::eR8G8B8A8Srgb)
            .setTiling(vk::ImageTiling::eOptimal)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
            .setSamples(vk::SampleCountFlagBits::e1);

        image = Context::GetInstance().logicaldevice.createImage(imageCreateInfo);
    }

    void texture::allocateMemory()
    {
        vk::Device logicaldevice = Context::GetInstance().logicaldevice;
        vk::MemoryAllocateInfo allocInfo;

        vk::MemoryRequirements memRequirements = logicaldevice.getImageMemoryRequirements(image);
        allocInfo.setAllocationSize(memRequirements.size);

        uint32_t index = queryImageMemoryIndex(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
        allocInfo.setMemoryTypeIndex(index);

        imageMemory = logicaldevice.allocateMemory(allocInfo, nullptr);
        assert(imageMemory != nullptr);

        logicaldevice.bindImageMemory(image, imageMemory, 0);
    }

    uint32_t texture::queryImageMemoryIndex(uint32_t flag, vk::MemoryPropertyFlagBits property)
    {
        vk::PhysicalDeviceMemoryProperties memProperties = Context::GetInstance().physicalDevice.getMemoryProperties();
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((flag & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & property) == property)
            {
                return i;
            }
        }
        return 0;
    }

    void texture::transitionImageLayoutFromUndefine2Dst()
    {
        vk::CommandBufferBeginInfo beginInfo;
        vk::CommandBuffer cmdbuf = Context::GetInstance().render_->CreateCommandBuffer(1)[0];
        beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        cmdbuf.begin(beginInfo);
        {
            vk::ImageSubresourceRange range;
            vk::ImageMemoryBarrier barrier;
            range.setLayerCount(1)
                .setBaseArrayLayer(0)
                .setLevelCount(1)
                .setBaseMipLevel(0)
                .setAspectMask(vk::ImageAspectFlagBits::eColor);

            barrier.setImage(image)
                .setSrcAccessMask(vk::AccessFlagBits::eNone)
                .setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
                .setOldLayout(vk::ImageLayout::eUndefined)
                .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setSubresourceRange(range);
            cmdbuf.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, {}, nullptr, barrier);
        }
        cmdbuf.end();
        vk::SubmitInfo submitInfo;
        submitInfo.setCommandBuffers(cmdbuf);
        Context::GetInstance().graphicQueue.submit(submitInfo, nullptr);
        Context::GetInstance().graphicQueue.waitIdle();
        Context::GetInstance().logicaldevice.freeCommandBuffers(Context::GetInstance().render_->cmdpool_, cmdbuf);
    }

    void texture::transformBufferdata2Image(vk::Buffer &buf, uint32_t width, uint32_t height)
    {
        vk::CommandBufferBeginInfo beginInfo;
        vk::CommandBuffer cmdbuf = Context::GetInstance().render_->CreateCommandBuffer(1)[0];
        beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        cmdbuf.begin(beginInfo);
        {
            vk::BufferImageCopy region;
            vk::ImageSubresourceLayers subsource;
            subsource.setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setBaseArrayLayer(0)
                .setMipLevel(0)
                .setLayerCount(1);
            region.setBufferImageHeight(0)
                .setBufferOffset(0)
                .setImageOffset({0, 0, 0})
                .setImageExtent(vk::Extent3D(width, height, 1))
                .setBufferRowLength(0)
                .setImageSubresource(subsource);
            cmdbuf.copyBufferToImage(buf, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);
        }
        cmdbuf.end();
        vk::SubmitInfo submitInfo;
        submitInfo.setCommandBuffers(cmdbuf);
        Context::GetInstance().graphicQueue.submit(submitInfo, nullptr);
        Context::GetInstance().graphicQueue.waitIdle();
        Context::GetInstance().logicaldevice.freeCommandBuffers(Context::GetInstance().render_->cmdpool_, cmdbuf);
    }

    void texture::transitionimageLayoutFromDst2Optimal()
    {
        vk::CommandBufferBeginInfo beginInfo;
        vk::CommandBuffer cmdbuf = Context::GetInstance().render_->CreateCommandBuffer(1)[0];
        beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        cmdbuf.begin(beginInfo);
        {
            vk::ImageMemoryBarrier barrier;
            vk::ImageSubresourceRange range;
            range.setLayerCount(1)
                .setBaseArrayLayer(0)
                .setLevelCount(1)
                .setBaseMipLevel(0)
                .setAspectMask(vk::ImageAspectFlagBits::eColor);
            barrier.setImage(image)
                .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
                .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setSubresourceRange(range);
            cmdbuf.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, nullptr, barrier);
        }
        cmdbuf.end();
        vk::SubmitInfo submitInfo;
        submitInfo.setCommandBuffers(cmdbuf);
        Context::GetInstance().graphicQueue.submit(submitInfo, nullptr);
        Context::GetInstance().graphicQueue.waitIdle();
        Context::GetInstance().logicaldevice.freeCommandBuffers(Context::GetInstance().render_->cmdpool_, cmdbuf);
    }

    void texture::createImageView()
    {
        vk::ImageViewCreateInfo imageViewInfo;
        vk::ImageSubresourceRange subresourceRange;
        subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setBaseArrayLayer(0)
            .setLayerCount(1)
            .setBaseMipLevel(0)
            .setLevelCount(1);
        imageViewInfo.setImage(image)
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(vk::Format::eR8G8B8A8Srgb)
            .setSubresourceRange(subresourceRange);

        imageView = Context::GetInstance().logicaldevice.createImageView(imageViewInfo);
    }

    void texture::generateTextureMipMap()
    {
        vk::PhysicalDeviceProperties properties;
        Context::GetInstance().physicalDevice.getProperties(&properties);
        vk::FormatProperties formatProperties;
        Context::GetInstance().physicalDevice.getFormatProperties(vk::Format::eR8G8B8A8Srgb, &formatProperties);

        vk::SamplerCreateInfo samplerInfo;
        samplerInfo.setMagFilter(vk::Filter::eLinear)
            .setMinFilter(vk::Filter::eLinear)
            .setAddressModeU(vk::SamplerAddressMode::eRepeat)
            .setAddressModeV(vk::SamplerAddressMode::eRepeat)
            .setAddressModeW(vk::SamplerAddressMode::eRepeat)
            .setAnisotropyEnable(vk::True)
            .setMaxAnisotropy(properties.limits.maxSamplerAnisotropy)
            .setBorderColor(vk::BorderColor::eFloatOpaqueBlack)
            .setUnnormalizedCoordinates(vk::False)
            .setCompareEnable(vk::False)
            .setCompareOp(vk::CompareOp::eAlways)
            .setMipmapMode(vk::SamplerMipmapMode::eLinear);

        if (Context::GetInstance().logicaldevice.createSampler(&samplerInfo, nullptr, &sampler) != vk::Result::eSuccess)
            throw std::runtime_error("failed to create texture sampler");
    }

    void texture::createDescriptorPool()
    {
        std::vector<vk::DescriptorPoolSize> poolSizes(2);
        poolSizes[0].setDescriptorCount(static_cast<uint32_t>(maxFrameCount_)).setDescriptorCount(static_cast<uint32_t>(maxFrameCount_));
        poolSizes[1].setDescriptorCount(static_cast<uint32_t>(maxFrameCount_)).setDescriptorCount(static_cast<uint32_t>(maxFrameCount_));

        vk::DescriptorPoolCreateInfo poolInfo;
        poolInfo.setPoolSizeCount(static_cast<uint32_t>(poolSizes.size()))
            .setPoolSizes(poolSizes)
            .setMaxSets(static_cast<uint32_t>(maxFrameCount_));

        if (Context::GetInstance().logicaldevice.createDescriptorPool(&poolInfo, nullptr, &descriptorPool) != vk::Result::eSuccess)
            throw std::runtime_error("failed to create descriptor pool");
    }

    void texture::createDescriptorSets()
    {
        std::vector<vk::DescriptorSetLayout> layouts(maxFrameCount_, Context::GetInstance().renderprocess_->descriptorSetLayouts[0]);
        vk::DescriptorSetAllocateInfo allocInfo;
        allocInfo.setDescriptorPool(descriptorPool)
            .setDescriptorSetCount(static_cast<uint32_t>(maxFrameCount_))
            .setSetLayouts(layouts);

        descriptorSets.resize(maxFrameCount_);
        descriptorSets = Context::GetInstance().logicaldevice.allocateDescriptorSets(allocInfo);

        for (size_t i = 0; i < maxFrameCount_; i++)
        {
            vk::DescriptorBufferInfo bufferInfo;
            bufferInfo.setBuffer(Context::GetInstance().renderprocess_->hostUniformBuffer[i])
                .setOffset(0)
                .setRange(sizeof(MVP));

            vk::DescriptorImageInfo imageInfo;
            imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setImageView(imageView)
                .setSampler(sampler);

            std::array<vk::WriteDescriptorSet, 2> descriptorWrites;

            descriptorWrites[0].setDstSet(descriptorSets[i]).setDstBinding(0).setDstArrayElement(0).setDescriptorType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(1).setBufferInfo(bufferInfo);

            descriptorWrites[1].setDstSet(descriptorSets[i]).setDstBinding(1).setDstArrayElement(0).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setImageInfo(imageInfo);

            Context::GetInstance().logicaldevice.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void texture::InitModel()
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "./models/viking_room.obj"))
        {
            throw std::runtime_error(warn + err);
        }

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto &shape : shapes)
        {
            for (const auto &index : shape.mesh.indices)
            {
                Vertex vertex{};

                vertex.pos = {attrib.vertices[3 * index.vertex_index + 0],
                              attrib.vertices[3 * index.vertex_index + 1],
                              attrib.vertices[3 * index.vertex_index + 2]};

                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

                vertex.color = {1.0f, 1.0f, 1.0f};

                if (uniqueVertices.count(vertex) == 0)
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }

    texture::texture(std::string filename, uint32_t maxFrameCount) : maxFrameCount_(maxFrameCount)
    {
        int32_t texWidth, texHeight, texChannel;
        stbi_uc *pixels = stbi_load((const char *)(filename.data()), &texWidth, &texHeight, &texChannel, STBI_rgb_alpha);
        vk::DeviceSize imagesize = texWidth * texHeight * 4;
        if (!pixels)
        {
            throw std::runtime_error("failed to load texture image");
        }

        vk::Buffer stagebuf = Context::GetInstance().renderprocess_->CreateVkBuffer(imagesize, vk::BufferUsageFlagBits::eTransferSrc);
        vk::DeviceMemory stagebufmemory = Context::GetInstance().renderprocess_->CreateDeviceMemory(stagebuf, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        Context::GetInstance().logicaldevice.bindBufferMemory(stagebuf, stagebufmemory, 0);
        void *ptrstagebuf = Context::GetInstance().logicaldevice.mapMemory(stagebufmemory, 0, imagesize);
        memcpy(ptrstagebuf, pixels, imagesize);

        createTextureImage(texWidth, texHeight);
        allocateMemory();
        createImageView();

        transitionImageLayoutFromUndefine2Dst();
        transformBufferdata2Image(stagebuf, texWidth, texHeight);
        transitionimageLayoutFromDst2Optimal();

        generateTextureMipMap();
        Context::GetInstance().logicaldevice.unmapMemory(stagebufmemory);
        Context::GetInstance().logicaldevice.destroyBuffer(stagebuf);
        Context::GetInstance().logicaldevice.freeMemory(stagebufmemory);
        stbi_image_free(pixels);
    }

    texture::~texture()
    {
        vk::Device logicaldevice = Context::GetInstance().logicaldevice;
        logicaldevice.destroyImage(image);
        logicaldevice.destroyImageView(imageView);
        logicaldevice.destroySampler(sampler);
        logicaldevice.freeMemory(imageMemory);
    }
}
