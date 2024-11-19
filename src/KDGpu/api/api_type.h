#pragma once

#include <cstdint>

namespace KDGpu {
enum class ApiType : uint8_t {
    Vulkan = 0,
    UserDefined = 255
};
}
