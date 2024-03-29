use anyhow::{anyhow, Result};
use log::*;
use pretty_env_logger::env_logger;
use std::{collections::HashSet, os::raw::c_void, sync::Arc, ffi::CStr};
use thiserror::Error;
use vulkanalia::{
    loader::{LibloadingLoader, LIBRARY},
    prelude::v1_0::*,
    vk::{ExtDebugUtilsExtension, Flags64},
    window as vk_window, Version,
};
use winit::{
    event::{Event, WindowEvent},
    event_loop::{ControlFlow, EventLoop},
    window::{Window, WindowBuilder},
};

const VALIDATION_ENABLED: bool = cfg!(debug_assertions);
const VALIDATION_LAYER: vk::ExtensionName =
    vk::ExtensionName::from_bytes(b"VK_LAYER_KHRONOS_validation");
const PORTABILITY_MACOS_VERSION: Version = Version::new(1, 3, 216);

fn main() -> Result<()> {
    env_logger::init();
    // 创建窗口循环事件
    let event_loop = EventLoop::new().unwrap();
    // 创建窗口并和窗口事件循环绑定
    let window = Arc::new(WindowBuilder::new().build(&event_loop).unwrap());
    // 创建一个标志位，用于标记是否正在销毁
    let mut destroying = false;
    // 创建App实例
    let app = unsafe { App::create(&window)? };
    event_loop
        .run(move |event, elwt| match event {
            Event::WindowEvent {
                window_id,
                event: WindowEvent::RedrawRequested,
            } if window_id == window.id() && !destroying => {
                unsafe { app.render(&window) }.unwrap();
            }
            Event::WindowEvent {
                window_id,
                event: WindowEvent::CloseRequested,
            } if window_id == window.id() && !destroying => {
                destroying = true;
                unsafe { app.destroy() };
                elwt.exit();
            }
            _ => {}
        })
        .unwrap();
    return Ok(());
}

#[derive(Clone, Debug)]
struct App {
    entry: Entry,
    instance: Instance,
    data: AppData,
    device: Device,
}
impl App {
    unsafe fn create(window: &Window) -> Result<Self> {
        let loader = LibloadingLoader::new(LIBRARY)?;
        let entry = Entry::new(loader).map_err(|e| anyhow!("{}", e))?;
        let mut data = AppData::default();
        let instance = create_instance(&entry, window, &mut data)?;
        pick_physical_device(&instance, &mut data)?;
        let device = create_logical_device(&instance, &data)?;
        return Ok(Self {
            entry,
            instance,
            data,
            device,
        });
    }

    unsafe fn render(&self, window: &Window) -> Result<()> {
        return Ok(());
    }
    unsafe fn destroy(&self) {
        if VALIDATION_ENABLED {
            self.instance
                .destroy_debug_utils_messenger_ext(self.data.messenger, None);
        }
        self.device.destroy_device(None);
        self.instance.destroy_instance(None);
    }
}

#[derive(Debug, Clone, Default)]
struct AppData {
    messenger: vk::DebugUtilsMessengerEXT,
    physical: vk::PhysicalDevice,
    graphics_queue: vk::Queue,
}

unsafe fn create_instance(entry: &Entry, window: &Window, data: &mut AppData) -> Result<Instance> {
    let application_info = vk::ApplicationInfo::builder()
        .application_name(b"Rust Vulkan Tutorial")
        .application_version(vk::make_version(1, 0, 0))
        .engine_name(b"No Engine!")
        .engine_version(vk::make_version(1, 0, 0))
        .api_version(vk::make_version(1, 0, 0));

    let avaiable_layers = entry
        .enumerate_instance_layer_properties()?
        .iter()
        .map(|l| l.layer_name)
        .collect::<HashSet<_>>();

    if VALIDATION_ENABLED && !avaiable_layers.contains(&VALIDATION_LAYER) {
        return Err(anyhow!("Validation layer requested, but not available"));
    }
    let mut layers = vec![];

    if VALIDATION_ENABLED {
        layers.push(VALIDATION_LAYER.as_ptr());
    }

    let mut extensions = vk_window::get_required_instance_extensions(window)
        .iter()
        .map(|e| e.as_ptr())
        .collect::<Vec<_>>();
    // 判断是不是macos
    let flags = if cfg!(target_os = "macos") && entry.version()? > PORTABILITY_MACOS_VERSION {
        info!("Enabling macos portability extensions");
        extensions.push(
            vk::KHR_GET_PHYSICAL_DEVICE_PROPERTIES2_EXTENSION
                .name
                .as_ptr(),
        );
        extensions.push(vk::KHR_PORTABILITY_ENUMERATION_EXTENSION.name.as_ptr());
        vk::InstanceCreateFlags::ENUMERATE_PORTABILITY_KHR
    } else {
        vk::InstanceCreateFlags::empty()
    };

    if VALIDATION_ENABLED {
        extensions.push(vk::EXT_DEBUG_UTILS_EXTENSION.name.as_ptr());
    }

    let mut create_info = vk::InstanceCreateInfo::builder()
        .application_info(&application_info)
        .enabled_layer_names(&layers)
        .enabled_extension_names(&extensions)
        .flags(flags);
    let mut debug_info = vk::DebugUtilsMessengerCreateInfoEXT::builder()
        .message_severity(vk::DebugUtilsMessageSeverityFlagsEXT::all())
        .message_type(vk::DebugUtilsMessageTypeFlagsEXT::all())
        .user_callback(Some(debug_callback));
    let instance = entry.create_instance(&create_info, None)?;
    if VALIDATION_ENABLED {
        let messenger = instance.create_debug_utils_messenger_ext(&debug_info, None)?;
        data.messenger = messenger;
    }
    return Ok(instance);
}
unsafe extern "system" fn debug_callback(
    severity: vk::DebugUtilsMessageSeverityFlagsEXT,
    type_: vk::DebugUtilsMessageTypeFlagsEXT,
    data: *const vk::DebugUtilsMessengerCallbackDataEXT,
    _: *mut c_void,
) -> vk::Bool32 {
    let data = unsafe { *data };
    let message = unsafe { CStr::from_ptr(data.message) }.to_string_lossy();
    if severity >= vk::DebugUtilsMessageSeverityFlagsEXT::ERROR {
        error!("{:?} {}", type_, message);
    } else if severity >= vk::DebugUtilsMessageSeverityFlagsEXT::WARNING {
        warn!("{:?} {}", type_, message);
    } else if severity >= vk::DebugUtilsMessageSeverityFlagsEXT::INFO {
        info!("{:?} {}", type_, message);
    } else if severity >= vk::DebugUtilsMessageSeverityFlagsEXT::VERBOSE {
        debug!("{:?} {}", type_, message);
    }

    return vk::FALSE;
}
#[derive(Debug, Error)]
#[error("{0}")]
pub struct SuitabilityError(pub &'static str);
