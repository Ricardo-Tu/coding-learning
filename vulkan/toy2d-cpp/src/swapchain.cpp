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
        swapchaininfo.format = formats[0];
        for (const auto &format : formats)
        {
            if (format.format == vk::Format::eR8G8B8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                swapchaininfo.format = format;
                break;
            }
        }
        auto capbilities = physicaldevice.getSurfaceCapabilitiesKHR(surface);
        swapchaininfo.imageCount = std::clamp<uint32_t>(2, capbilities.minImageCount, capbilities.maxImageCount);
        swapchaininfo.extent.width = std::clamp<uint32_t>(width, capbilities.minImageExtent.width, capbilities.maxImageExtent.width);
        swapchaininfo.extent.height = std::clamp<uint32_t>(height, capbilities.minImageExtent.height, capbilities.maxImageExtent.height);
        swapchaininfo.transform = capbilities.currentTransform;
        auto presents = physicaldevice.getSurfacePresentModesKHR(surface);
        swapchaininfo.presentMode = vk::PresentModeKHR::eFifo;
        for (const auto &present : presents)
        {
            if (present == vk::PresentModeKHR::eMailbox)
            {
                swapchaininfo.presentMode = present;
                break;
            }
        }
    }

    void Swapchain::createImageandImageViews()
    {
        images = Context::GetInstance().logicaldevice.getSwapchainImagesKHR(swapChain);
        imageviews.resize(images.size());
        for (int i = 0; i < images.size(); i++)
        {
            vk::ImageViewCreateInfo ivcreateinfo;
            vk::ImageSubresourceRange range;
            range.setBaseMipLevel(0)
                .setBaseArrayLayer(0)
                .setLevelCount(1)
                .setLayerCount(1)
                .setAspectMask(vk::ImageAspectFlagBits::eColor);

            ivcreateinfo.setImage(images[i])
                .setFormat(swapchaininfo.format.format)
                .setViewType(vk::ImageViewType::e2D)
                .setComponents(vk::ComponentMapping{})
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
            fbcreateinfo.setRenderPass(Context::GetInstance().renderprocess_->renderPass)
                .setAttachmentCount(1)
                .setPAttachments(&imageviews[i])
                .setWidth(width)
                .setHeight(height)
                .setLayers(1);
            framebuffers[i] = Context::GetInstance().logicaldevice.createFramebuffer(fbcreateinfo);
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
            .setImageColorSpace(swapchaininfo.format.colorSpace)
            .setImageFormat(swapchaininfo.format.format)
            .setImageExtent(swapchaininfo.extent)
            .setMinImageCount(swapchaininfo.imageCount)
            .setPresentMode(vk::PresentModeKHR::eFifo)
            .setPreTransform(swapchaininfo.transform)
            .setOldSwapchain(nullptr)
            .setClipped(true)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);

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
        swapChain = Context::GetInstance().logicaldevice.createSwapchainKHR(swapchaincreateinfo);

        std::cout << "create swapchain 1" << std::endl;
        // cleanup();
        // Context::GetInstance().logicaldevice.destroySwapchainKHR(swapchain);
        std::cout << "create swapchain 2" << std::endl;
        // cleanup();
        // createImageandImageViews();
        std::cout << "create swapchain 3" << std::endl;
    }
    void Swapchain::cleanup()
    {
        std::cout << "entry cleanup" << std::endl;
        Context::GetInstance().logicaldevice.destroySwapchainKHR(swapChain);
        std::cout << "leave cleanup" << std::endl;
    }

    Swapchain::~Swapchain()
    {
        std::cout << "entry destroy swapchain" << std::endl;
        cleanup();
        // Context::GetInstance().logicaldevice.destroySwapchainKHR(swapchain);
        std::cout << "leave swapchain" << std::endl;
    }
}