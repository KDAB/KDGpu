# KDGpu

KDGpu is a thin wrapper around Vulkan to make modern graphics easier to learn and use.

## Introduction

### Is KDGpu for me?

Have you ever wanted to be productive with Vulkan but can't get your head around the verbose
syntax, managing object lifetimes, and the intricacies of just getting something working without
drowning in synchronization and memory handling?

If the answer is yes, then KDGpu is the library for you!

KDGpu is a thin wrapper around Vulkan (and perhaps later other modern graphics APIs) with
the aim of making modern graphics available to mortals and to massively reduce the amount
of verbosity involved.

KDGpu also aims to make it easy to teach the concepts of modern graphics programming without
having to be bogged down in the intricacies.

KDGpu is independent of any particular windowing system. You can use it purely with platform
native APIs or you can checkout the included KDGpuExample library which makes it trivial to
use KDGpu with KDGui::Window from the [KDUtils](https://github.com/KDAB/KDUtils) library.
Check out the handy [KDBindings](https://github.com/KDAB/KDBindings) repository too for some
more syntactic sugar with Signals and Slots, Properties and reactive Property Bindings.

In addition to being an enabler for Vulkan, the KDGpu project now also includes a new library,
KDXr, which aims to be an enabler for OpenXR. Using the combination of KDGpu and KDXr allows
application developers to very easily create AR and VR application experiences.

More information can be found in the [documentation](https://docs.kdab.com/kdgpu/unstable/index.html).

### What does KDGpu provide?

- The KDGpu library
- The KDXr library to make OpenXR easier to use
- An example framework called KDGpuExample that provides integration with [KDGui](https://github.com/KDAB/KDUtils)
  to make it easy to experiment. Let us take care of the boring bits so you can make pretty pictures.
- A set of illustrative examples showing how to use KDGpu for common rendering tasks.

### How do I get up and running with KDGpu?

- Install the [Vulkan SDK](https://vulkan.lunarg.com/), setup the runtime environment and verify the
  SDK as described in the [Vulkan documentation](https://vulkan.lunarg.com/doc/sdk)
- Clone this repository
- Open up the directory in VS Code with the CMakeTools extension loaded
- Configure and Build - all of the dependencies will be pulled down via CMake's FetchContent feature
  (see cmake/dependencies.cmake)
- Try running the examples

### What does KDGpu code look like?

A typical render function for KDGPu looks something like this:

```cpp
auto opaquePass = commandRecorder.beginRenderPass(m_opaquePassOptions);
opaquePass.setPipeline(m_pipeline);
opaquePass.setVertexBuffer(0, m_buffer);
opaquePass.setBindGroup(0, m_textureBindGroup);
opaquePass.draw(DrawCommand{ .vertexCount = 4 });
renderImGuiOverlay(&opaquePass);
opaquePass.end();
m_commandBuffer = commandRecorder.finish();
```

Creating GPU resources is just as easy and intuitive. Creating a Buffer that resides on the GPU
and can be uploaded to and used as a vertex buffer is as simple as:

```cpp
const DeviceSize dataByteSize = vertexData.size() * sizeof(Vertex);
BufferOptions bufferOptions = {
    .size = dataByteSize,
    .usage = BufferUsageFlagBits::VertexBufferBit | BufferUsageFlagBits::TransferDstBit,
    .memoryUsage = MemoryUsage::GpuOnly
};
m_buffer = m_device.createBuffer(bufferOptions);
```

This pattern of using options structs and initializing them with C++20 designated initializers
permeates through the API. It makes it easily discoverable, extensible and trivial to queue up
for deferred invocations.

Uploading data to the above buffer is just as easy:

```cpp
const BufferUploadOptions uploadOptions = {
    .destinationBuffer = m_buffer,
    .dstStages = PipelineStageFlagBit::VertexAttributeInputBit,
    .dstMask = AccessFlagBit::VertexAttributeReadBit,
    .data = vertexData.data(),
    .byteSize = dataByteSize
};
m_queue.uploadBufferData(uploadOptions);
```

## Contact

- Visit us on GitHub: <https://github.com/KDAB/KDGpu>
- Email info@kdab.com for questions about copyright, licensing or commercial support.

Stay up-to-date with KDAB product announcements:

- [KDAB Newsletter](https://news.kdab.com)
- [KDAB Blogs](https://www.kdab.com/category/blogs)
- [KDAB on Twitter](https://twitter.com/KDABQt)

## Licensing

KDGpu is © Klarälvdalens Datakonsult AB and is available under the terms of
the [MIT](https://github.com/KDAB/KDGpu/blob/main/LICENSES/MIT.txt) license.

Contact KDAB at <info@kdab.com> if you need different licensing options.

KDGpu includes these source files, also available under the terms of the MIT license:

[doctest.h](https://github.com/onqtam/doctest) - the lightest feature-rich C++ single-header testing
framework for unit tests and TDD (C) 2016-2021 Viktor Kirilov <vik.kirilov@gmail.com>

## Get Involved

Please submit your contributions or issue reports from our GitHub space at <https://github.com/KDAB/KDGpu>.

Contact <info@kdab.com> for more information.

## About KDAB

KDGpu is supported and maintained by Klarälvdalens Datakonsult AB (KDAB).

The [KDAB](https://www.kdab.com) Group is a globally recognized provider for software consulting, development and training, specializing in embedded devices and complex cross-platform desktop applications. In addition to being leading experts in Qt, C++ and 3D technologies for over two decades, KDAB provides deep expertise across the stack, including Linux, Rust and modern UI frameworks. With 100+ employees from 20 countries and offices in Sweden, Germany, USA, France and UK, KDAB serves clients around the world.

Please visit <https://www.kdab.com> to meet the people who write code like this.


Blogs and publications: https://www.kdab.com/resources

Videos (Tutorials and more): https://www.youtube.com/@KDABtv

Software Developer Training for Qt, Modern C++, Rust, OpenGL and more: https://training.kdab.com  

Software Consulting and Development Services for Embedded and Desktop Applications https://www.kdab.com/services/

