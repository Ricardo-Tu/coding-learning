#pragma once
#include <memory>
#include <iostream>
#include <stdexcept>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace toy2d
{
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;
    };
    class RenderProcess final
    {
    public:
        vk::RenderPass renderPass;
        vk::Pipeline pipeline;
        vk::PipelineLayout pipelineLayout;
        vk::Buffer hostVertexBuffer;
        vk::Buffer gpuVertexBuffer;
        vk::DeviceMemory hostBufferMemory;
        vk::DeviceMemory gpuBufferMemory;
        void InitRenderPass();
        void InitRenderPassLayout();
        void InitPipeline(int width, int height);
        vk::DeviceMemory CreateDeviceMemory(vk::Buffer buf, vk::MemoryPropertyFlags flags);
        vk::Buffer CreateVkBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage);
        void CreateVertexBuffer();
        RenderProcess();
        ~RenderProcess();

    private:
    };
}