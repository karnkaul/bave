# bave

**BAsic Vulkan Engine**

[![Build Status](https://github.com/karnkaul/bave/actions/workflows/ci.yml/badge.svg)](https://github.com/karnkaul/bave/actions/workflows/ci.yml)

`bave` is a simple 2D engine written in C++20 and Vulkan.

## Demo

https://github.com/karnkaul/bave/assets/16272243/86539229-0640-42c5-baac-ca74eb382158

## Features

- [x] Desktop / Android support.
- [x] Efficient event queue.
- [x] Double buffered render resources.
- [x] Meshes and textures.
- [x] Shaders, optional custom descriptor sets (but fixed layout).
- [x] `Drawable` API.
- [x] Fonts and text.
- [x] Multi touch support.
- [x] Audio playback.
- [x] Streaming audio (music).

## Requirements

### Runtime

- Windows, Linux on X11, or Android targets.
  - MacOS support is experimental, and requires MoltenVK.
  - Wayland is untested and may have some edge cases.
- Vulkan 1.1+ capable GPU, loader, and driver.
- C++ runtime.

### Build time

- All runtime requirements, and:
- C++20 compiler and standard library*.
- CMake 3.22+.
- (Optional but highly recommended) Vulkan SDK and/or validation layers.
  - Android validation layers are downloaded by the example, feel free to copy them / the script into your project.
  - `bave` vendors Vulkan headers and loads functions at runtime, so the SDK / loader is not needed at build time.

> _*Usage of C++20 library features currently unsupported on Android NDK and MacOS clang (eg `std::format`, `std::ranges`, etc) has been avoided throughout bave._

## Example

See [example](example) for an app that's designed to target Android while being developed on the desktop. 
