#pragma once
#include <vulkan/vulkan.hpp>

namespace toy2d
{
    class Swapchain
    {
    public:
        vk::SwapchainKHR swapchain;
        Swapchain(int width, int height);
        ~Swapchain();
        struct SwapchainInfo
        {
            vk::Extent2D extent;
            uint32_t imageCount;
            vk::SurfaceFormatKHR format;
            vk::SurfaceTransformFlagsKHR transform;
            vk::PresentModeKHR presentMode;
        };
        SwapchainInfo info;
        std::vector<vk::Image> images;
        std::vector<vk::ImageView> imageviews;
        std::vector<vk::Framebuffer> framebuffers;

        void queryInfo(int width, int height);
        void getImages();
        void createimageViews();
        void createFramebuffers(int width, int height);
    };
}