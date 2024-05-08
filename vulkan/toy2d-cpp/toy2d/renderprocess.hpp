#pragma once
#include <memory>
#include <iostream>
#include <stdexcept>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan.hpp>

namespace toy2d
{
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;
        bool operator==(const Vertex &other) const
        {
            return pos == other.pos && color == other.color && texCoord == other.texCoord;
        }
    };
    struct MVP
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 project;
    };
    class RenderProcess final
    {
    public:
        vk::RenderPass renderPass;
        vk::Pipeline pipeline;
        vk::PipelineLayout pipelineLayout;
        vk::DescriptorSetLayout descriptorSetLayout;
        vk::Buffer hostVertexBuffer;
        vk::Buffer gpuVertexBuffer;
        vk::Buffer indexBuffer;
        std::vector<vk::Buffer> hostUniformBuffer;
        vk::DeviceMemory hostBufferMemory;
        vk::DeviceMemory gpuBufferMemory;
        vk::DeviceMemory indexBufferMemory;
        std::vector<vk::DeviceMemory> hostUniformBufferMemory;
        vk::DescriptorPool descriptorPool;
        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
        std::vector<void *> hostUniformBufferMemoryPtr;
        std::vector<vk::DescriptorSet> descriptorSets;
        void InitRenderPass();
        void InitRenderPassLayout();
        void InitPipeline();
        void InitLayoutDescriptorSet(uint32_t count);
        vk::DeviceMemory CreateDeviceMemory(vk::Buffer buf, vk::MemoryPropertyFlags flags);
        vk::Buffer CreateVkBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage);
        void CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
        void CreateCommandDescriptorSets();
        void CreateVertexBuffer();
        void CreateIndexBuffer();
        void CreateUniformBuffer(uint32_t count);
        RenderProcess();
        ~RenderProcess();
        uint32_t maxFramesCount_ = 2;

    private:
    };
    extern MVP mvp;
    extern std::vector<Vertex> vertices;
    extern std::vector<uint32_t> indices;
}