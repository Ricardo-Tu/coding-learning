#pragma once
#include <iostream>
#include <string>
#include <vulkan/vulkan.hpp>
#include <string_view>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
namespace toy2d
{
    class texture
    {
    public:
        vk::Image image;
        vk::DeviceMemory imageMemory;
        vk::ImageView imageView;
        texture(std::string_view filename);
        ~texture();
        void createTextureImage(uint32_t width, uint32_t height);
        void allocateMemory();
        void generateTextureMipMap();
        uint32_t queryImageMemoryIndex(uint32_t flag, vk::MemoryPropertyFlagBits property);
        void transitionImageLayoutFromUndefine2Dst();
        void transformBufferdata2Image(vk::Buffer &buf, uint32_t width, uint32_t height);
        void transitionimageLayoutFromDst2Optimal();

    private:
    };
}