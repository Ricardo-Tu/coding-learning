#pragma once
#include <iostream>
#include <stdexcept>
#include <vulkan/vulkan.hpp>

namespace lve {
    class LveDeviceEngine {
    public:
    #ifdef NDEBUG
        const bool enableValidationLayers = false;
    #else
        const bool enableValidationLayers = true;
    #endif
        LveDeviceEngine();
        ~LveDeviceEngine();
    private:
        bool createVulkanEngineInstance();
        bool checkValidationLayerSupport();
    };
}