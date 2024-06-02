use anyhow::Result;
use sdl2::event::Event;
use sdl2::image::{self, LoadTexture};
use sdl2::keyboard::Keycode;
use sdl2::mixer::{self, AUDIO_S16LSB, DEFAULT_CHANNELS};
use sdl2::rect::Rect;
use sdl2::render::TextureCreator;
use sdl2::video::WindowContext;
use std::path::Path;
use std::time::{Duration, Instant};

const FRAMES: f64 = 25.0;

pub fn main() -> Result<()> {
    let sdl_context = sdl2::init().unwrap();
    let video_subsystem = sdl_context.video().unwrap();

    let window = video_subsystem
        .window("rust-sdl2 demo", 800, 600)
        .resizable()
        .position_centered()
        .build()
        .unwrap();

    mixer::open_audio(44100, AUDIO_S16LSB, DEFAULT_CHANNELS, 1024).unwrap();
    mixer::init(mixer::InitFlag::all()).unwrap();
    mixer::allocate_channels(4);
    let music = mixer::Music::from_file("./resources/backgroundmusic/bgm.mp3").unwrap();
    music.play(1).unwrap();

    let mut canvas = window
        .into_canvas()
        .build()
        .map_err(|e| e.to_string())
        .unwrap();
    let texture_creator: TextureCreator<WindowContext> = canvas.texture_creator();

    image::init(image::InitFlag::PNG).unwrap();
    let mut i = 0;
    let target_frame_duration = Duration::from_secs_f64(1.0 / FRAMES as f64);

    // canvas.set_draw_color(Color::RGB(0, 255, 255));
    // canvas.clear();
    let mut event_pump = sdl_context.event_pump().unwrap();
    'running: loop {
        let start = Instant::now();
        i = (i + 1) % 490;
        if i == 0 {
            i = 1;
        }
        for event in event_pump.poll_iter() {
            match event {
                Event::Quit { .. }
                | Event::KeyDown {
                    keycode: Some(Keycode::Escape),
                    ..
                } => {
                    break 'running;
                }
                _ => {}
            }
        }
        // 加载图片
        let file_path = format!("./resources/images/{}.png", i);
        if !Path::new(&file_path).exists() {
            println!("Image not found: {}", file_path);
            continue;
        }

        let texture = texture_creator.load_texture(file_path).unwrap();

        // 清除画布
        canvas.clear();

        // 获取窗口大小并调整图片显示位置
        let (window_width, window_height) = canvas.output_size().unwrap();
        let target_rect = Rect::new(0, 0, window_width, window_height);

        // 显示图片
        canvas.copy(&texture, None, Some(target_rect)).unwrap();
        canvas.present();
        let duration_time = start.elapsed();
        let sleep_duration = if duration_time < target_frame_duration {
            target_frame_duration - duration_time
        } else {
            Duration::from_secs(0)
        };
        std::thread::sleep(sleep_duration);
        // ::std::thread::sleep(Duration::new(0, 1_000_000_000u32 / 60));
    }
    Ok(())
}
