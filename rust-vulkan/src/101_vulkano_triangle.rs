use vulkano::{
    device::{Device, Queue},
    instance::{debug::DebugUtilsMessengerCallback, Instance, InstanceExtensions, Version},
    swapchain::{Surface, Swapchain, },
    image::{ImageUsage, view::ImageView}
    
};

use std::sync::Arc;

pub struct HelloTriangleApplication {
    instance: Arc<Instance>,
    debug_callback: Option<DebugUtilsMessengerCallback>,
    swapchain: Arc<Swapchain>,
    surface: Arc<Surface>,
    physical_device_index: usize,
    logical_device: Arc<Device>,
    graphics_queue: Arc<Queue>,
    present_queue: Arc<Queue>,
    swap_chain_images: Vec<Arc<imagevi>

}

fn main() {

    // create vulkano instance
}
