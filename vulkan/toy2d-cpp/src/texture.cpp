#include "../toy2d/context.hpp"
#include "../toy2d/texture.hpp"
#include "../toy2d/buffer.hpp"
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
                .setOldLayout(vk::ImageLayout::eUndefined)
                .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
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
                .setImageOffset(0)
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
            .setAnisotropyEnable(vk::False)
            // .setMaxAnisotropy(properties.limits.maxSamplerAnisotropy)
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

    texture::texture(std::string filename, uint32_t maxFrameCount) : maxFrameCount_(maxFrameCount)
    {
        int32_t width, height, channel;
        stbi_uc *pixels = stbi_load((const char *)(filename.data()), &width, &height, &channel, STBI_rgb_alpha);
        vk::DeviceSize imagesize = width * height * 4;
        if (!pixels)
        {
            throw std::runtime_error("failed to load texture image");
        }

        vk::Buffer stagebuf = Context::GetInstance().renderprocess_->CreateVkBuffer(imagesize, vk::BufferUsageFlagBits::eTransferSrc);
        vk::DeviceMemory stagebufmemory = Context::GetInstance().renderprocess_->CreateDeviceMemory(stagebuf, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostVisible);
        Context::GetInstance().logicaldevice.bindBufferMemory(stagebuf, stagebufmemory, 0);
        void *ptrstagebuf = Context::GetInstance().logicaldevice.mapMemory(stagebufmemory, 0, imagesize);
        memcpy(ptrstagebuf, pixels, imagesize);

        createTextureImage(width, height);
        createImageView();

        allocateMemory();

        transitionImageLayoutFromUndefine2Dst();
        transformBufferdata2Image(stagebuf, width, height);
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
        logicaldevice.destroyImageView(imageView);
    }
}
