use vulkano::buffer::{Buffer, BufferCreateInfo, BufferUsage, Subbuffer};
use vulkano::command_buffer::allocator::StandardCommandBufferAllocator;
use vulkano::command_buffer::{
    AutoCommandBufferBuilder, ClearColorImageInfo, CommandBufferUsage, CopyImageToBufferInfo,
};
use vulkano::device::physical::PhysicalDeviceType;
use vulkano::device::{Device, DeviceExtensions, Features, QueueCreateInfo, QueueFlags};
use vulkano::format::ClearColorValue;
use vulkano::instance::{Instance, InstanceCreateInfo};
use vulkano::memory::allocator::{AllocationCreateInfo, DeviceLayout, MemoryTypeFilter};
use vulkano::pipeline::compute::ComputePipelineCreateInfo;

use vulkano::pipeline::layout::{PipelineDescriptorSetLayoutCreateInfo, PipelineLayout};
use vulkano::pipeline::{ComputePipeline, PipelineShaderStageCreateInfo};
// use vulkano::sync;
// use vulkano::pipeline::{ComputePipeline, PipelineBindPoint, PipelineShaderStageCreateInfo};
// use vulkano::pipeline::{compute::ComputePipelineCreateInfo, Pipeline};
// use vulkano::descriptor_set::{PersistentDescriptorSet, WriteDescriptorSet};
// use vulkano::image::view::{ImageView, ImageViewCreateInfo};
use vulkano::sync::GpuFuture;

use std::sync::Arc;
use std::time::Duration;
use vulkano::descriptor_set::allocator::StandardDescriptorSetAllocator;
use vulkano::image::{Image, ImageUsage};

