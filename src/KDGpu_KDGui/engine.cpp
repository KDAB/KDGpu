#include "engine.h"

#include <KDFoundation/core_application.h>
#include <KDFoundation/postman.h>

namespace KDGpuKDGui {

using namespace KDBindings;

Engine::Engine()
    : KDFoundation::Object()
{
    m_logger = spdlog::get("engine");
    if (!m_logger) {
        m_logger = spdlog::stdout_color_mt("engine");
        SPDLOG_LOGGER_INFO(m_logger, "Hello from the Engine Logger");
    }

    running.valueChanged().connect(&Engine::onRunningChanged, this);

    auto app = KDFoundation::CoreApplication::instance();
    if (app)
        app->postman()->addFilter(this);
}

Engine::~Engine()
{
    // Stop execution of the engine
    running = false;

    auto app = KDFoundation::CoreApplication::instance();
    if (app)
        app->postman()->removeFilter(this);

    // Detach and destroy all of the engine layers
    int32_t layerIndex = static_cast<int32_t>(m_engineLayers.size());
    while (layerIndex > 0) {
        EngineLayer *rawLayer = m_engineLayers.at(--layerIndex).get();
        auto layer = detachEngineLayer(rawLayer);
    }
}

void Engine::onRunningChanged(const bool &newValue)
{
    if (newValue == true) {
        // Starting up
        SPDLOG_LOGGER_INFO(m_logger, "Engine {} starting", objectName());

        // Record the start time
        m_frameCounter = 0;
        m_totalFrameCounter = 0;
        m_startTime = std::chrono::high_resolution_clock::now();
        m_currentFrameTime = m_startTime;

        // Execute the first frame and request the next one
        doFrame();
        requestFrame();
    } else {
        // Shutting down
        SPDLOG_LOGGER_INFO(m_logger, "Engine {} stopping", objectName());

        // Execute the last frame and do not request another
        doFrame();

        SPDLOG_LOGGER_INFO(m_logger, "Simulated a total of {} frames.", frameNumber());
    }
}

EngineLayer *Engine::attachEngineLayer(std::unique_ptr<EngineLayer> &&engineLayer)
{
    // Caller has to transfer ownership to us so there should not be an old parent
    assert(engineLayer->engine() == nullptr);

    engineLayer->m_engine = this;
    m_engineLayers.push_back(std::move(engineLayer));
    auto layerPtr = m_engineLayers.back().get();
    layerPtr->onAttached();
    return layerPtr;
}

std::unique_ptr<EngineLayer> Engine::detachEngineLayer(EngineLayer *engineLayer)
{
    // Find the child from the raw pointer
    auto engineLayerIt = std::find_if(
            m_engineLayers.begin(),
            m_engineLayers.end(),
            [engineLayer](const auto &v) {
                return v.get() == engineLayer;
            });

    // Didn't find a matching application layer?
    if (engineLayerIt == m_engineLayers.end())
        return {};

    // Remove the component and return it along with ownership!
    auto takenEngineLayer = std::move(*engineLayerIt);
    takenEngineLayer->onDetached();
    takenEngineLayer->m_engine = nullptr;
    m_engineLayers.erase(engineLayerIt);
    return takenEngineLayer;
}

void Engine::event(KDFoundation::EventReceiver *target, KDFoundation::Event *ev)
{
    // Give the application layers each a chance to process the event.
    // We process these in the reverse order in which they were attached.
    auto it = m_engineLayers.rbegin();
    while (it != m_engineLayers.rend()) {
        (*it)->event(target, ev);
        ++it;
    }

    // Handle events we care about
    if (target == this && ev->type() == KDFoundation::Event::Type::Update) {
        // Do stuff for this frame
        doFrame();

        // Request the next frame
        if (running.get())
            requestFrame();

        ev->setAccepted(true);
    }

    Object::event(target, ev);
}

void Engine::requestFrame()
{
    KDFoundation::CoreApplication::instance()->postEvent(this, std::make_unique<KDFoundation::UpdateEvent>());
}

void Engine::doFrame()
{
    SPDLOG_LOGGER_TRACE(m_logger, "{}()", __FUNCTION__);

    // Record the frame time
    m_previousFrameTime = m_currentFrameTime;
    m_currentFrameTime = std::chrono::high_resolution_clock::now();

    // Let each application layer do any necessary processing in the order in which they were attached
    for (const auto &engineLayer : m_engineLayers)
        engineLayer->update();

    // Update frame count, and once per second update the fps too
    ++m_frameCounter;
    ++m_totalFrameCounter;
    const auto frameEndTime = std::chrono::high_resolution_clock::now();
    const auto fpsTimer = std::chrono::duration<double, std::milli>(frameEndTime - m_lastFpsTimestamp).count();
    if (fpsTimer > 1000.0) {
        fps = m_frameCounter * (1000.0 / fpsTimer);
        m_frameCounter = 0;
        m_lastFpsTimestamp = frameEndTime;

        SPDLOG_LOGGER_TRACE(m_logger, "fps = {}", fps());
    }
}

} // namespace KDGpuKDGui
