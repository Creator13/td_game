# td_game

An attempt to build a 3D tower defense game from scratch that escalated into building a fully fledged 3D renderer.

## Technology & features
- Handcrafted free-range OpenGL 4.6 renderer with automatic instancing.
- Fully test-covered custom math library (because I couldn't be bothered with glm and its annoying include patterns).
- ECS-first approach powered by [flecs](https://github.com/SanderMertens/flecs).
- Custom asset management system.
- SDF-based font renderer based on [msdfgen](https://github.com/Chlumsky/msdfgen).
- Built on C++23 (using cmake and vcpkg, targeting clang-cl) and GLFW for multiplatform support. Built on Windows, tested on Linux (compiled on clang) from time to time.

## Screenshots

Will put some here soon (dm me if you want to see now)

## Future plans
- Have fun.
- Expand engine/renderer features until it is capable of handling a small tower defense game.
- Continue implementing features that are fun to implement.
- Convert to Vulkan.
- Build-time asset system.

## Test for yourself

You can clone this repository and test the project for yourself. All you need is a working C++23 compiler, cmake 3.31 or higher, and a vcpkg distribution. My recommended environment is CLion, and I use clang-cl against MSVC 18/2026 to compile the project. I test against MinGW from time to time but I can not guarantee it works at any given point. 

The included CMakePresets.json includes a preset for windows called `windows-msvc-static` which is the preferred build configuration and should work against a standard installation of cmake on a windows machine with Visual Studio Build Tools 18 2026 installed. Run the following cmake commands to configure and build:
```
cmake --preset windows-msvc-static
cmake --build --preset relWithDeb-windows-msvc-static
```

---
All code is free to look at, criticize, compliment, test out, but not use, copy or reproduce. Copyrighted by me.
