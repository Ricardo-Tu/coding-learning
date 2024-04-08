#pragma once
#include <memory>
#include <iostream>
#include <optional>
#include <functional>
#include <vector>
#include <array>
#include <vulkan/vulkan.hpp>

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
        vk::Instance CreateInstance(std::vector<const char *> extensions);
        vk::PhysicalDevice PickupPhysicalDevice();
        vk::Device CreateLogicalDevice();
        static void Init(std::vector<const char *> extensions, std::function<vk::SurfaceKHR(vk::Instance)> retsurface);
        static void Quit();
        static Context &GetInstance();
        void QueryQueueFamilyIndexes();
        ~Context();

    private:
        Context(std::vector<const char *> extensions,
                std::function<vk::SurfaceKHR(vk::Instance)> retsurface);
        static std::unique_ptr<Context> instance_;
    };
}