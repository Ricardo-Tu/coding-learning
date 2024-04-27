#pragma once
#include <vulkan/vulkan.hpp>

namespace toy2d
{
    class gpuBuffer 
    {
    public:
        gpuBuffer(vk::BufferUsageFlags usage, vk::DeviceSize size, vk::MemoryPropertyFlags properties);
        ~gpuBuffer();
        void *map;
        vk::gpuBuffer vkbufferRegion;
        void createBuffer();
        void allocateMemory();
        void copyData();
        void queryMemoryIndex();
        void transitionBufferLayout();
        void transformData2Buffer();

    private:
    };
}