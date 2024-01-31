# Changelog

## v0.4

### Features

- Added bave::Gamepad support, queried through bave::App::get_gamepad() and bave::App::get_gamepads().
  - Not yet supported on Android.
- Added bave::KeyState and bave::App::get_key_state().

### Changes

- Replaced `bave::mod::*` with bave::Mod.
- Refactored bave::ParticleEmitter::Modifier into an enum.
