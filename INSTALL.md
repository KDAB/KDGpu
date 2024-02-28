# Building and Installing KDGpu

## Prerequisites

### Windows

- Microsoft Visual Studio C++ 2019 compiler (or later)
- [git](https://gitforwindows.org)
- CMake, make sure it is in your PATH

- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/)
  - Make sure to install the additional debug libraries when prompted during the install process
  - Make sure VULKAN_SDK is set accordingly in your environment

### Linux

- C++20 compiler (GCC / Clang)
- git
- CMake

- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/)
  - Make sure VULKAN_SDK is set accordingly in your environment
  - **Note:** We recommend installing the Vulkan SDK directly from [LunarG](https://www.lunarg.com/vulkan-sdk/)
    rather than relying on your distribution packages as those might not provide all of the tools
    and Vulkan layers commonly needed.

### Mac

- C++20 compiler (GCC / Clang)
- git
- CMake

- [MoltenVk SDK](https://github.com/KhronosGroup/MoltenVK)

### Hardware

Anything that supports Vulkan 1.2 or later

## Dependencies

Mandatory dependencies will be automatically fetched and downloaded during the configuration process.

### Optional Dependencies

#### Documentation

If you wish to build the documentation locally, you'll need to install the following dependencies

- [Doxygen](https://doxygen.nl)
- [doxybook2](https://github.com/matusnovak/doxybook2)
- [mkdocs](https://www.mkdocs.org/)
- [mkdocs-material](https://squidfunk.github.io/mkdocs-material/)

## Building & Installation

KDGpu builds with CMake. Unpack or clone the KDGpu sources in the directory of your choosing.

```bash
    mkdir build
    cd build
    cmake -DKDGPU_BUILD_EXAMPLES=ON -DCMAKE_INSTALL_PREFIX=/path/to/install/location/ ..
    cmake --build . --config Release
    cmake --install .
```

### CMake Options

- _KDGPU_BUILD_KDXR=ON_ to enable building of the KDXr library and OpenXR backend
- _KDGPU_BUILD_EXAMPLES=ON_ to enable building of the examples
- _KDGPU_BUILD_TESTS=ON_ to enable building of the tests
- _KDGPU_HLSL_SUPPORT=OFF_ to look for the dxc compiler and build an example using HLSL shaders
- _KDGPU_DOCS=ON_ to build the documentation
- _CMAKE_INSTALL_PREFIX=/path/to/install_ to override the default installation path

## Deployment

### Using KDGpu in your project

```cmake
    find_package(KDGpu REQUIRED)

    set(SOURCES main.cpp ...)

    add_executable(${PROJECT_NAME} ${SOURCES})

    target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
    target_link_libraries(${PROJECT_NAME} PRIVATE  KDGpu::KDGpu)
```

#### Notes

CMake will looks for a file named KDGpuConfig.cmake. This file is located in <kdgpu_install_dir>/lib/cmake/KDGpu.
If KDGpu was installed into a user defined location, you might have to set the
CMAKE_PREFIX_PATH variable to contain the KDGpu install path:

```cmake
    cmake -DCMAKE_PREFIX_PATH=<kdgpu_install_dir> ..
```

In case cmake is unable to locate the KDGpu CMake package, you could try to set CMAKE_FIND_DEBUG_MODE=ON
to get more insight about where CMake is looking for.

```cmake
    cmake -DCMAKE_PREFIX_PATH=<kdgpu_install_dir> -DCMAKE_FIND_DEBUG_MODE=ON ..
```
