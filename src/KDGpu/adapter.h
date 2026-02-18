/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/adapter_features.h>
#include <KDGpu/adapter_properties.h>
#include <KDGpu/adapter_queue_type.h>
#include <KDGpu/adapter_swapchain_properties.h>
#include <KDGpu/device.h>
#include <KDGpu/device_options.h>
#include <KDGpu/handle.h>

#include <KDGpu/kdgpu_export.h>

#include <stdint.h>
#include <span>
#include <string>
#include <vector>

namespace KDGpu {

struct Adapter_t;
struct Instance_t;
struct Surface_t;

/*!
    \struct AdapterOptions
    \brief Configuration options for device creation from an adapter
    \ingroup public
    \headerfile adapter.h <KDGpu/adapter.h>

    Specifies device-level Vulkan layers and extensions to enable when creating a device.
    Most applications use default options; advanced use cases may enable extensions like
    ray tracing, mesh shaders, or debugging layers.

    <b>Usage example:</b>

    This struct is typically used when creating a device. See Adapter::createDevice() for usage examples.

    \sa Adapter::createDevice(), DeviceOptions
 */
struct AdapterOptions {
    std::vector<std::string> layers; //!< Device-level validation/debug layers
    std::vector<std::string> extensions; //!< Device extensions (e.g., "VK_KHR_swapchain", "VK_KHR_ray_tracing_pipeline")
};

/*!
    \class Adapter
    \brief Represents a physical GPU device available in the system
    \ingroup public
    \headerfile adapter.h <KDGpu/adapter.h>

    <b>Vulkan equivalent:</b> [VkPhysicalDevice](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDevice.html)

    An Adapter represents a physical GPU (graphics card) in the system. It provides
    information about the GPU's capabilities, features, and supported formats, and
    is used to create a logical Device for rendering.

    <b>Key responsibilities:</b>
    - Query GPU properties (name, type, version, limits)
    - Query supported features (geometry shaders, ray tracing, etc.)
    - Query format support (color formats, depth formats, compression)
    - Query queue families (graphics, compute, transfer capabilities)
    - Create logical devices for rendering

    <b>Lifetime:</b> Adapters are owned by the Instance and remain valid for its lifetime.
    Do not store Adapter pointers beyond the Instance's scope.

    <b>Enumerating adapters:</b>

    \snippet kdgpu_doc_snippets.cpp adapter_enumerate

    <b>Querying adapter properties:</b>

    \snippet kdgpu_doc_snippets.cpp adapter_properties

    <b>Checking features:</b>

    \snippet kdgpu_doc_snippets.cpp adapter_features

    <b>Checking limits:</b>

    \snippet kdgpu_doc_snippets.cpp adapter_limits

    <b>Querying queue families:</b>

    \snippet kdgpu_doc_snippets.cpp adapter_queue_types

    <b>Selecting a discrete GPU:</b>

    \snippet kdgpu_doc_snippets.cpp adapter_select_discrete

    <b>Checking surface compatibility:</b>

    \snippet kdgpu_doc_snippets.cpp adapter_swapchain_info

    <b>Querying format support:</b>

    \snippet kdgpu_doc_snippets.cpp adapter_format_support

    <b>Creating a device:</b>

    \snippet kdgpu_doc_snippets.cpp adapter_create_device

    <b>Vulkan mapping:</b>
    - Adapter is a wrapper around VkPhysicalDevice
    - Adapter::properties() → vkGetPhysicalDeviceProperties()
    - Adapter::features() → vkGetPhysicalDeviceFeatures()
    - Adapter::queueTypes() → vkGetPhysicalDeviceQueueFamilyProperties()
    - Adapter::formatProperties() → vkGetPhysicalDeviceFormatProperties()
    - Adapter::createDevice() → vkCreateDevice()

    \sa Instance, Instance::adapters(), Instance::selectAdapter(), Device
    \sa AdapterProperties, AdapterFeatures, AdapterQueueType
    \sa \ref kdgpu_api_overview
    \sa \ref kdgpu_vulkan_mapping
 */
class KDGPU_EXPORT Adapter
{
public:
    //! \brief Default constructor creates an invalid adapter
    Adapter() = default;

    //! \brief Destructor - note that adapter is owned by Instance, not destroyed here
    ~Adapter();

    //! \brief Move constructor transfers ownership of the adapter handle
    Adapter(Adapter &&);

    //! \brief Move assignment transfers ownership of the adapter handle
    Adapter &operator=(Adapter &&);

    //! \brief Copy constructor deleted - adapters cannot be copied
    Adapter(const Adapter &) = delete;

    //! \brief Copy assignment deleted - adapters cannot be copied
    Adapter &operator=(const Adapter &) = delete;

    //! \brief Returns the internal handle to the Vulkan physical device
    Handle<Adapter_t> handle() const noexcept { return m_adapter; }

    //! \brief Checks if this adapter is valid
    bool isValid() const noexcept { return m_adapter.isValid(); }

    //! \brief Implicit conversion to handle for passing to low-level APIs
    operator Handle<Adapter_t>() const noexcept { return m_adapter; }

    /*!
        \brief Queries all device extensions available on this adapter
        \return Vector of available extensions with name and version

        <b>Vulkan equivalent:</b> vkEnumerateDeviceExtensionProperties()

        \snippet kdgpu_doc_snippets.cpp adapter_extensions
     */
    std::vector<Extension> extensions() const;

