#pragma once
#include <iostream>
#include <string>
#include <vulkan/vulkan.hpp>
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
        void createImageView();
        uint32_t queryImageMemoryIndex();
        void transitionImageLayoutFromUndefine2Dst();
        void transitionimageLayoutFromDst2Optimal();
        void transformData2Image(gpuBuffer &buffer, uint32_t width, uint32_t height);

    private:
    };
}