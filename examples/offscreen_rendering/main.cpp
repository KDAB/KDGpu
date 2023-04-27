#include "offscreen.h"

#include <glm/glm.hpp>
#include <glm/gtx/color_space.hpp>

#include <spdlog/spdlog.h>

#include <random>

using namespace KDGpu;

std::vector<Offscreen::Vertex> generateData(uint32_t count)
{
    std::vector<Offscreen::Vertex> data;
    data.reserve(count);

    // Quadratic function with some added noise. Useful to test directly in NDC (-1, -1) to (1, 1)
    double xMin = -1.0f;
    double xMax = 1.0f;

    std::default_random_engine rndEngine(0);
    std::normal_distribution<float> rndDist(0.0f, 1.0f);
    const double noiseScale = 0.15;

    double dx = (xMax - xMin) / (count - 1);
    for (uint32_t i = 0; i < count; ++i) {
        double x = xMin + double(i) * dx;
        double y = 2.0 * x * x - 1.0 + rndDist(rndEngine) * noiseScale;

        // Hue goes from 0->360
        const glm::vec3 hsv = { 360.0f * (0.5f + x / (xMax - xMin)), 0.71f, 0.9f };
        const glm::vec3 rgb = glm::rgbColor(hsv);

        Offscreen::Vertex v = {
            .pos = { float(x), float(y) },
            .color = { rgb, 1.0f }
        };

        data.emplace_back(v);
    }

    return data;
}

int main()
{
    // Let's prepare some data to plot
    const uint32_t dataPointCount = 1000;
    const std::vector<Offscreen::Vertex> data = generateData(dataPointCount);

    // Set up the pipeline and other rendering resources (default is 1920x1080)
    Offscreen offscreen;
    offscreen.initializeScene();

    // Uncomment to do an 8k offscreen render
    // const uint32_t fullHdScale = 4;
    // offscreen.resize(fullHdScale * 1920, fullHdScale * 1080);

    // Upload the data to the GPU
    offscreen.setData(data);

    // Make some sample renders at different scales and save the results to disk
    offscreen.render("test-default");

    offscreen.setProjection(-3.0f, 3.0f, -3.0f, 3.0f);
    offscreen.render("test-zoomed-out");

    offscreen.setProjection(-0.5f, 0.0f, -1.1f, -0.5f);
    offscreen.render("test-zoomed-in");
}
