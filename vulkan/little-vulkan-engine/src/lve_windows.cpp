#include "../lve/lve_windows.hpp"

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
    }

    LveWindow::~LveWindow()
    {
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
    void LveWindow::lveRun()
    {
        SDL_Renderer *renderer;
        renderer = SDL_CreateRenderer(window, -1, 0);
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderClear(renderer);
        while (!quitFlag)
        {
            if (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    quitFlag = true;
                }
                else if (event.type == SDL_WINDOWEVENT_RESIZED || event.type == SDL_WINDOWEVENT_SIZE_CHANGED || event.type == SDL_WINDOWEVENT_RESTORED)
                {
                    std::cout << "Window resized to " << event.window.data1 << "x" << event.window.data2 << std::endl;
                }
            }
            SDL_RenderPresent(renderer);
        }
    }

    void LveWindow::GetWIndowSize(uint32_t &width, uint32_t &height)
    {
        SDL_GetWindowSize(window, (int *)&width, (int *)&height);
    }

    App::App(uint32_t width, uint32_t height, const std::string windowname)
    {
        lvewindow = new LveWindow(width, height, windowname);
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