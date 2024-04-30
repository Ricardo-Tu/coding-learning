#pragma once
#include <iostream>
#include <string>
#include <vulkan/vulkan.hpp>
#include <string_view>

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
        uint32_t maxFrameCount_ = 2;

    private:
    };
}