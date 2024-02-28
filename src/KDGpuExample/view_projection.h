#pragma once

#include <KDGpuExample/kdgpuexample_export.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <openxr/openxr.h>

namespace KDGpuExample {

KDGPUEXAMPLE_EXPORT glm::mat4 postViewCorrection();

enum class ApplyPostViewCorrection : uint8_t {
    No,
    Yes
};

// Planes are specified in camera space.
struct OrthoOptions {
    float left{ -1.0f };
    float right{ 1.0f };
    float bottom{ -1.0f };
    float top{ 1.0f };
    float nearPlane{ -1.0f };
    float farPlane{ 1.0f };
    ApplyPostViewCorrection applyPostViewCorrection{ ApplyPostViewCorrection::Yes };
};

// Constructs an orthographic projection matrix.
KDGPUEXAMPLE_EXPORT glm::mat4 ortho(const OrthoOptions &options);

// Options are specified in camera space. The near plane and far plane
// are the positive distances from the camera to the planes.
// TODO: Include an option to use reversed depth for greater floating point precision.
// TODO: Include an option to use an infinite far plane.
struct PerspectiveOptions {
    float verticalFieldOfView{ 45.0f };
    float aspectRatio{ 1.0f };
    float nearPlane{ 0.1f };
    float farPlane{ 100.0f };
    ApplyPostViewCorrection applyPostViewCorrection{ ApplyPostViewCorrection::Yes };
};

KDGPUEXAMPLE_EXPORT glm::mat4 perspective(const PerspectiveOptions &options);

struct AsymmetricPerspectiveOptions {
    float left{ -1.0f };
    float right{ 1.0f };
    float bottom{ -1.0f };
    float top{ 1.0f };
    float nearPlane{ 0.1f };
    float farPlane{ 100.0f };
    ApplyPostViewCorrection applyPostViewCorrection{ ApplyPostViewCorrection::Yes };
};

KDGPUEXAMPLE_EXPORT glm::mat4 perspective(const AsymmetricPerspectiveOptions &options);

struct AsymmetricFieldOfViewPerspectiveOptions {
    float leftFieldOfView{ -0.5f };
    float rightFieldOfView{ 0.5f };
    float upFieldOfView{ -0.5f };
    float downFieldOfView{ 0.5f };
    float nearPlane{ 0.1f };
    float farPlane{ 100.0f };
    ApplyPostViewCorrection applyPostViewCorrection{ ApplyPostViewCorrection::Yes };
};

KDGPUEXAMPLE_EXPORT glm::mat4 perspective(const AsymmetricFieldOfViewPerspectiveOptions &options);

struct ViewMatrixOptions {
    glm::quat orientation{ glm::identity<glm::quat>() };
    glm::vec3 position{ 0.0f };
};

KDGPUEXAMPLE_EXPORT glm::mat4 viewMatrix(const ViewMatrixOptions &options);

} // namespace KDGpuExample
