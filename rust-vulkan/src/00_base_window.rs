#![allow(
    dead_code,
    unused_variables,
    clippy::too_many_arguments,
    clippy::unnecessary_wraps
)]
use anyhow::{Context, Result};
use softbuffer;
use std::num::NonZeroU32;
use std::sync::Arc;
use winit::{
    event::{Event, WindowEvent},
    event_loop::{ControlFlow, EventLoop},
    window::{Window, WindowBuilder},
};

fn main() -> Result<()> {
    let event_loop = EventLoop::new().unwrap();
    let window = Arc::new(WindowBuilder::new().build(&event_loop).unwrap());
    let context = softbuffer::Context::new(window.clone()).unwrap();
    let mut surface = softbuffer::Surface::new(&context, window.clone()).unwrap();

    // 初始化应用程序

    let mut app = unsafe { App::create(&window)? };
    let mut destroying = false;

    event_loop
        .run(move |event, elwt| {
            elwt.set_control_flow(ControlFlow::Poll);

            match event {
                Event::WindowEvent {
                    window_id,
                    event: WindowEvent::RedrawRequested,
                } if window_id == window.id() => {
                    let (width, height) = {
                        let size = window.inner_size();
                        (size.width, size.height)
                    };
                    surface
                        .resize(
                            NonZeroU32::new(width).unwrap(),
                            NonZeroU32::new(height).unwrap(),
                        )
                        .unwrap();

                    let mut buffer = surface.buffer_mut().unwrap();
                    for index in 0..(width * height) {
                        let y = index / width;
                        let x = index % width;
                        let red = x % 255;
                        let green = y % 255;
                        let blue = (x * y) % 255;

                        // buffer[index as usize] = blue | (green << 8) | (red << 16);
                        buffer[index as usize] =
                            0b11111111 << 8;//| (0b11111111 << 8) | (0b11111111 << 16);
                    }
                    buffer.present().unwrap();
                }
                Event::WindowEvent {
                    event: WindowEvent::CloseRequested,
                    window_id,
                } if window_id == window.id() => {
                    elwt.exit();
                }
                _ => {}
            }
        })
        .unwrap();
    Ok(())
}

/// 我们的 Vulkan 应用程序
#[derive(Clone, Debug)]
struct App {}

impl App {
    /// 创建 Vulkan App
    unsafe fn create(window: &Window) -> Result<Self> {
        Ok(Self {})
    }

    /// 渲染帧
    unsafe fn render(&mut self, window: &Window) -> Result<()> {
        Ok(())
    }

    /// 销毁 Vulkan App
    unsafe fn destroy(&mut self) {}
}

/// 我们的 Vulkan 应用程序所使用的 Vulkan 句柄和相关属性
#[derive(Clone, Debug, Default)]
struct AppData {}
