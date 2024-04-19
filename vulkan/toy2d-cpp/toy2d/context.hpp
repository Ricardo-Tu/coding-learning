#pragma once
#include <memory>
#include <iostream>
#include <optional>
#include <functional>
#include <vector>
#include <array>
#include <vulkan/vulkan.hpp>
#include <algorithm>
#include "swapchain.hpp"
#include "render.hpp"
#include "renderprocess.hpp"

namespace toy2d
{
    class Context final
    {
    public:
        struct QueueFamilyIndexes final
        {
            std::optional<uint32_t> graphicsFamilyIndex;
            std::optional<uint32_t> presentFamilyIndex;
        };
        QueueFamilyIndexes queueInfo;
        vk::PhysicalDevice physicalDevice;
        vk::Instance instance;
        vk::Device logicaldevice;
        vk::Queue graphicQueue;
        vk::Queue presentQueue;
        vk::SurfaceKHR surface = nullptr;
        static std::unique_ptr<Swapchain> swapchain_;
        static std::unique_ptr<RenderProcess> renderprocess_;
        static std::unique_ptr<render> render_;
        static std::unique_ptr<Context> instance_;
        vk::Instance CreateInstance(std::vector<const char *> extensions);
        vk::PhysicalDevice PickupPhysicalDevice();
        vk::Device CreateLogicalDevice();
        static void Init(std::vector<const char *> extensions, std::function<vk::SurfaceKHR(vk::Instance)> retsurface);
        static void Quit();
        static Context &GetInstance();
        void QueryQueueFamilyIndexes();
        void InitSwapchain(int width, int height);
        Context(std::vector<const char *> extensions,
                std::function<vk::SurfaceKHR(vk::Instance)> retsurface);
        ~Context();

    private:
    };
}