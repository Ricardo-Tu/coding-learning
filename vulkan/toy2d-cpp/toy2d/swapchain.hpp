#pragma once
#include <vulkan/vulkan.hpp>

namespace toy2d
{

    class Swapchain
    {
    public:
        Swapchain(int width, int height);
        ~Swapchain();
        struct SwapchainInfo
        {
            vk::Extent2D extent;
            uint32_t imageCount;
            vk::SurfaceFormatKHR format;
            vk::SurfaceTransformFlagBitsKHR transform;
            vk::PresentModeKHR presentMode;
        };
        SwapchainInfo swapchaininfo;
        std::vector<vk::Image> images;
        std::vector<vk::ImageView> imageviews;
        std::vector<vk::Framebuffer> framebuffers;
        vk::SwapchainKHR swapChain;
        void queryInfo(int width, int height);
        void createImageandImageViews();
        void createFramebuffers();

    private:
    };
}