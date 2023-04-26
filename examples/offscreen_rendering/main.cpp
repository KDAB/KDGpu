#include "offscreen.h"

#include <glm/glm.hpp>

#include <spdlog/spdlog.h>

#include <random>

using namespace KDGpu;

std::vector<glm::vec2> generateData(uint32_t count)
{
    std::vector<glm::vec2> data;
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
        // SPDLOG_INFO("{}, {}", x, y);

        data.emplace_back(float(x), float(y));
    }

    return data;
}

int main()
{
    // Let's prepare some data to plot
    const uint32_t dataPointCount = 300;
    const std::vector<glm::vec2> data = generateData(dataPointCount);

    Offscreen offscreen;
    offscreen.initializeScene();
    offscreen.setData(data);
    offscreen.render();
}
