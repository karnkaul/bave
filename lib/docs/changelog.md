# Changelog

## v0.4

### Features

- Added bave::Gamepad support, queried through bave::App::get_gamepad() and bave::App::get_gamepads().
  - Not yet supported on Android.
- Added bave::KeyState and bave::App::get_key_state().
- Added bave::EnumFlags and bave::EnumArray.
- Added bave::ParticleSystem.
- Added bave::LineRectShape (specialization of bave::Shape).

### Changes

- Unified all bit flags to be bave::EnumFlags.
- Renamed `particle_emitter.?pp` to `particle_system.?pp`.
- Removed primitive topology from bave::Shader. It is now stored in bave::Geometry, the shader obtains it from passed bave::RenderPrimitive during bave::Shader::draw.
