#pragma once
#include <iostream>
#include <string>
#include <vulkan/vulkan.hpp>
#include <string_view>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <tiny_obj_loader.h>

#include "renderprocess.hpp"

namespace toy2d
{
    class texture
    {
    public:
        vk::Image image;
        vk::DeviceMemory imageMemory;
        vk::ImageView imageView;
        vk::Sampler sampler;
        vk::DescriptorPool descriptorPool;
        std::vector<vk::DescriptorSet> descriptorSets;
        texture(std::string filename, uint32_t maxFrameCount);
        ~texture();
        void createTextureImage(uint32_t width, uint32_t height);
        void allocateMemory();
        void generateTextureMipMap();
        uint32_t queryImageMemoryIndex(uint32_t flag, vk::MemoryPropertyFlagBits property);
        void createImageView();
        void transitionImageLayoutFromUndefine2Dst();
        void transformBufferdata2Image(vk::Buffer &buf, uint32_t width, uint32_t height);
        void transitionimageLayoutFromDst2Optimal();
        void createDescriptorPool();
        void createDescriptorSets();
        void InitModel();
        uint32_t maxFrameCount_ = 2;

    private:
    };
}

namespace std {
    template<> struct hash<toy2d::Vertex> {
        size_t operator()(toy2d::Vertex const& vertex) const
        {
            return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}