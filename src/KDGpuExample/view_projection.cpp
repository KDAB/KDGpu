#include "view_projection.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <cmath>

namespace KDGpuExample {

// See https://www.kdab.com/projection-matrices-with-vulkan-part-1/
// Converts from y-up, -ve z-in view space to y-down, +ve z-in view space in preparation for being
// further transformed by a projection matrix that does not have any magic axis flipping built in.
glm::mat4 postViewCorrection()
{
    glm::mat4 x(1.0f);
    x[1][1] = -1.0f;
    x[2][2] = -1.0f;
    return x;
}

glm::mat4 ortho(const OrthoOptions &options)
{
    const auto farMinusNear = options.farPlane - options.nearPlane;
    const auto rightMinusLeft = options.right - options.left;

    if (options.applyPostViewCorrection == ApplyPostViewCorrection::No) {
        const auto bottomMinusTop = options.bottom - options.top;
        const glm::mat4 m = {
            2.0f / rightMinusLeft,
            0.0f,
            0.0f,
            0.0f,

            0.0f,
            2.0f / bottomMinusTop,
            0.0f,
            0.0f,

            0.0f,
            0.0f,
            1.0f / farMinusNear,
            0.0f,

            -(options.right + options.left) / rightMinusLeft,
            -(options.bottom + options.top) / bottomMinusTop,
            -options.nearPlane / farMinusNear,
            1.0f
        };

        return m;
    } else {
        // If we are applying the post view correction, we need to negate the signs of the
        // top and bottom planes to take into account the fact that the post view correction
        // rotate them 180 degrees around the x axis.
        //
        // This has the effect of treating the top and bottom planes as if they were specified
        // in the non-rotated eye space coordinate system.
        //
        // We do not need to flip the signs of the near and far planes as these are always
        // treated as positive distances from the camera.
        const auto bottom = -options.bottom;
        const auto top = -options.top;
        const auto bottomMinusTop = bottom - top;

        // In addition to negating the top and bottom planes, we also need to post-multiply
        // the projection matrix by the post view correction matrix. This amounts to negating
        // the y and z axes of the projection matrix.
        const glm::mat4 m = {
            2.0f / rightMinusLeft,
            0.0f,
            0.0f,
            0.0f,

            0.0f,
            -2.0f / bottomMinusTop,
            0.0f,
            0.0f,

            0.0f,
            0.0f,
            -1.0f / farMinusNear,
            0.0f,

            -(options.right + options.left) / rightMinusLeft,
            -(bottom + top) / bottomMinusTop,
            -options.nearPlane / farMinusNear,
            1.0f
        };

        return m;
    }
}

glm::mat4 perspective(const PerspectiveOptions &options)
{
    const float t = tanf(glm::radians(options.verticalFieldOfView / 2.0f));
    const auto farMinusNear = options.farPlane - options.nearPlane;

    if (options.applyPostViewCorrection == ApplyPostViewCorrection::No) {
        const glm::mat4 m = {
            1.0f / (options.aspectRatio * t),
            0.0f,
            0.0f,
            0.0f,

            0.0f,
            1.0f / t,
            0.0f,
            0.0f,

            0.0f,
            0.0f,
            options.farPlane / farMinusNear,
            1.0f,

            0.0f,
            0.0f,
            -options.nearPlane * options.farPlane / farMinusNear,
            0.0f
        };

        return glm::mat4(m);
    } else {
        // We need to post-multiply the projection matrix by the post view correction matrix.
        // This amounts to negating the y and z axes of the projection matrix. As this function
        // creates a frustum that is symmetric about the z axis there is no need to negate
        // anything else (unlike the asymmetric case below).
        const glm::mat4 m = {
            1.0f / (options.aspectRatio * t),
            0.0f,
            0.0f,
            0.0f,

            0.0f,
            -1.0f / t,
            0.0f,
            0.0f,

            0.0f,
            0.0f,
            -options.farPlane / farMinusNear,
            -1.0f,

            0.0f,
            0.0f,
            -options.nearPlane * options.farPlane / farMinusNear,
            0.0f
        };

        return glm::mat4(m);
    }
}

glm::mat4 perspective(const AsymmetricPerspectiveOptions &options)
{
    const auto twoNear = 2.0f * options.nearPlane;
    const auto rightMinusLeft = options.right - options.left;
    const auto farMinusNear = options.farPlane - options.nearPlane;

    if (options.applyPostViewCorrection == ApplyPostViewCorrection::No) {
        const auto bottomMinusTop = options.bottom - options.top;

        const glm::mat4 m = {
            twoNear / rightMinusLeft,
            0.0f,
            0.0f,
            0.0f,

            0.0f,
            twoNear / bottomMinusTop,
            0.0f,
            0.0f,

            -(options.right + options.left) / rightMinusLeft,
            -(options.bottom + options.top) / bottomMinusTop,
            options.farPlane / farMinusNear,
            1.0f,

            0.0f,
            0.0f,
            -options.nearPlane * options.farPlane / farMinusNear,
            0.0f
        };

        return m;
    } else {
        // If we are applying the post view correction, we need to negate the signs of the
        // top and bottom planes to take into account the fact that the post view correction
        // rotate them 180 degrees around the x axis.
        //
        // This has the effect of treating the top and bottom planes as if they were specified
        // in the non-rotated eye space coordinate system.
        //
        // We do not need to flip the signs of the near and far planes as these are always
        // treated as positive distances from the camera.
        const auto bottom = -options.bottom;
        const auto top = -options.top;
        const auto bottomMinusTop = bottom - top;

        // In addition to negating the top and bottom planes, we also need to post-multiply
        // the projection matrix by the post view correction matrix. This amounts to negating
        // the y and z axes of the projection matrix.
        const glm::mat4 m = {
            twoNear / rightMinusLeft,
            0.0f,
            0.0f,
            0.0f,

            0.0f,
            -twoNear / (bottomMinusTop),
            0.0f,
            0.0f,

            (options.right + options.left) / rightMinusLeft,
            (bottom + top) / bottomMinusTop,
            -options.farPlane / farMinusNear,
            -1.0f,

            0.0f,
            0.0f,
            -options.nearPlane * options.farPlane / farMinusNear,
            0.0f
        };

        return m;
    }
}

glm::mat4 perspective(const AsymmetricFieldOfViewPerspectiveOptions &options)
{
    const AsymmetricPerspectiveOptions options2 = {
        .left = options.nearPlane * tanf(options.leftFieldOfView),
        .right = options.nearPlane * tanf(options.rightFieldOfView),
        .bottom = options.nearPlane * tanf(options.downFieldOfView),
        .top = options.nearPlane * tanf(options.upFieldOfView),
        .nearPlane = options.nearPlane,
        .farPlane = options.farPlane,
        .applyPostViewCorrection = options.applyPostViewCorrection
    };
    return perspective(options2);
}

glm::mat4 viewMatrix(const ViewMatrixOptions &options)
{
    glm::mat4 mTrans = glm::translate(glm::mat4(1.0f), options.position);
    glm::mat4 mRot = glm::toMat4(options.orientation);
    const auto m = mTrans * mRot;

    // View matrix is the inverse of the camera's model-to-world matrix.
    return glm::inverse(m);
}

} // namespace KDGpuExample
