#![allow(
    dead_code,
    unused_variables,
    clippy::too_many_arguments,
    clippy::unnecessary_wraps
)]

use anyhow::{anyhow, Result};
use log::*;
use pretty_env_logger::env_logger;
use std::collections::HashSet;
use std::ffi::CStr;
use std::os::raw::c_void;
use std::sync::Arc;
use vulkanalia::{
    loader::{LibloadingLoader, LIBRARY},
    prelude::v1_0::*,
    vk::ExtDebugUtilsExtension,
    window as vk_window, Version,
};
use winit::{
    event::{Event, WindowEvent},
    event_loop::{ControlFlow, EventLoop},
    window::{Window, WindowBuilder},
};

// 定义时候开启vulkan验证层, cfg!(debug_assertions)表示判断时候开启了调试断言, 如果开启了调试断言，VALIDATION_ENABLED为true，否则为false
const VALIDATION_ENABLED: bool = cfg!(debug_assertions);
// 定义vulkan验证层扩展名称
const VALIDATION_LAYER: vk::ExtensionName =
    vk::ExtensionName::from_bytes(b"VK_LAYER_KHRONOS_validation");
// 定义macos版本号
const PORTABILITY_MACOS_VERSION: Version = Version::new(1, 3, 216);

fn main() -> Result<()> {
    env_logger::init();
    // 创建一个事件循环对象，用来处理用户处理网络处理和GUI编程等
    let event_loop = EventLoop::new().unwrap();
    // 创建一个windowbuilder对象，builder对象一般用来存储一些配置信息，然后调用build函数，传入event_loop对象，绑定窗口对象和事件循环对象
    let window = Arc::new(WindowBuilder::new().build(&event_loop).unwrap());
    // 调用new函数创建一个软渲染context，将window窗口对象的克隆作为参数传入，原因为在context内部可能会需要用到窗口对象的所有权
    // let context = softbuffer::Context::new(window.clone()).unwrap();
    // 创建一个软渲染表面对象，也就是surface对象，传入context和window对象的克隆作为参数
    // let surface = softbuffer::Surface::new(&context, window.clone()).unwrap();

    // 创建vulkan实例对象，传入window对象的克隆作为参数
    let mut app = unsafe { App::create(&window)? };
    // 窗口销毁标志位，如果为true，表示窗口进入销毁流程，此时不再进行render操作
    let mut destroying = false;

    // 事件循环处理主函数，传入一个闭包，闭包内部处理事件循环的事件
    event_loop
        .run(move |event, elwt| {
            elwt.set_control_flow(ControlFlow::Poll);
            match event {
                // 处理窗口事件,RedrawRequested表示窗口需要重绘
                Event::WindowEvent {
                    window_id,
                    event: WindowEvent::RedrawRequested,
                } if window_id == window.id() && !destroying => {
                    unsafe { app.render(&window) }.unwrap();
                }
                // CloseRequested表示窗口关闭请求
                Event::WindowEvent {
                    window_id,
                    event: WindowEvent::CloseRequested,
                } if window_id == window.id() && !destroying => {
                    // 首先设置窗口销毁标志位为true，然后调用app.destroy()函数销毁app对象，最后调用elwt.exit()函数退出事件循环
                    destroying = true;
                    unsafe { app.destroy() };
                    elwt.exit();
                }
                // 其他事件，不做处理
                _ => {}
            }
        })
        .unwrap();
    Ok(())
}

#[derive(Clone, Debug)]
struct App {
    // vulkan入口点结构体
    entry: Entry,
    // a vulkan instance 实例
    instance: Instance,
    // 用来存储应用数据的结构体
    data: AppData,
}

impl App {
    unsafe fn create(window: &Window) -> Result<Self> {
        // 创建一个加载动态链接库的加载对象，参数为动态链接库的路径或者为动态链接库的名称，这里传入的为libvulkan.so.1
        let loader = LibloadingLoader::new(LIBRARY)?;
        // 将加载对象传入Entry::new函数，创建一个vulkan入口点对象，传入的loader对象用于加载vulkan api函数，map_err用于处理错误信息，其中的闭包将Restult错误转化后使用anyhow输出
        let entry = Entry::new(loader).map_err(|e| anyhow!("{}", e))?;
        // 创建一个AppData结构体对象
        let mut data = AppData::default();
        // 创建一个vulkan实例对象，传入window对象，vulkan入口点对象和自定义的AppData结构体对象作为参数
        let instance = create_instance(window, &entry, &mut data)?;
        Ok(Self {
            entry,
            instance,
            data,
        })
    }

    unsafe fn render(&mut self, window: &Window) -> Result<()> {
        Ok(())
    }

    unsafe fn destroy(&mut self) {
        if VALIDATION_ENABLED {
            self.instance
                .destroy_debug_utils_messenger_ext(self.data.messenger, None);
        }
        self.instance.destroy_instance(None);
    }
}

