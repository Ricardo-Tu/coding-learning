#include "../lve/lve_device_engine.hpp"
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_structs.hpp>

namespace lve
{
    LveDeviceEngine::LveDeviceEngine()
    {
    }

    LveDeviceEngine::~LveDeviceEngine()
    {
    }

    bool LveDeviceEngine::createVulkanEngineInstance()
    {
        vk::InstanceCreateInfo createInfo;
        vk::ApplicationInfo appInfo;
        if (enableValidationLayers)
            if (!checkValidationLayerSupport())
            {
                throw std::runtime_error("Validation layers requested, but not available!");
                return false;
            }
        appInfo.setApplicationVersion(VK_API_VERSION_1_3)
            .setPEngineName("no engine")
            .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
            .setPApplicationName("little vulkan engine")
            .setEngineVersion(VK_MAKE_VERSION(1, 0, 0));

        createInfo.setPApplicationInfo(&appInfo)
            .setPEnabledExtensionNames(nullptr)
            .set
        
        return true;
    }

    bool LveDeviceEngine::checkValidationLayerSupport()
    {
        uint32_t layerCount = 0;
        vk::Result result = vk::Result::eSuccess;
        result = vk::enumerateInstanceLayerProperties(&layerCount, nullptr);
        if (result != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to enumerate instance layer properties");
            return false;
        }

        return true;
    }
}