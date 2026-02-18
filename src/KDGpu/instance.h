/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/adapter.h>
#include <KDGpu/device.h>
#include <KDGpu/gpu_core.h>
#include <KDGpu/handle.h>
#include <KDGpu/surface.h>
#include <KDGpu/surface_options.h>

#include <KDGpu/kdgpu_export.h>

#include <string>
#include <vector>

namespace KDGpu {

class Surface;
struct Instance_t;

/*!
    \struct InstanceOptions
    \brief Configuration options for creating a KDGpu Instance
    \ingroup public
    \headerfile instance.h <KDGpu/instance.h>

    Specifies the application name, version, and required Vulkan layers/extensions.
    Maps to [VkInstanceCreateInfo](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkInstanceCreateInfo.html).

    <b>Usage example:</b>

    \snippet kdgpu_doc_snippets.cpp instance_creation

    \sa Instance, Instance::createInstance()
 */
struct InstanceOptions {
    std::string applicationName{ "KDGpu Application" }; //!< Application name (used for Vulkan debugging)
    uint32_t applicationVersion{ KDGPU_MAKE_API_VERSION(0, 1, 0, 0) }; //!< Application version
    uint32_t apiVersion{ KDGPU_MAKE_API_VERSION(0, 1, 2, 0) }; //!< Highest Vulkan API version the app expects to use
    std::vector<std::string> layers; //!< Vulkan validation/debug layers to enable (e.g., "VK_LAYER_KHRONOS_validation")
    std::vector<std::string> extensions; //!< Instance extensions to enable (e.g., "VK_KHR_surface")
};

/*!
    \struct AdapterAndDevice
    \brief Convenience struct holding an adapter pointer and its created device
    \ingroup public
    \headerfile instance.h <KDGpu/instance.h>

    Returned by Instance::createDefaultDevice() to provide both the chosen adapter
    and the created device in one operation.

    \sa Instance::createDefaultDevice()
 */
struct AdapterAndDevice {
    Adapter *adapter; //!< Pointer to the selected adapter (owned by Instance)
    Device device; //!< Created device (move this into your application state)
};

/*!
    \class Instance
    \brief Represents a Vulkan instance - the root object for KDGpu applications
    \ingroup public
    \headerfile instance.h <KDGpu/instance.h>

    <b>Vulkan equivalent:</b> [VkInstance](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkInstance.html)

    The Instance is the first KDGpu object you create. It initializes the Vulkan library,
    enumerates available physical devices (adapters), and manages instance-level resources
    like validation layers and extensions.

    <b>Key responsibilities:</b>
    - Initialize Vulkan runtime
    - Enumerate physical devices (adapters)
    - Create window surfaces for rendering
    - Manage instance-level extensions (e.g., VK_KHR_surface, debug utils)
    - Provide access to validation layers
    .
    <br/>

    <b>Lifetime:</b> The Instance should live for the entire application runtime. All
    adapters and surfaces are owned by the Instance and become invalid when it's destroyed.

    ## Usage

    <b>Typical initialization flow:</b>

    \snippet kdgpu_doc_snippets.cpp instance_creation

    <b>Advanced adapter selection:</b>

    \snippet kdgpu_doc_snippets.cpp instance_adapters


    <b>Registering layers or instance extensions:</b>

    \snippet kdgpu_doc_snippets.cpp instance_extensions

    \note KDGpu will try to request some extensions by default KDGpu::getDefaultRequestedInstanceExtensions

    ## Vulkan mapping:
    - Instance::Instance() → vkCreateInstance()
    - Instance::adapters() → vkEnumeratePhysicalDevices()
    - Instance::createSurface() → Platform-specific (e.g., vkCreateXcbSurfaceKHR())
    - Instance::~Instance() → vkDestroyInstance()

    ## See also
    \sa Adapter, InstanceOptions, Device, Surface, InstanceOptions
    \sa \ref kdgpu_api_overview
    \sa \ref kdgpu_vulkan_mapping
 */
class KDGPU_EXPORT Instance
{
public:
    //! \brief Default constructor creates an invalid instance
    Instance();

    //! \brief Destructor destroys the Vulkan instance and all owned adapters
    ~Instance();

    //! \brief Move constructor transfers ownership of the Vulkan instance
    Instance(Instance &&) noexcept;

    //! \brief Move assignment transfers ownership of the Vulkan instance
    Instance &operator=(Instance &&) noexcept;

    //! \brief Copy constructor deleted - instances cannot be copied
    Instance(const Instance &) = delete;

    //! \brief Copy assignment deleted - instances cannot be copied
    Instance &operator=(const Instance &) = delete;

