#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <SDL2/SDL_video.h>
#include <iostream>
#include <string>
#include <stdexcept>

namespace lve
{
    class LveWindow
    {
    public:
        LveWindow(int width, int height, const std::string windowname);
        ~LveWindow();
        void ResizeWindow(uint32_t *width, uint32_t *height);
        void lveRun();
        LveWindow(const LveWindow &) = delete;
        LveWindow &operator=(const LveWindow &) = delete;

    private:
        uint32_t width;
        uint32_t height;
        std::string windowname;
        SDL_Window *window;
        SDL_Event event;
        SDL_Renderer *renderer;
        bool quitFlag = 0;
        void renderColor(uint32_t r, uint32_t g, uint32_t b, uint32_t a);
    };

    class App
    {
    public:
        LveWindow *lvewindow;
        App(uint32_t width, uint32_t height, const std::string windowname);
        ~App();
        void run();

    private:
    };
}