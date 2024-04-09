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
        Context::GetInstance().logicaldevice.destroySwapchainKHR(swapchain);
    }
}