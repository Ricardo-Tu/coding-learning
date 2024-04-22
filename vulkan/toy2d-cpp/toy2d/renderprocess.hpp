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
        vk::Buffer vertexBuffer;
        vk::DeviceMemory vertexBufferMemory;
        void InitRenderPass();
        void InitRenderPassLayout();
        void InitPipeline(int width, int height);
        void CreateVertexBuffer();
        RenderProcess();
        ~RenderProcess();

    private:
    };
}