fn main() {
    let lib = vulkano::VulkanLibrary::new().unwrap();
    let api_version = lib.api_version();
    println!("lib api_version: {:?}.", api_version);
    let create_info = InstanceCreateInfo::default();
    let instance: Arc<Instance> = Instance::new(lib, create_info).unwrap();
    // let physical = PhysicalDevice::enumerate(&instance).next().unwrap();

    let ph = instance
        .enumerate_physical_devices()
        .unwrap();

    println!("num: {:?}", ph.len());
    
    for phy in ph {
        println!("physical: {} {:?}", phy.properties().device_name, phy.properties().device_type);
    }

    let physical = instance
        .enumerate_physical_devices()
        .unwrap()
        .find_map(|phy| {
            println!("physical: {} {:?}", phy.properties().device_name, phy.properties().device_type);
            if phy.properties().device_type == PhysicalDeviceType::DiscreteGpu {
                Some(phy)
            } else {
                None
            }
        })
        .unwrap();

    println!(
        "physical device name: {}.",
        physical.properties().device_name
    );
    println!(
        "Format: {:?}, {:#?}.",
        vulkano::format::Format::A1R5G5B5_UNORM_PACK16,
        physical
            .format_properties(vulkano::format::Format::A1R5G5B5_UNORM_PACK16)
            .unwrap()
    );
    println!(
        "Format: {:?}, {:#?}.",
        vulkano::format::Format::R8G8B8A8_UNORM,
        physical
            .format_properties(vulkano::format::Format::R8G8B8A8_UNORM)
            .unwrap()
    );

    // println!("physical: {:#?}", physical);
    // println!("features: {:#?}.", physical.supported_features());
    let (queue_index, _queue_family) = physical
        .queue_family_properties()
        .iter()
        .enumerate()
        .find(|(idx, q)| {
            println!("queue {} iter: {:?}", idx, q);
            q.queue_flags.intersects(QueueFlags::COMPUTE)
        })
        .unwrap();

    let create_info = vulkano::device::DeviceCreateInfo {
        queue_create_infos: vec![QueueCreateInfo {
            queue_family_index: queue_index as u32,
            queues: vec![0.5],
            ..Default::default()
        }],
        enabled_extensions: DeviceExtensions {
            khr_storage_buffer_storage_class: true,
            khr_synchronization2: physical.supported_extensions().khr_synchronization2,
            ..DeviceExtensions::empty()
        },
        // enabled_features: physical.supported_features().clone(),
        enabled_features: Features::empty(),
        // physical_devices: vec![physical.clone()].into(),
        ..Default::default()
    };

    let (device, mut queues) = Device::new(physical, create_info).unwrap();
    // let (device, mut queues) = Device::new(
    //     physical,
    //     physical.supported_features(),
    //     &DeviceExtensions {
    //         khr_storage_buffer_storage_class: true,
    //         ..DeviceExtensions::none()
    //     },
    //     [(queue_family, 0.5)].iter().cloned(),
    // )
    // .unwrap();
    let queue = queues.next().unwrap();

    let memory_allocator =
        Arc::new(vulkano::memory::allocator::StandardMemoryAllocator::new_default(device.clone()));
    let descriptor_set_allocator = Arc::new(StandardDescriptorSetAllocator::new(
        device.clone(),
        Default::default(),
    ));
    let command_buffer_allocator = Arc::new(StandardCommandBufferAllocator::new(
        device.clone(),
        Default::default(),
    ));

    let shader = cs::load(device.clone()).unwrap();
    let entry_point = shader.entry_point("main").unwrap();

    let shader_stage = PipelineShaderStageCreateInfo::new(entry_point);
    let pipeline_layout = PipelineLayout::new(
        device.clone(),
        PipelineDescriptorSetLayoutCreateInfo::from_stages([&shader_stage])
            .into_pipeline_layout_create_info(device.clone())
            .unwrap(),
    )
    .unwrap();
    let pipeline = ComputePipelineCreateInfo::stage_layout(shader_stage, pipeline_layout.clone());
    let pipeline = ComputePipeline::new(device.clone(), None, pipeline).unwrap();

    println!("Create pipeline success: {:#?}.", pipeline);
    // let pipeline = Arc::new(ComputePipeline::new(device.clone(), None, &shader.main_entry_point(),).unwrap(),);

    let extent = [128u32, 128, 1];
    let image = vulkano::image::ImageCreateInfo {
        extent,
        format: vulkano::format::Format::A1R5G5B5_UNORM_PACK16,
        tiling: vulkano::image::ImageTiling::Optimal,
        usage: ImageUsage::TRANSFER_SRC | ImageUsage::TRANSFER_DST,
        ..Default::default()
    };

    let image = Image::new(
        memory_allocator.clone(),
        image,
        AllocationCreateInfo::default(),
    )
    .unwrap();

    // let image_view =
    //     ImageView::new(image.clone(), ImageViewCreateInfo::from_image(&image)).unwrap();
    // let layout = &pipeline.layout().set_layouts()[0];

    // let descriptor_set = PersistentDescriptorSet::new(
    //     &descriptor_set_allocator,
    //     layout.clone(),
    //     [WriteDescriptorSet::image_view(0, image_view.clone())],
    //     [],
    // )
    // .unwrap();

    // let mut image_barrier = vulkano::sync::ImageMemoryBarrier {
    //     old_layout: ImageLayout::Undefined,
    //     new_layout: ImageLayout::General,
    //     ..vulkano::sync::ImageMemoryBarrier::image(image.clone())
    // };

    let mut command_buffer_builder = AutoCommandBufferBuilder::primary(
        &command_buffer_allocator,
        queue_index as u32,
        CommandBufferUsage::MultipleSubmit,
    )
    .unwrap();

    // command_buffer_builder
    //     .bind_pipeline_compute(pipeline)
    //     .unwrap()
    //     .bind_descriptor_sets(
    //         PipelineBindPoint::Compute,
    //         pipeline_layout.clone(),
    //         0,
    //         descriptor_set,
    //     )
    //     .unwrap()
    //     .push_constants(pipeline_layout, 0, [0.5f32, 0.0, 0.0, 1.0])
    //     .unwrap()
    //     .dispatch([extent[0] / 16, extent[1] / 16, 1])
    //     .unwrap();

    let alpha: f32 = unsafe { *(&0x3F80_0000u32 as *const u32 as *const f32) };

    command_buffer_builder
        .clear_color_image(ClearColorImageInfo {
            image_layout: vulkano::image::ImageLayout::General,
            clear_value: ClearColorValue::Float([0.3f32, 0.0, 0.0, alpha]),
            ..ClearColorImageInfo::image(image.clone())
        })
        .unwrap();

    let command_buffer = command_buffer_builder.build().unwrap();

    let gpu_future = vulkano::sync::now(device.clone())
        .then_execute(queue.clone(), command_buffer)
        .unwrap()
        .then_signal_fence_and_flush()
        .unwrap();

    gpu_future.wait(Some(Duration::from_secs(10))).unwrap();
    println!("Compute shader run finished.");

    let mut command_buffer_builder = AutoCommandBufferBuilder::primary(
        &command_buffer_allocator,
        queue_index as u32,
        CommandBufferUsage::OneTimeSubmit,
    )
    .unwrap();

    let buffer_create_info = BufferCreateInfo {
        size: 0u64,
        usage: BufferUsage::STORAGE_TEXEL_BUFFER | BufferUsage::TRANSFER_DST,
        ..Default::default()
    };

    let pixel_buffer = Subbuffer::new(
        Buffer::new(
            memory_allocator,
            buffer_create_info,
            AllocationCreateInfo {
                memory_type_filter: MemoryTypeFilter::HOST_RANDOM_ACCESS,
                ..Default::default()
            },
            DeviceLayout::from_size_alignment((extent[0] * extent[1] * 2) as u64, 16).unwrap(),
        )
        .unwrap(),
    );

    command_buffer_builder
        .copy_image_to_buffer(CopyImageToBufferInfo::image_buffer(
            image.clone(),
            pixel_buffer.clone(),
        ))
        .unwrap();

    let command_buffer = command_buffer_builder.build().unwrap();

    let gpu_future = vulkano::sync::now(device)
        .then_execute(queue, command_buffer)
        .unwrap()
        .then_signal_fence_and_flush()
        .unwrap();

    gpu_future.wait(Some(Duration::from_secs(10))).unwrap();

    let pixel_slice = pixel_buffer.read().unwrap();
    println!(
        "pixel buffer_data: 0x{:x?}",
        bytemuck::cast_slice::<u8, u16>(&pixel_slice[..])
    );
    println!("alpha = {}, bin = 0x{:x}.", alpha, unsafe {
        *(&alpha as *const f32 as *const u32)
    });
    println!("Success");
}

mod cs {
    vulkano_shaders::shader! {
        ty: "compute",
        src: r#"
        #version 450
        layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
        layout(set = 0, binding = 0, rgba8) writeonly uniform image2D dst_image;
        layout(push_constant) uniform ClearValue {
            vec4 clear_value;
        } info;

        void main() {
            uvec2 dst_pos = gl_GlobalInvocationID.xy;
            uvec2 dst_size = uvec2(imageSize(dst_image));

            if ((dst_pos.x < dst_size.x) && (dst_pos.y < dst_size.y)) {
                imageStore(dst_image, ivec2(dst_pos), info.clear_value);
            }
        }
        "#,
    }
}
