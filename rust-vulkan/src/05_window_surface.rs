use anyhow::{anyhow, Result};
use thiserror::Error;
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

const VALIDATION_ENABLED = cfg!(debug_assertions);
const VALIDATION_LAYER = vk::ExtensionName::from_bytes(b"VK_LAYER_KHRONOS_validation");
const PORTABILITY_MACOS_VERSION = 

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
    fn create(window: &Window) -> Result<Self> {}
}

#[derive(Debug, Clone, Default)]
struct AppData {
    messenger: vk::DebugUtilsMessengerEXT,
    physical: vk::PhysicalDevice,
    graphics_queue: vk::Queue,
}

#[derive(Debug, Error)]
#[error("{0}")]
pub struct SuitabilityError(pub &'static str);
