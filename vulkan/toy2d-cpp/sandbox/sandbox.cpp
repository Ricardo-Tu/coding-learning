#include <stdint.h>
#include <iostream>
#include <vector>
#include "../toy2d/toy2d.hpp"
#include "../toy2d/render.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <SDL2/SDL_video.h>

void windowSizeChange()
{
    SDL_GetWindowSize(toy2d::window, (int *)&toy2d::WindowWidth, (int *)&toy2d::WindowHeight);
    toy2d::framebufferResizeFlag = true;
    std::cout << "window size change" << toy2d::WindowWidth << "  " << toy2d::WindowHeight << std::endl;
}

int main(int argc, char *argv[])
{

    SDL_Init(SDL_INIT_EVENTS);
    toy2d::window = SDL_CreateWindow("sandbox",
                                     SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                     toy2d::WindowWidth, toy2d::WindowHeight,
                                     SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);

    if (!toy2d::window)
    {
        SDL_Log("Create window failed");
        exit(2);
    }

    bool shouldClose = false;
    SDL_Event event;
    uint32_t count = 0;
    SDL_Vulkan_GetInstanceExtensions(toy2d::window, &count, nullptr);
    std::vector<const char *> extensions(count);
    SDL_Vulkan_GetInstanceExtensions(toy2d::window, &count, extensions.data());
    toy2d::Init(
        extensions, [&](vk::Instance instance) -> vk::SurfaceKHR
        { vk::SurfaceKHR surface;
                  SDL_Vulkan_CreateSurface(toy2d::window , instance, (VkSurfaceKHR*)&surface);
                  return surface; },
        toy2d::WindowWidth, toy2d::WindowHeight);
    auto &renderer = toy2d::GetRenderInstance();

    while (!shouldClose)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                shouldClose = true;
            }
            else if (event.type == SDL_WINDOWEVENT_RESIZED || event.type == SDL_WINDOWEVENT_SIZE_CHANGED || event.type == SDL_WINDOWEVENT_RESTORED)
            {
                windowSizeChange();
            }
        }
        renderer.DrawColorTriangle();
    }

    toy2d::Quit();
    SDL_DestroyWindow(toy2d::window);
    SDL_Quit();

    // SDL_Init(SDL_INIT_EVERYTHING);
    // SDL_Window *window = SDL_CreateWindow("sandbox",
    //                                       SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    //                                       WindowWidth, WindowHeight,
    //                                       SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
    // if (!window)
    // {
    //     SDL_Log("create window failed");
    //     exit(2);
    // }

    // uint32_t count = 0;
    // SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
    // std::vector<const char *> extensions = std::vector<const char *>(count);
    // SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data());
    // toy2d::init(
    //     extensions,
    //     [&](VkInstance instance)
    //     {
    //         VkSurfaceKHR surface;
    //         SDL_Vulkan_CreateSurface(window, instance, &surface);
    //         return surface;
    //     },
    //     1024, 720);
    // auto renderer = toy2d::GetRenderer();
    // bool shouldClose = false;
    // SDL_Event event;
    // floadt x = 100, y = 100;
    // toy2d::Texture *texture1 = toy2d::LoadTexture("resources/tole.png");
    // toy2d::Texture *texture2 = toy2d::LoadTexture("resources/texture.jpg");

    // while (!shouldClose)
    // {
    //     while (SDL_PollEvent(&event))
    //     {
    //         if (event.type == SDL_QUIT)
    //         {
    //             shouldClose = true;
    //         }
    //         if (event.type == SDL_KEYDOWN){
    //             if(event.key.keysym.sym == SDLK_a){
    //                 x -= 10;
    //             }
    //             else if(event.key.keysym.sym == SDLK_d){
    //                 x += 10;
    //             }
    //             else if(event.key.keysym.sym == SDLK_w){
    //                 y -= 10;
    //             }
    //             else if(event.key.keysym.sym == SDLK_s){
    //                 y += 10;
    //             }
    //         }
    //     }
    //     if (event.type == SDL_WINDOWEVENT){
    //         if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
    //             toy2d::ResizeSwapchainImage(event.window.data1, event.window.data2);
    //         }
    //     }
    // }
    // renderer->StartRender();
    // renderer->SetDrawColor(toy2d::Color{1, 0, 0});
    // renderer->DrawTexture(toy2d::rect{toy2d::vec{x, y}, toy2d::Size{200, 300}}, *texture);
    return 0;
}
