# bave

**BAsic Vulkan Engine**

`bave` is a simple 2D engine written in C++20* and Vulkan.

> _*As of 2023-12-11, Android and MacOS clang don't have full library support for C++20 (eg `std::format`, `std::ranges`, etc). Usage of such features has been avoided in the library._

## Features

- [x] Desktop / Android support.
- [x] Efficient event queue.
- [x] Double buffered render resources.
- [x] Meshes and textures.
- [x] Shaders, optional custom descriptor sets (but fixed layout).
- [ ] `Drawable` API.

## Requirements

### Runtime

- Windows, Linux on X11, or Android targets.
  - MacOS support is experimental, and requires MoltenVK.
  - Wayland is untested and may have some edge cases.
- Vulkan 1.2+ capable GPU, loader, and driver.
- C++ runtime.

### Build time

- All runtime requirements, and:
- C++20 compiler and standard library.
- CMake 3.22+.
- (Optional but highly recommended) Vulkan SDK and/or validation layers.
  - Android validation layers are downloaded by the example, feel free to copy them / the script into your project.
  - `bave` vendors Vulkan headers and loads functions at runtime, so the SDK / loader is not needed at build time.

## Example

WIP.

See [example](example) for an app that's designed to target Android while being developed on the desktop. The common code is in [example/flappy](example/flappy), while platform specific executables and code are in [example/desktop](example/desktop) and [example/android](example/android). Open the latter in Android Studio to build for Android.