#[derive(Clone, Debug, Default)]
struct AppData {
    messenger: vk::DebugUtilsMessengerEXT,
}
unsafe fn create_instance(window: &Window, entry: &Entry, data: &mut AppData) -> Result<Instance> {
    // 创建一个vulkan应用信息结构体对象，用于存储应用的一些信息，比如应用名称，引擎名称，api版本等
    let application_info = vk::ApplicationInfo::builder()
        .application_name(b"rust vulkan validation\0")
        .application_version(vk::make_version(1, 0, 0))
        .engine_name(b"No Engine\0")
        .engine_version(vk::make_version(1, 0, 0))
        .api_version(vk::make_version(1, 0, 0));

    // 获取所有的vulkan层对象，然后将层对象的名称存储到一个HashSet集合中
    let available_layers = entry
        .enumerate_instance_layer_properties()?
        .iter()
        .map(|l| l.layer_name)
        .collect::<HashSet<_>>();

    // 判断时候开启了验证层，如果开启了验证层，但是验证层不在可用层中，返回错误信息
    if VALIDATION_ENABLED && !available_layers.contains(&VALIDATION_LAYER) {
        return Err(anyhow!("Validation layer requested, but not available"));
    }

    // 创建一个vulkan层名称的指针数组，如果开启了验证层，将验证层的名称存储到数组中
    let layers = if VALIDATION_ENABLED {
        vec![VALIDATION_LAYER.as_ptr()]
    } else {
        Vec::new()
    };

    // 获取所有的vulkan实例扩展对象，然后将扩展对象的名称存储到一个HashSet中
    let mut extensions = vk_window::get_required_instance_extensions(window)
        .iter()
        .map(|e| e.as_ptr())
        .collect::<Vec<_>>();

    // 判断时候开启了macos平台的扩展，如果开启了macos平台扩展，将macos平台扩展的名称存储到数组中，并设置flags标志位
    let flags = if cfg!(target_os = "macos") && entry.version()? >= PORTABILITY_MACOS_VERSION {
        info!("Enabling extensions for macOS portability");
        // 如果是在macos平台，则开启这些macos平台所需的扩展
        extensions.push(
            vk::KHR_GET_PHYSICAL_DEVICE_PROPERTIES2_EXTENSION
                .name
                .as_ptr(),
        );
        extensions.push(vk::KHR_PORTABILITY_ENUMERATION_EXTENSION.name.as_ptr());
        // 并设置标志位为macos平台可移植扩展平台
        vk::InstanceCreateFlags::ENUMERATE_PORTABILITY_KHR
    } else {
        // 如果不再macos平台且版本不大于该版本，表示是在linux或者window平台，这设置标志位为空
        vk::InstanceCreateFlags::empty()
    };

    // 如果开启了验证层，将验证层的扩展名称存储到数组中
    if VALIDATION_ENABLED {
        extensions.push(vk::EXT_DEBUG_UTILS_EXTENSION.name.as_ptr());
    }

    // create instance
    // 创建一个vulkan实例创建信息结构体对象，在其中设置应用程序所需要的一些信息，比如需要使用vulkan的哪些层，哪些扩展，应用程序的信息等
    let mut info = vk::InstanceCreateInfo::builder()
        .application_info(&application_info)
        .enabled_layer_names(&layers)
        .enabled_extension_names(&extensions)
        .flags(flags);

    // 如果开启了验证层，设置验证层的回调函数,创建调试信息结构体对象
    let mut debug_info = vk::DebugUtilsMessengerCreateInfoEXT::builder()
        .message_severity(vk::DebugUtilsMessageSeverityFlagsEXT::all())
        .message_type(vk::DebugUtilsMessageTypeFlagsEXT::all())
        .user_callback(Some(debug_callback));

    // 如果开启了验证层，将验证层的信息存储到info结构体中
    if VALIDATION_ENABLED {
        info = info.push_next(&mut debug_info);
    }

    // 创建一个vulkan实例对象，传入vulkan实例创建信息结构体对象，然后返回vulkan实例对象
    let instance = entry.create_instance(&info, None)?;

    // 如果开启了验证层，创建一个验证层调试信息对象，调试信息结构体对象作为参数
    if VALIDATION_ENABLED {
        data.messenger = instance.create_debug_utils_messenger_ext(&debug_info, None)?;
    }

    Ok(instance)
}

// 定义一个验证层的回调函数，用于输出验证层的信息，这个函数是在创建debug_info结构体的时候传入的
extern "system" fn debug_callback(
    severity: vk::DebugUtilsMessageSeverityFlagsEXT,
    type_: vk::DebugUtilsMessageTypeFlagsEXT,
    data: *const vk::DebugUtilsMessengerCallbackDataEXT,
    _: *mut c_void,
) -> vk::Bool32 {
    let data = unsafe { *data };
    let message = unsafe { CStr::from_ptr(data.message) }.to_string_lossy();

    if severity >= vk::DebugUtilsMessageSeverityFlagsEXT::ERROR {
        error!("({:?}) {}", type_, message);
    } else if severity >= vk::DebugUtilsMessageSeverityFlagsEXT::WARNING {
        warn!("({:?}) {}", type_, message);
    } else if severity >= vk::DebugUtilsMessageSeverityFlagsEXT::INFO {
        debug!("({:?}) {}", type_, message);
    } else {
        trace!("({:?}) {}", type_, message);
    }

    vk::FALSE
}
