#include "../lve/lve_windows.hpp"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>

namespace lve
{
    LveWindow::LveWindow(int width, int height, const std::string windowname) : width(width), height(height), windowname(windowname)
    {
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        {
            std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
            throw std::runtime_error("SDL_Init Error: " + std::string(SDL_GetError()));
        }

        window = SDL_CreateWindow(windowname.c_str(),
                                  SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED,
                                  width,
                                  height,
                                  SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
        if (window == nullptr)
        {
            SDL_Quit();
            std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
            throw std::runtime_error("SDL_CreateWindow Error: " + std::string(SDL_GetError()));
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    }

    LveWindow::~LveWindow()
    {
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
    void LveWindow::lveRun()
    {
        while (!quitFlag)
        {
            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                case SDL_QUIT:
                    quitFlag = true;
                    break;
                case SDL_WINDOWEVENT:
                    switch (event.window.event)
                    {
                    case SDL_WINDOWEVENT_RESIZED:
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    case SDL_WINDOWEVENT_MAXIMIZED:
                    case SDL_WINDOWEVENT_RESTORED:
                    case SDL_WINDOWEVENT_MINIMIZED:
                        std::cout << "Window resized to " << event.window.data1 << "x" << event.window.data2 << std::endl;
                        ResizeWindow(&this->width, &this->height);
                        renderColor(255, 0, 0, 255);
                        break;
                    default:
                        break;
                    }
                    break;
                default:
                    break;
                }
                renderColor(255, 0, 0, 255);
            }
        }
    }

    void LveWindow::renderColor(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
    {
        SDL_SetRenderDrawColor(renderer, r, g, b, a);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

    void LveWindow::ResizeWindow(uint32_t *pWidth, uint32_t *pHeight)
    {
        SDL_GetWindowSize(window, (int *)pWidth, (int *)pHeight);
        SDL_SetWindowSize(window, (int)*pWidth, (int)*pHeight);
    }

    App::App(uint32_t width, uint32_t height, const std::string WindowsName)
    {
        lvewindow = new LveWindow(width, height, WindowsName);
    }

    App::~App()
    {
        delete lvewindow;
    }

    void App::run()
    {
        lvewindow->lveRun();
    }
}