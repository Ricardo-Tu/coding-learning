#include "../toy2d/context.hpp"
#include "../toy2d/shader.hpp"
namespace toy2d
{
    std::unique_ptr<Context> Context::instance_ = nullptr;
    std::unique_ptr<Swapchain> Context::swapchain_ = nullptr;
    std::unique_ptr<RenderProcess> Context::renderprocess_ = nullptr;
    std::unique_ptr<render> Context::render_ = nullptr;

    void Context::Init(std::vector<const char *> extensions, std::function<vk::SurfaceKHR(vk::Instance)> retsurface)
    {
        instance_.reset(new Context(extensions, retsurface));
    }

    void Context::Quit()
    {
        std::cout << "context quit" << std::endl;
        // vk::Device &logicaldevice = Context::GetInstance().logicaldevice;
        std::cout << "1" << std::endl;
        // for (auto &fb : Context::GetInstance().swapchain->framebuffers)
        // Context::GetInstance().logicaldevice.destroyFramebuffer(fb);
        std::cout << "1" << std::endl;
        // for (auto &image : Context::GetInstance().swapchain->images)
        // Context::GetInstance().logicaldevice.destroyImage(image);
        std::cout << "2" << std::endl;
        // for (auto &view : Context::GetInstance().swapchain->imageviews)
        // ctx.logicaldevice.destroyImageView(view, nullptr);
        std::cout << "3" << std::endl;
        // Context::GetInstance().logicaldevice.destroySwapchainKHR(swapchain_->swapchain);
        // ctx.logicaldevice.destroyPipeline(ctx.renderprocess_->pipeline);
        // ctx.logicaldevice.destroyPipelineLayout(ctx.renderprocess_->pipelineLayout);
        // ctx.logicaldevice.destroyRenderPass(ctx.renderprocess_->renderPass);
        // renderprocess_.reset();
        std::cout << "5" << std::endl;
        Context::GetInstance().swapchain_.reset();
        instance_.reset();
        std::cout << "context quit leave" << std::endl;
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
        float priority = 1.0f;
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        vk::PhysicalDeviceFeatures deviceFeatures;
        vk::DeviceQueueCreateInfo queueCreateInfo;
        QueryQueueFamilyIndexes();
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
            .setPEnabledFeatures(&deviceFeatures)
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
        swapchain_.reset(new Swapchain(width, height));
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
        // std::cout << "1" << std::endl;
        // auto &logicaldevice = Context::GetInstance().logicaldevice;
        // logicaldevice.destroyPipeline(Context::GetInstance().renderprocess->pipeline);
        // logicaldevice.destroyPipelineLayout(Context::GetInstance().renderprocess->pipelineLayout);
        // logicaldevice.destroyRenderPass(Context::GetInstance().renderprocess->renderPass);

        // render_.release();
        // std::cout << "destroy swapchain" << std::endl;
        // std::cout << "destroy swapchain" << std::endl;
        // for (auto &view : swapchain->imageviews)
        //     logicaldevice.destroyImageView(view);
        // std::cout << "1" << std::endl;
        // for (auto &fb : swapchain->framebuffers)
        //     logicaldevice.destroyFramebuffer(fb);
        // std::cout << "2" << std::endl;
        // logicaldevice.destroySwapchainKHR(swapchain->swapchain);
        // std::cout << "destroy swapchain" << std::endl;

        // instance.destroySurfaceKHR(surface);
        logicaldevice.destroy();
        instance.destroy();
    }
}