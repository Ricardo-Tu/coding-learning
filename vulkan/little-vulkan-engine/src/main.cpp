#include "../lve/lve_windows.hpp"
#include "../lve/main.hpp"

#define WINDOWS_WIDTH 1920
#define WINDOWS_HEIGHT 1080
#define WINDOWS_NAME "LittleVulkanEngine!"

int main(void)
{
    lve::App app(WINDOWS_WIDTH, WINDOWS_HEIGHT, WINDOWS_NAME);
    app.run();
    return 0;
}
