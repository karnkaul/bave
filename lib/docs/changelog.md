# Changelog

## v0.4

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
