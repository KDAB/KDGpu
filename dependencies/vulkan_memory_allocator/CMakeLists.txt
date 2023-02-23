FetchContent_Declare(
    vulkan_memory_allocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG 55868965ae1fa956c07695d4642e1add8c9450f7
)
FetchContent_Populate(vulkan_memory_allocator)

set(VULKAN_MEMORY_ALLOCATOR_LIBRARY_SOURCE_FILES
    ${vulkan_memory_allocator_SOURCE_DIR}/src/VmaUsage.cpp
)

find_package(Vulkan REQUIRED)

add_library(vulkan_memory_allocator STATIC ${VULKAN_MEMORY_ALLOCATOR_LIBRARY_SOURCE_FILES})
set_target_properties(vulkan_memory_allocator PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(vulkan_memory_allocator PUBLIC Vulkan::Vulkan)
set_property(TARGET vulkan_memory_allocator PROPERTY CXX_STANDARD 11)

target_include_directories(vulkan_memory_allocator PRIVATE
    $<BUILD_INTERFACE:${vulkan_memory_allocator_SOURCE_DIR}/src>
)

target_include_directories(vulkan_memory_allocator PUBLIC
    $<BUILD_INTERFACE:${vulkan_memory_allocator_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/toy_renderer/vulkan_memory_allocator>
)

option(VMA_RECORDING_ENABLED "Enable VMA memory recording for debugging" OFF)
option(VMA_USE_STL_CONTAINERS "Use C++ STL containers instead of VMA's containers" OFF)
option(VMA_STATIC_VULKAN_FUNCTIONS "Link statically with Vulkan API" OFF)
option(VMA_DYNAMIC_VULKAN_FUNCTIONS "Fetch pointers to Vulkan functions internally (no static linking)" OFF)
option(VMA_DEBUG_ALWAYS_DEDICATED_MEMORY "Every allocation will have its own memory block" OFF)
option(VMA_DEBUG_INITIALIZE_ALLOCATIONS "Automatically fill new allocations and destroyed allocations with some bit pattern" OFF)
option(VMA_DEBUG_GLOBAL_MUTEX "Enable single mutex protecting all entry calls to the library" OFF)
option(VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT "Never exceed VkPhysicalDeviceLimits::maxMemoryAllocationCount and return error" OFF)

target_compile_definitions(
    vulkan_memory_allocator

    PUBLIC
    VMA_USE_STL_CONTAINERS=$<BOOL:${VMA_USE_STL_CONTAINERS}>
    VMA_DYNAMIC_VULKAN_FUNCTIONS=$<BOOL:${VMA_DYNAMIC_VULKAN_FUNCTIONS}>
    VMA_DEBUG_ALWAYS_DEDICATED_MEMORY=$<BOOL:${VMA_DEBUG_ALWAYS_DEDICATED_MEMORY}>
    VMA_DEBUG_INITIALIZE_ALLOCATIONS=$<BOOL:${VMA_DEBUG_INITIALIZE_ALLOCATIONS}>
    VMA_DEBUG_GLOBAL_MUTEX=$<BOOL:${VMA_DEBUG_GLOBAL_MUTEX}>
    VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT=$<BOOL:${VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT}>
    VMA_RECORDING_ENABLED=$<BOOL:${VMA_RECORDING_ENABLED}>
)

add_library(ToyRenderer::vulkan_memory_allocator ALIAS vulkan_memory_allocator)

install(FILES ${vulkan_memory_allocator_SOURCE_DIR}/include/vk_mem_alloc.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/toy_renderer/vulkan_memory_allocator/)