    /*!
        \brief Returns the internal handle to the Vulkan instance
        \return Handle to the underlying VkInstance
        \sa Handle, isValid()
     */
    [[nodiscard]] Handle<Instance_t> handle() const noexcept { return m_instance; }

    /*!
        \brief Checks if this instance is valid and ready for use
        \return true if the instance was successfully created, false otherwise

        An instance is invalid if:
        - It was default-constructed
        - It was moved from
        - Creation failed
     */
    [[nodiscard]] bool isValid() const { return m_instance.isValid(); }

    //! \brief Implicit conversion to handle for passing to low-level APIs
    operator Handle<Instance_t>() const noexcept { return m_instance; }

    /*!
        \brief Queries all instance extensions available in Vulkan
        \return Vector of available extensions with name and version

        Query this after creating the instance to check which extensions are supported:

        \snippet kdgpu_doc_snippets.cpp instance_extensions

        <b>Vulkan equivalent:</b> vkEnumerateInstanceExtensionProperties()
     */
    [[nodiscard]] std::vector<Extension> extensions() const;

    /*!
        \brief Creates a device using the best suitable adapter for the given surface
        \param surface Window surface for presentation (must be created first)
        \param deviceType Type of device to prefer (Default, Discrete, Integrated, CPU)
        \return Structure containing the selected adapter and created device

        This is a convenience function that:
        1. Selects an appropriate adapter based on deviceType
        2. Creates a device with defaults suitable for rendering to the surface
        3. Returns both for your use

        <b>Example:</b>

        \snippet kdgpu_doc_snippets.cpp instance_surface

        <b>Vulkan mapping:</b>
        - Calls vkEnumeratePhysicalDevices() to find adapters
        - Selects best match based on deviceType
        - Calls vkCreateDevice() with appropriate queue families

        \sa adapters(), selectAdapter(), Adapter::createDevice()
     */
    [[nodiscard]] AdapterAndDevice createDefaultDevice(const Surface &surface,
                                                       AdapterDeviceType deviceType = AdapterDeviceType::Default) const;

    /*!
        \brief Returns all available physical devices (GPUs)
        \return Vector of pointers to adapters (owned by this instance)

        Each adapter represents a physical GPU in the system. You can query properties
        and create devices from adapters:

        \snippet kdgpu_doc_snippets.cpp instance_api_version

        <b>Vulkan equivalent:</b> vkEnumeratePhysicalDevices()

        \sa Adapter, Adapter::properties(), selectAdapter()
     */
    [[nodiscard]] std::vector<Adapter *> adapters() const;

    /*!
        \brief Returns logical groups of adapters (for multi-GPU setups)
        \return Vector of adapter groups

        Adapter groups represent sets of GPUs that can work together (e.g., SLI/CrossFire).
        Most systems have one group per GPU.

        \sa AdapterGroup
     */
    [[nodiscard]] const std::vector<AdapterGroup> &adapterGroups() const;

    /*!
        \brief Selects an adapter matching the requested device type
        \param deviceType Type of device to select (Discrete, Integrated, CPU, etc.)
        \return Pointer to selected adapter, or nullptr if no match found

        Preference order when deviceType is Default:
        1. Discrete GPU
        2. Integrated GPU
        3. Virtual GPU
        4. CPU

        \sa AdapterDeviceType, adapters()
     */
    [[nodiscard]] Adapter *selectAdapter(AdapterDeviceType deviceType) const;

    /*!
        \brief Creates a window surface for presentation
        \param options Platform-specific surface creation options
        \return Surface object for rendering and presentation

        The surface represents the window where rendering will be displayed. Surface
        creation is platform-dependent (Windows, X11, Wayland, Android, etc.).

        <b>Vulkan mapping:</b> Platform-specific:
        - Windows: vkCreateWin32SurfaceKHR()
        - X11: vkCreateXcbSurfaceKHR() or vkCreateXlibSurfaceKHR()
        - Wayland: vkCreateWaylandSurfaceKHR()
        - Android: vkCreateAndroidSurfaceKHR()

        \sa Surface, SurfaceOptions
     */
    [[nodiscard]] Surface createSurface(const SurfaceOptions &options);

private:
    Instance(GraphicsApi *api, const InstanceOptions &options);

    GraphicsApi *m_api{ nullptr };
    Handle<Instance_t> m_instance;
    mutable std::vector<Adapter> m_adapters;
    mutable std::vector<AdapterGroup> m_adapterGroups;

    friend class VulkanGraphicsApi;
};

} // namespace KDGpu