    /*!
        \brief Returns GPU properties (name, type, version, limits, etc.)
        \return Reference to adapter properties (cached at adapter creation)

        Properties include:
        - Device name (e.g., "NVIDIA GeForce RTX 3080")
        - Device type (Discrete, Integrated, Virtual, CPU)
        - Vulkan API version
        - Driver version
        - Limits (max texture size, max uniform buffer size, etc.)
        - Unique IDs and vendor/device IDs

        <b>Vulkan equivalent:</b> vkGetPhysicalDeviceProperties()

        \sa AdapterProperties
     */
    const AdapterProperties &properties() const noexcept;

    /*!
        \brief Returns GPU feature support (shaders, ray tracing, etc.)
        \return Reference to adapter features (cached at adapter creation)

        Features include support for:
        - Shader stages (geometry, tessellation)
        - Ray tracing pipelines
        - Mesh shaders
        - Variable rate shading
        - And many more

        <b>Vulkan equivalent:</b> vkGetPhysicalDeviceFeatures()

        \sa AdapterFeatures
     */
    const AdapterFeatures &features() const noexcept;

    /*!
        \brief Returns available queue families on this GPU
        \return Span of queue type descriptors

        Each queue type describes a family of queues with specific capabilities:
        - Graphics queues (rendering)
        - Compute queues (compute shaders)
        - Transfer queues (memory copies)
        - Sparse binding queues

        <b>Vulkan equivalent:</b> vkGetPhysicalDeviceQueueFamilyProperties()

        \sa AdapterQueueType
     */
    std::span<AdapterQueueType> queueTypes() const;

    /*!
        \brief Queries swapchain capabilities for a given surface
        \param surface Handle to window surface
        \return Swapchain properties including supported formats and present modes

        <b>Vulkan equivalent:</b> vkGetPhysicalDeviceSurfaceCapabilitiesKHR() +
                                    vkGetPhysicalDeviceSurfaceFormatsKHR() +
                                    vkGetPhysicalDeviceSurfacePresentModesKHR()

        See also the class-level documentation for complete swapchain querying examples.

        \sa AdapterSwapchainProperties
     */
    AdapterSwapchainProperties swapchainProperties(const Handle<Surface_t> &surface) const;

    /*!
        \brief Checks if a queue family supports presenting to a surface
        \param surface Handle to window surface
        \param queueTypeIndex Index of the queue family to check
        \return true if this queue can present to the surface

        <b>Vulkan equivalent:</b> vkGetPhysicalDeviceSurfaceSupportKHR()

        \snippet kdgpu_doc_snippets.cpp adapter_presentation_support
     */
    bool supportsPresentation(const Handle<Surface_t> &surface, uint32_t queueTypeIndex) const noexcept;

    /*!
        \brief Queries format properties for a specific texture/image format
        \param format Format to query (e.g., Format::R8G8B8A8_UNORM)
        \return Format properties describing supported features

        <b>Vulkan equivalent:</b> vkGetPhysicalDeviceFormatProperties()

        See the class-level documentation for format support examples.

        \sa FormatProperties, Format
     */
    FormatProperties formatProperties(Format format) const;

    /*!
        \brief Checks if blitting (texture copying with scaling) is supported between two formats
        \param srcFormat Source texture format
        \param srcTiling Source texture tiling mode
        \param dstFormat Destination texture format
        \param dstTiling Destination texture tiling mode
        \return true if vkCmdBlitImage can be used between these formats

        \snippet kdgpu_doc_snippets.cpp adapter_blitting
     */
    bool supportsBlitting(Format srcFormat, TextureTiling srcTiling, Format dstFormat, TextureTiling dstTiling) const;

    /*!
        \brief Checks if a format supports blitting (shorthand for same src/dst format)
        \param format Texture format
        \param tiling Tiling mode
        \return true if blitting is supported for this format
     */
    bool supportsBlitting(Format format, TextureTiling tiling) const;

    /*!
        \brief Queries DRM format modifier properties (Linux-specific multi-plane formats)
        \param format Format to query
        \return Vector of DRM format modifier properties

        This is primarily used on Linux for direct scanout and multi-plane image formats.

        \sa DrmFormatModifierProperties
     */
    std::vector<DrmFormatModifierProperties> drmFormatModifierProperties(Format format) const;

    /*!
        \brief Creates a logical device from this adapter
        \param options Device creation options (queue families, features, extensions)
        \return Created device

        Creates a VkDevice which is the primary object for creating GPU resources.
        You typically create one device per adapter and use it for the application lifetime.

        <b>Vulkan mapping:</b> vkCreateDevice()

        See the class-level documentation for device creation examples.

        \snippet kdgpu_doc_snippets.cpp adapter_create_device

        \sa Device, DeviceOptions
     */
    Device createDevice(const DeviceOptions &options = DeviceOptions());

protected:
    explicit Adapter(GraphicsApi *api, const Handle<Adapter_t> &adapter);

    GraphicsApi *m_api{ nullptr };
    Handle<Adapter_t> m_adapter;

    AdapterProperties m_properties;
    AdapterFeatures m_features;
    mutable std::vector<AdapterQueueType> m_queueTypes;

    friend class Instance;
    friend class VulkanGraphicsApi;
};

} // namespace KDGpu
