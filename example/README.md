# Bave Example

The example project demonstrates usage of `bave` to create both desktop and Android apps. The directory structure is:

```
example/                              # root directory
|-- assets/                           # assets
    |-- ...                           # compiled shaders (SPIR-V), textures, animations, etc.
|-- flappy/                           # game code / entrypoint
    |-- glsl/                         # GLSL (shader) sources
    |-- src/                          # C++ sources
    |-- CMakeLists.txt                # static library target
|-- android/                          # android project root
    |-- app/
        |-- src/
            |-- cpp/
                |-- main.cpp          # android_main()
                |-- CMakeLists.txt    # Android shared library target
    ...                               # AndroidManifest, build.gradle, etc.
|-- desktop/                          # desktop project root
    |-- main.cpp                      # int main()
    |-- CMakeLists.txt                # desktop executable target
```

- [assets](assets): Contains all assets used by the game at runtime. This directory is copied over to `android/app/src/main/assets` during a build.
- [flappy](flappy): Library containing all game code and logic.
- [android](android): Android project root - open this in Android Studio to build it / use `gradlew`. Links to `flappy`.
- [desktop](desktop)`: Desktop project root - executable target. Links to `flappy`.

The desktop target is quite straightforward: it simply creates an executable with one source file, and links it to `flappy`. The subdirectory is added to the build tree if `BAVE_BUILD_EXAMPLE` is set in CMake, so nothing else is required. Your own project would instead be the root project that imports / adds `bave` via `FetchContent` / `add_subdirectory` / etc.

The Android target is a bit more complex, as it needs to import `bave` and `flappy` into the build tree, which is several parent directories up. Your own project would instead have `bave` in a subdirectory somewhere (whether organized manually or in an automated fashion as with `FetchContent`). This example also does a few other things:

- Downloads Vulkan validation layers (shared libraries) and unzips them into `android/app/src/debug/` - this means that Release builds will be much smaller but also not have validation layers.
- Adds a custom target that (deletes and then) copies `assets/` into `android/app/src/main/` to be picked up for packaging into the APK / App Bundle by Gradle.

The code in the final targets - ie that's platform-specific - is a tiny wrapper that sets up a `bave::App` instance, its game factory (that returns a new instance of `Flappy`), and calls `run()` on it. All other user-side code is platform agnostic - with the exception of Dear ImGui usage, which is only avaiable on desktop builds.
