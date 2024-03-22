#![allow(
    unused_variables,
    dead_code,
    clippy::too_many_arguments,
    clippy::unnecessary_wraps
)]

use anyhow::{anyhow, Result};
use log::*;
use pretty_env_logger::env_logger;
use std::{collections::HashSet, sync::Arc};
use vulkanalia::{
    loader::{LibloadingLoader, LIBRARY},
    prelude::v1_0::*,
    window as vk_window, Version,
};
use winit::{
    event::{Event, WindowEvent},
    event_loop::{ControlFlow, EventLoop},
    window::{Window, WindowBuilder},
};

const VALIDATION_ENABLED: bool = cfg!(debug_assertions);
const VALIDATION_LAYER: vk::ExtensionName = vk::ExtensionName::from_bytes(b"VK_LAYER_KHRONOS_validation");
const PORTABILITY_MACOS_VERSION: Version = Version::new(1, 3, 216);

fn main() -> Result<()> {
    env_logger::init();
    let event_loop = EventLoop::new().unwrap();
    let window = Arc::new(WindowBuilder::new().build(&event_loop).unwrap());
    let destroying: bool = false;
    let app = unsafe { App::create(&window)? };

    event_loop
        .run(move |event, elwt| {
            elwt.set_control_flow(ControlFlow::Poll);
            match event {
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
                    destroying = false;
                    unsafe { app.destroy() };
                    elwt.exit();
                }
                _ => {}
            }
        })
        .unwrap();
    return Ok(());
}

#[derive(Clone, Debug)]
struct App {
    entry: Entry,
    instance: Instance,
    data: AppData,
}
impl App {
    unsafe fn create(window: &Window) -> Result<Self> {
        let loader = LibloadingLoader::new(LIBRARY)?;
        let entry = Entry::new(loader).map_err(|e| anyhow!("{}", e))?;
        let mut data = AppData::default();
        let instance = create_instance(&window, &entry, &mut data)?;
        return Ok(Self {
            entry,
            instance,
            data,
        });
    }
    unsafe fn render(&self, window: &Window) -> Result<()> {
        return Ok(());
    }
    unsafe fn destroy(&self) {
        self.instance.destroy_instance(None);
    }
}

#[derive(Clone, Debug, Default)]
struct AppData {
    messenger: vk::DebugUtilsMessengerEXT,
}

unsafe fn create_instance(window: &Window, entry: &Entry, data: &mut AppData) -> Result<Instance> {
    let application_info = vk::ApplicationInfo::builder()
        .application_name(b"rust vulkan app\0")
        .application_version(vk::make_version(1, 0, 0))
        .engine_name(b"No Engine")
        .engine_version(vk::make_version(1, 0, 0))
        .api_version(vk::make_version(1, 0, 0));

    let avaiable_layers = entry.enumerate_instance_layer_properties()?
        .iter()
        .map(|l| l.layer_name)
        .collect::<HashSet<_>>();

    
}
