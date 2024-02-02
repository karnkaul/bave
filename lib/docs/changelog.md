# Changelog

## v0.4

### Features

- Added bave::Gamepad support, queried through bave::App::get_gamepad() and bave::App::get_gamepads().
  - Not yet supported on Android.
- Added bave::KeyState and bave::App::get_key_state().
- Added bave::EnumFlags and bave::EnumArray.
- Added bave::ParticleSystem.

### Changes

- Unified all bit flags to be bave::EnumFlags.
- Renamed `particle_emitter.?pp` to `particle_system.?pp`.
