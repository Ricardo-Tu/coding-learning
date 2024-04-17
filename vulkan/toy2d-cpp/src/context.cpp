#include "../toy2d/context.hpp"
#include "../toy2d/shader.hpp"
namespace toy2d
{
    std::unique_ptr<Context> Context::instance_ = nullptr;
    std::unique_ptr<Swapchain> Context::swapchain = nullptr;
    std::unique_ptr<RenderProcess> Context::renderprocess = nullptr;
    void Context::Init(std::vector<const char *> extensions, std::function<vk::SurfaceKHR(vk::Instance)> retsurface)
    {
        Context::instance_.reset(new Context(extensions, retsurface));
    }

    void Context::Quit()
    {
        instance_.release();
    }

    Context &Context::GetInstance()
    {
        return *instance_;
    }

    vk::Instance Context::CreateInstance(std::vector<const char *> extensions)
    {
        vk::InstanceCreateInfo info;
        vk::ApplicationInfo appInfo;
        std::vector<const char *> layers = {"VK_LAYER_KHRONOS_validation"};
        appInfo.setApiVersion(VK_API_VERSION_1_3)
            .setPApplicationName("vulkan tutoial")
            .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
            .setPEngineName("No_Engine")
            .setEngineVersion(VK_MAKE_VERSION(1, 0, 0));

        info.setPApplicationInfo(&appInfo)
            .setPEnabledExtensionNames(extensions)
            .setPEnabledLayerNames(layers);

        return vk::createInstance(info);
    }

    vk::PhysicalDevice Context::PickupPhysicalDevice()
    {
        auto physicalDevices = instance.enumeratePhysicalDevices();
        for (auto &device : physicalDevices)
        {
            if (device.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
            {
                std::cout << "discrete gpu:" << std::endl;
                std::cout << device.getProperties().deviceName << std::endl;
                physicalDevice = device;
            }
            else if (device.getProperties().deviceType == vk::PhysicalDeviceType::eIntegratedGpu)
            {
                std::cout << "integrated gpu:" << std::endl;
                std::cout << device.getProperties().deviceName << std::endl;
            }
            else if (device.getProperties().deviceType == vk::PhysicalDeviceType::eCpu)
            {
                std::cout << "eCpu gpu:" << std::endl;
                std::cout << device.getProperties().deviceName << std::endl;
            }
            else if (device.getProperties().deviceType == vk::PhysicalDeviceType::eVirtualGpu)
            {
                std::cout << "eVirtualGpu gpu:" << std::endl;
                std::cout << device.getProperties().deviceName << std::endl;
            }
            else if (device.getProperties().deviceType == vk::PhysicalDeviceType::eOther)
            {
                std::cout << "eOther gpu:" << std::endl;
                std::cout << device.getProperties().deviceName << std::endl;
            }
        }
        std::cout << "----------" << std::endl;
        std::cout << "select gpu" << std::endl;
        std::cout << physicalDevice.getProperties().deviceName << std::endl;
        std::cout << "----------" << std::endl;
        return physicalDevice;
    }

    vk::Device Context::CreateLogicalDevice()
    {
        vk::DeviceCreateInfo devicecreateinfo;
        std::array deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        QueryQueueFamilyIndexes();
        float priority = 1.0f;
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        vk::DeviceQueueCreateInfo queueCreateInfo;
        if (queueInfo.graphicsFamilyIndex.value() == queueInfo.presentFamilyIndex.value())
        {
            queueCreateInfo.setQueueFamilyIndex(queueInfo.graphicsFamilyIndex.value())
                .setQueueCount(1)
                .setPQueuePriorities(&priority);
            queueCreateInfos.push_back(queueCreateInfo);
        }
        else
        {
            queueCreateInfo.setQueueFamilyIndex(queueInfo.graphicsFamilyIndex.value())
                .setQueueCount(1)
                .setPQueuePriorities(&priority);
            queueCreateInfos.push_back(queueCreateInfo);
            queueCreateInfo.setQueueFamilyIndex(queueInfo.presentFamilyIndex.value())
                .setQueueCount(1)
                .setPQueuePriorities(&priority);
            queueCreateInfos.push_back(queueCreateInfo);
        }
        devicecreateinfo.setQueueCreateInfos(queueCreateInfos)
            .setPEnabledExtensionNames(deviceExtensions);

        return physicalDevice.createDevice(devicecreateinfo);
    }

    void Context::QueryQueueFamilyIndexes()
    {
        auto deviceproperties = physicalDevice.getQueueFamilyProperties();
        for (int32_t i = 0; i < deviceproperties.size(); i++)
        {
            if (deviceproperties[i].queueFlags & vk::QueueFlagBits::eGraphics)
                queueInfo.graphicsFamilyIndex = i;

            if (physicalDevice.getSurfaceSupportKHR(i, surface))
                queueInfo.presentFamilyIndex = i;

            if (queueInfo.graphicsFamilyIndex.has_value() && queueInfo.presentFamilyIndex.has_value())
            {
                std::cout << "graphicsFamilyIndex:" << queueInfo.graphicsFamilyIndex.value() << std::endl;
                break;
            }
        }
    }

    void Context::InitSwapchain(int width, int height)
    {
        swapchain.reset(new Swapchain(width, height));
    }

    Context::Context(std::vector<const char *> extensions,
                     std::function<vk::SurfaceKHR(vk::Instance)> retsurface)
    {
        std::function<vk::SurfaceKHR(vk::Instance)> rsf = retsurface;
        instance = CreateInstance(extensions);
        surface = rsf(instance);
        if (surface == nullptr)
        {
            std::cout << "create surface failed" << std::endl;
            exit(2);
        }
        PickupPhysicalDevice();
        logicaldevice = CreateLogicalDevice();
        if (queueInfo.graphicsFamilyIndex.has_value() && queueInfo.presentFamilyIndex.has_value())
        {
            graphicQueue = logicaldevice.getQueue(queueInfo.graphicsFamilyIndex.value(), 0);
            presentQueue = logicaldevice.getQueue(queueInfo.presentFamilyIndex.value(), 0);
            std::cout << "graphicQueue:" << graphicQueue << std::endl;
        }
    }

    Context::~Context()
    {
        Context::GetInstance().swapchain.reset();
        instance.destroySurfaceKHR(surface);
        logicaldevice.destroy();
        instance.destroy();
    }
}