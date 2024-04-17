#include <algorithm>
#include "../toy2d/context.hpp"
#include "../toy2d/swapchain.hpp"

namespace toy2d
{
    void Swapchain::queryInfo(int width, int height)
    {
        auto physicaldevice = Context::GetInstance().physicalDevice;
        auto surface = Context::GetInstance().surface;
        auto formats = physicaldevice.getSurfaceFormatsKHR(surface);
        info.format = formats[0];
        for (const auto &format : formats)
        {
            if (format.format == vk::Format::eR8G8B8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                info.format = format;
                break;
            }
        }
        auto capbilities = physicaldevice.getSurfaceCapabilitiesKHR(surface);
        info.imageCount = std::clamp<uint32_t>(2, capbilities.minImageCount, capbilities.maxImageCount);
        info.extent.width = std::clamp<uint32_t>(width, capbilities.minImageExtent.width, capbilities.maxImageExtent.width);
        info.extent.height = std::clamp<uint32_t>(height, capbilities.minImageExtent.height, capbilities.maxImageExtent.height);
        info.transform = capbilities.currentTransform;
        auto presents = physicaldevice.getSurfacePresentModesKHR(surface);
        info.presentMode = vk::PresentModeKHR::eFifo;
        for (const auto &present : presents)
        {
            if (present == vk::PresentModeKHR::eMailbox)
            {
                info.presentMode = present;
                break;
            }
        }
    }

    Swapchain::Swapchain(int width, int height)
    {
        queryInfo(width, height);
        vk::SwapchainCreateInfoKHR swapchaincreateinfo;
        swapchaincreateinfo.setClipped(true)
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setSurface(Context::GetInstance().surface)
            .setImageColorSpace(info.format.colorSpace)
            .setImageFormat(info.format.format)
            .setImageExtent(info.extent)
            .setMinImageCount(info.imageCount)
            .setPresentMode(info.presentMode);

        auto &queueIndexes = Context::GetInstance().queueInfo;
        if (queueIndexes.graphicsFamilyIndex.value() == queueIndexes.presentFamilyIndex.value())
        {
            swapchaincreateinfo.setQueueFamilyIndices(queueIndexes.graphicsFamilyIndex.value())
                .setImageSharingMode(vk::SharingMode::eExclusive);
        }
        else
        {
            std::array queueFamilyIndexes = {queueIndexes.graphicsFamilyIndex.value(), queueIndexes.presentFamilyIndex.value()};
            swapchaincreateinfo.setQueueFamilyIndices(queueFamilyIndexes)
                .setImageSharingMode(vk::SharingMode::eConcurrent);
        }
        swapchain = Context::GetInstance().logicaldevice.createSwapchainKHR(swapchaincreateinfo);
    }

    Swapchain::~Swapchain()
    {
        for (auto &fb : framebuffers)
            Context::GetInstance().logicaldevice.destroyFramebuffer(fb);

        for (auto &view : imageviews)
            Context::GetInstance().logicaldevice.destroyImageView(view);

        Context::GetInstance().logicaldevice.destroySwapchainKHR(swapchain);
    }

    void Swapchain::getImages()
    {
        images = Context::GetInstance().logicaldevice.getSwapchainImagesKHR(swapchain);
    }

    void Swapchain::createimageViews()
    {
        imageviews.resize(images.size());
        for (int i = 0; i < images.size(); i++)
        {
            vk::ImageViewCreateInfo ivcreateinfo;
            vk::ComponentMapping mapping;
            vk::ImageSubresourceRange range;
            range.setBaseMipLevel(0)
                .setLevelCount(1)
                .setBaseArrayLayer(0)
                .setLayerCount(1)
                .setAspectMask(vk::ImageAspectFlagBits::eColor);

            ivcreateinfo.setImage(images[i])
                .setViewType(vk::ImageViewType::e2D)
                .setComponents(mapping)
                .setFormat(info.format.format)
                .setSubresourceRange(range);
            imageviews[i] = Context::GetInstance().logicaldevice.createImageView(ivcreateinfo);
        }
    }

    void Swapchain::createFramebuffers(int width, int height)
    {
        framebuffers.resize(images.size());
        for (int i = 0; i < images.size(); i++)
        {
            vk::FramebufferCreateInfo fbcreateinfo;
            fbcreateinfo.setRenderPass(Context::GetInstance().renderprocess->renderPass)
                .setAttachmentCount(1)
                .setPAttachments(&imageviews[i])
                .setWidth(width)
                .setHeight(height)
                .setLayers(1);
            framebuffers[i] = Context::GetInstance().logicaldevice.createFramebuffer(fbcreateinfo);
        }
    }
}