# Changelog

## v0.5

### v0.5.4

- Desktop: if aspect ratio is locked, set the window size right after window creation (to enforce the locked aspect ratio). 
- Split instanced rendering from bave::Drawable into bave::Instanced.
- Renamed `CombinedImageSampler` to bave::SamplerImage.
- Split bave::IDrawable, bave::ParticleEmitter, bave::SamplerImage into their own headers.

### v0.5.3

- bave::SpriteAnim only overwrites texture[0] if it has an atlas.
- Added bave::Loader::load_particle_emitter().

### v0.5.2

- Added bave::Persistor::exists() to check if a persisted resource exists before trying to read from it.
- Write functions in bave::file create parent directories as needed.

### v0.5.1

- Added bave::Persistor for manipulating files in platform-specific persistent storage.

### v0.5.0

- bave::DesktopApp::CreateInfo now allows customizing the bave::IDataLoader in use instead of storing the asset patterns to search for.
- Added bave::DataLoaderBuilder to search for asset zips / directories and build a corresponding concrete bave::IDataLoader for desktop.

## v0.4

### v0.4.10

- Added ZIP VFS (virtual filesystem) in bave::zip. ZIP archives can be mounted/unmounted and read from (io/zip_io.hpp).

### v0.4.9

- Fixed overflow bug in key events.

### v0.4.8

- Added JSON serialization for bave::ParticleConfig.

### v0.4.7

- Linearized render pass clear colour.
- Added bave::ImInputTextMultiLine.
- Added bave::Driver::should_close(), called on desktop when the user attempts to close the window.

### v0.4.6

- Fixed MoltenVk flow, targets now run on MacOS with VulkanSDK installed.
- Updated bave::ParticleEmitter to spawn particles at least once.

### v0.4.5

- Fixed Vulkan RenderPass synchronization and Swapchain recreation bugs.
- Added support for parenting transforms of each bave::RenderInstance in a bave::Drawable.
- Fixed positioning of bave::ParticleEmitter. Annotated that its bave::Transform will be ignored.

### v0.4.4

- Added bave::App::update_gamepad_mappings() for custom GLFW gamepad mappings.
- Inverted GLFW gamepad left and right axis Y values.
- Added `dead_zone` argument to bave::Gamepad::get_axis().

### v0.4.3

- Added bave::to_uv_rect() and bave::RenderView::to_n_scissor.
- Added MSAA (multi sampled anti aliasing) support.
- Fixed potential overflow in the general constructor of bave::FixedString.
- Made bave::DesktopApp and bave::AndroidApp inherit privately from bave::App. This tightens the API available to either platform's main: they can only call bave::App::set_bootloader() and bave::App::run().

### v0.4.2

- Fixed app shutdown/restart flow on Android.
- Made bave::Texture polymorphic. (Marked its destructor as `virtual`.)

### v0.4.1

- Fixed bave::Rgba::to_hex_str(). Previously it would skip leading zeroes for each byte.
- Add some convenience functions on bave::EnumFlags.
- Linearize vertex RGBA in bave::Geometry.
- Preload font atlas for default bave::TextHeight.

### v0.4.0

#### Features

- Added bave::Gamepad support, queried through bave::App::get_gamepad() and bave::App::get_gamepads().
  - Not yet supported on Android.
- Added bave::KeyState and bave::App::get_key_state().
- Added bave::EnumFlags and bave::EnumArray.
- Added bave::ParticleSystem.
- Added bave::LineRectShape (specialization of bave::Shape).

#### Changes

- Unified all bit flags to be bave::EnumFlags.
- Renamed `particle_emitter.?pp` to `particle_system.?pp`.
- Removed primitive topology from bave::Shader. It is now stored in bave::Geometry, the shader obtains it from passed bave::RenderPrimitive during bave::Shader::draw.
- Refactor vertices and indices from bave::Geometry into bave::VertexArray.
