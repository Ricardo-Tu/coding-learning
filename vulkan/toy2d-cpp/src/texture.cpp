#include "../toy2d/context.hpp"
#include "../toy2d/texture.hpp"
#include "../toy2d/stb_image.h"
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

        auto index = queryBufferMemTypeIndex(requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
        allocInfo.setMemoryTypeIndex(index);

        imageMemory = logicaldevice.allocateMemory(allocInfo);
    }

    uint32_t texture::queryImageMemoryIndex()
    {
        return 0;
    }

    void texture::transitionImageLayoutFromUndefine2Dst()
    {
        Context::GetInstance().commandManager->ExecuteCmd(Context::GetInstance().graphicQueue,
                                                          [&](vk::CommandBuffer cmdBuf)
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
                                                              cmdBuf.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, {}, nullptr, barrier);
                                                          });
    }

    void texture::transitionimageLayoutFromDst2Optimal()
    {
        Context::GetInstance().commandManager->ExecuteCmd(Context::GetInstance().graphicQueue,
                                                          [&](vk::CommandBuffer cmdBuf)
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
                                                              cmdBuf.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, nullptr, barrier);
                                                          });
    }

    void texture::transformData2Image(gpuBuffer &buffer, uint32_t width, uint32_t height)
    {
        Context::GetInstance().commandManager->ExecuteCmd(Context::GetInstance().graphicQueue, [&](vk::CommandBuffer cmdBuf)
                                                          { vk::BufferImageCopy region;
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
                                                          cmdBuf.copyBufferToImage(buffer.vkbufferRegion, image, vk::ImageLayout::eTransferDstOptimal, 1, &region); });
    }

    texture::texture(std::string_view filename)
    {
        int width, height, channel;
        stbi_uc *pixels = stbi_load(filename.data(), &width, &height, &channel, STBI_rgb_alpha);
        size_t size = width * height * 4;
        if (!pixels)
        {
            throw std::runtime_error("failed to load texture image");
        }

        std::unique_ptr<gpuBuffer> buf(new gpuBuffer(vk::BufferUsageFlagBits::eTransferSrc, size, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));

        memcpy(buf->map, pixels, size);

        createTextureImage(width, height);

        allocateMemory();

        Context::GetInstance().logicaldevice.bindImageMemory(image, imageMemory, 0);

        transitionImageLayoutFromUndefine2Dst();
        transformData2Image(*buf, width, height);
        transitionimageLayoutFromDst2Optimal();

        createImageView();
        stbi_image_free(pixels);
    }

    texture::~texture()
    {
        vk::Device logicaldevice = Context::GetInstance().logicaldevice;
        logicaldevice.destroyImageView(imageView);
    }
}
