#include "../toy2d/context.hpp"
#include "../toy2d/texture.hpp"
#include "../toy2d/buffer.hpp"

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

        auto requirements = logicaldevice.getImageMemoryRequirements(image);
        allocInfo.setAllocationSize(requirements.size);

        auto index = queryImageMemoryIndex(requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
        allocInfo.setMemoryTypeIndex(index);

        imageMemory = logicaldevice.allocateMemory(allocInfo);
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
            vk::ImageMemoryBarrier barrier;
            vk::ImageSubresourceRange range;
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

    void texture::generateTextureMipMap()
    {
        vk::FormatProperties formatProperties;
        Context::GetInstance().physicalDevice.getFormatProperties(vk::Format::eR8G8B8A8Srgb, &formatProperties);
        if(formatProperties.optimalTilingFeatures )
        
    }

    texture::texture(std::string_view filename)
    {
        int width, height, channel;
        stbi_uc *pixels = stbi_load(filename.data(), &width, &height, &channel, STBI_rgb_alpha);
        vk::DeviceSize imagesize = width * height * 4;
        if (!pixels)
        {
            throw std::runtime_error("failed to load texture image");
        }

        vk::Buffer stagebuf = Context::GetInstance().renderprocess_->CreateVkBuffer(imagesize, vk::BufferUsageFlagBits::eTransferSrc);
        vk::DeviceMemory stagebufmemory = Context::GetInstance().renderprocess_->CreateDeviceMemory(stagebuf, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostVisible);
        void *ptrstagebuf = Context::GetInstance().logicaldevice.mapMemory(stagebufmemory, 0, imagesize);
        memcpy(ptrstagebuf, pixels, imagesize);

        createTextureImage(width, height);

        allocateMemory();

        Context::GetInstance().logicaldevice.bindImageMemory(image, imageMemory, 0);

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
