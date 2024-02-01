/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "hello_xr.h"
#include "projection_layer.h"

#include <KDGpuExample/engine.h>
#include <KDGpuExample/xr_compositor/xr_cylinder_imgui_layer.h>
#include <KDGpuExample/xr_compositor/xr_quad_imgui_layer.h>

#include <KDXr/session.h>

void HelloXr::onAttached()
{
    XrExampleEngineLayer::onAttached();

    // Create a projection layer to render the 3D scene
    const XrProjectionLayerOptions projectionLayerOptions = {
        .device = &m_device,
        .queue = &m_queue,
        .session = &m_session,
        .colorSwapchainFormat = m_colorSwapchainFormat,
        .depthSwapchainFormat = m_depthSwapchainFormat,
        .samples = m_samples.get()
    };
    m_projectionLayer = createCompositorLayer<ProjectionLayer>(projectionLayerOptions);
    m_projectionLayer->setReferenceSpace(m_referenceSpace);

    // Create a quad layer to render the ImGui overlay
    const XrQuadLayerOptions quadLayerOptions = {
        .device = &m_device,
        .queue = &m_queue,
        .session = &m_session,
        .colorSwapchainFormat = m_colorSwapchainFormat,
        .depthSwapchainFormat = m_depthSwapchainFormat,
        .samples = m_samples.get()
    };
    m_quadImguiLayer = createCompositorLayer<XrQuadImGuiLayer>(quadLayerOptions);
    m_quadImguiLayer->setReferenceSpace(m_referenceSpace);
    m_quadImguiLayer->position = { -1.0f, 0.2f, -1.5f };

    // Create a cylinder layer to render the ImGui overlay
    const XrCylinderLayerOptions cylinderLayerOptions = {
        .device = &m_device,
        .queue = &m_queue,
        .session = &m_session,
        .colorSwapchainFormat = m_colorSwapchainFormat,
        .depthSwapchainFormat = m_depthSwapchainFormat,
        .samples = m_samples.get()
    };
    m_cylinderImguiLayer = createCompositorLayer<XrCylinderImGuiLayer>(cylinderLayerOptions);
    m_cylinderImguiLayer->setReferenceSpace(m_referenceSpace);
    m_cylinderImguiLayer->position = { 1.0f, 0.2f, 0.0f };
    m_cylinderImguiLayer->radius = 2.0f;
    m_cylinderImguiLayer->centralAngle = 1.0f; // 1 radians = 57.3 degrees

    // Create an action set and actions
    m_actionSet = m_xrInstance.createActionSet({ .name = "default", .localizedName = "Default" });
    m_toggleAnimationAction = m_actionSet.createAction({ .name = "toggle_animation",
                                                         .localizedName = "Toggle Animation",
                                                         .type = KDXr::ActionType::BooleanInput,
                                                         .subactionPaths = m_handPaths });
    m_scaleAction = m_actionSet.createAction({ .name = "scale",
                                               .localizedName = "Scale",
                                               .type = KDXr::ActionType::FloatInput,
                                               .subactionPaths = { m_handPaths[0] } });
    m_translateAction = m_actionSet.createAction({ .name = "translate",
                                                   .localizedName = "Translate",
                                                   .type = KDXr::ActionType::Vector2Input,
                                                   .subactionPaths = { m_handPaths[0] } });
    m_palmPoseAction = m_actionSet.createAction({ .name = "palm_pose",
                                                  .localizedName = "Palm Pose",
                                                  .type = KDXr::ActionType::PoseInput,
                                                  .subactionPaths = m_handPaths });
    m_buzzAction = m_actionSet.createAction({ .name = "buzz",
                                              .localizedName = "Buzz",
                                              .type = KDXr::ActionType::VibrationOutput,
                                              .subactionPaths = m_handPaths });

    // Create action spaces for the palm poses. Default is no offset from the palm pose. If you wish to
    // apply an offset, you can do so by setting the poseInActionSpace member of the ActionSpaceOptions.
    for (uint32_t i = 0; i < 2; ++i)
        m_palmPoseActionSpaces[i] = m_session.createActionSpace({ .action = m_palmPoseAction, .subactionPath = m_handPaths[i] });

    // Suggest some bindings for the actions. NB: This assumes we are using a Meta Quest. If you are using a different
    // device, you will need to change the suggested bindings.
    const auto bindingOptions = KDXr::SuggestActionBindingsOptions{
        .interactionProfile = "/interaction_profiles/oculus/touch_controller",
        .suggestedBindings = {
                { .action = m_toggleAnimationAction, .binding = "/user/hand/left/input/x/click" },
                { .action = m_toggleAnimationAction, .binding = "/user/hand/right/input/a/click" },
                { .action = m_scaleAction, .binding = "/user/hand/left/input/trigger/value" },
                { .action = m_translateAction, .binding = "/user/hand/left/input/thumbstick" },
                { .action = m_palmPoseAction, .binding = "/user/hand/left/input/grip/pose" },
                { .action = m_palmPoseAction, .binding = "/user/hand/right/input/grip/pose" },
                { .action = m_buzzAction, .binding = "/user/hand/left/output/haptic" },
                { .action = m_buzzAction, .binding = "/user/hand/right/output/haptic" } }
    };
    if (m_xrInstance.suggestActionBindings(bindingOptions) != KDXr::SuggestActionBindingsResult::Success) {
        SPDLOG_LOGGER_ERROR(m_logger, "Failed to suggest action bindings.");
    }

    // Attach the action set to the session
    const auto attachOptions = KDXr::AttachActionSetsOptions{ .actionSets = { m_actionSet } };
    if (m_session.attachActionSets(attachOptions) != KDXr::AttachActionSetsResult::Success) {
        SPDLOG_LOGGER_ERROR(m_logger, "Failed to attach action set.");
    }
}

void HelloXr::onDetached()
{
    clearCompositorLayers();
    m_quadImguiLayer = nullptr;
    m_projectionLayer = nullptr;
    XrExampleEngineLayer::onDetached();
}

void HelloXr::onInteractionProfileChanged()
{
    if (!m_session.isValid())
        return;
    SPDLOG_LOGGER_INFO(m_logger, "Interaction Profile Changed.");

    auto profileState = m_session.getInteractionProfile({ .topLevelUserPath = m_handPaths[0] });
    if (profileState.result == KDXr::GetInteractionProfileResult::Success) {
        SPDLOG_LOGGER_INFO(m_logger, "Interaction Profile for {}: {}", m_handPaths[0], profileState.interactionProfile);
    } else {
        SPDLOG_LOGGER_ERROR(m_logger, "Failed to get interaction profile.");
    }

    profileState = m_session.getInteractionProfile({ .topLevelUserPath = m_handPaths[1] });
    if (profileState.result == KDXr::GetInteractionProfileResult::Success) {
        SPDLOG_LOGGER_INFO(m_logger, "Interaction Profile for {}: {}", m_handPaths[1], profileState.interactionProfile);
    } else {
        SPDLOG_LOGGER_ERROR(m_logger, "Failed to get interaction profile.");
    }
}

void HelloXr::pollActions(KDXr::Time predictedDisplayTime)
{
    // Sync the action set
    const auto syncActionOptions = KDXr::SyncActionsOptions{ .actionSets = { { m_actionSet } } };
    const auto syncActionResult = m_session.syncActions(syncActionOptions);
    if (syncActionResult != KDXr::SyncActionsResult::Success) {
        SPDLOG_LOGGER_ERROR(KDXr::Logger::logger(), "Failed to sync action set.");
        return;
    }

    // Poll the actions and do something with the results
    processToggleAnimationAction();
    processScaleAction();
    processTranslateAction();
    processPalmPoseAction(predictedDisplayTime);
    processHapticAction();
}

void HelloXr::processToggleAnimationAction()
{
    bool toggleAnimation{ false };
    for (uint32_t i = 0; i < 2; ++i) {
        // Query the toggle animation action
        const auto toggleAnimationResult = m_session.getBooleanState(
                { .action = m_toggleAnimationAction, .subactionPath = m_handPaths[i] },
                m_toggleAnimationActionStates[i]);
        if (toggleAnimationResult == KDXr::GetActionStateResult::Success) {
            if (m_toggleAnimationActionStates[i].currentState &&
                m_toggleAnimationActionStates[i].changedSinceLastSync &&
                m_toggleAnimationActionStates[i].active) {
                toggleAnimation = true;
                m_buzzHand = i;
                break;
            }
        } else {
            SPDLOG_LOGGER_ERROR(KDXr::Logger::logger(), "Failed to get toggle animation action state.");
        }
    }

    // If the toggle animation action was pressed, toggle the animation and buzz the controller
    if (toggleAnimation) {
        m_projectionLayer->animate = !m_projectionLayer->animate();
        m_buzzAmplitudes[m_buzzHand] = 1.0f;
        SPDLOG_LOGGER_INFO(KDXr::Logger::logger(), "Animation enabled = {}", m_projectionLayer->animate());
    }
}

void HelloXr::processScaleAction()
{
    // Query the scale action from the left trigger value
    float scale = 1.0f;
    const auto scaleResult = m_session.getFloatState({ .action = m_scaleAction, .subactionPath = m_handPaths[0] }, m_scaleActionState);
    if (scaleResult == KDXr::GetActionStateResult::Success) {
        if (m_scaleActionState.active)
            scale = 1.0 + m_scaleActionState.currentState;
        m_projectionLayer->scale = scale;
    } else {
        SPDLOG_LOGGER_ERROR(KDXr::Logger::logger(), "Failed to get scale action state.");
    }
}

void HelloXr::processTranslateAction()
{
    // Query the translate action from the left thumbstick
    const float dt = engine()->deltaTimeSeconds();
    glm::vec3 delta{ 0.0f, 0.0f, 0.0f };
    const auto translateResult = m_session.getVector2State(
            { .action = m_translateAction, .subactionPath = m_handPaths[0] }, m_translateActionState);
    if (translateResult == KDXr::GetActionStateResult::Success) {
        if (m_translateActionState.active) {
            delta = dt * glm::vec3(m_translateActionState.currentState.x, 0.0f, -m_translateActionState.currentState.y);
            delta *= m_linearSpeed;
        }
        m_projectionLayer->translation = m_projectionLayer->translation() + delta;
    } else {
        SPDLOG_LOGGER_ERROR(KDXr::Logger::logger(), "Failed to get translate action state.");
    }
}

void HelloXr::processPalmPoseAction(KDXr::Time predictedDisplayTime)
{
    for (uint32_t i = 0; i < 2; ++i) {
        // Query the palm pose action
        const auto palmPoseResult = m_session.getPoseState(
                { .action = m_palmPoseAction, .subactionPath = m_handPaths[i] }, m_palmPoseActionStates[i]);
        if (palmPoseResult == KDXr::GetActionStateResult::Success) {
            if (m_palmPoseActionStates[i].active) {
                // Update the action space for the palm pose
                const auto locateSpaceResult = m_palmPoseActionSpaces[i].locateSpace(
                        { .baseSpace = m_referenceSpace, .time = predictedDisplayTime }, m_palmPoseActionSpaceStates[i]);
                if (locateSpaceResult == KDXr::LocateSpaceResult::Success) {
                    // Update the pose of the projection layer
                    if (i == 0)
                        m_projectionLayer->leftPalmPose = m_palmPoseActionSpaceStates[i].pose;
                    else
                        m_projectionLayer->rightPalmPose = m_palmPoseActionSpaceStates[i].pose;
                } else {
                    SPDLOG_LOGGER_ERROR(KDXr::Logger::logger(), "Failed to locate space for palm pose.");
                }
            }
        } else {
            SPDLOG_LOGGER_ERROR(KDXr::Logger::logger(), "Failed to get palm pose action state.");
        }
    }
}

void HelloXr::processHapticAction()
{
    // Apply any haptic feedback
    for (uint32_t i = 0; i < 2; ++i) {
        if (m_buzzAmplitudes[i] > 0.0f) {
            const auto buzzOptions = KDXr::VibrationOutputOptions{
                .action = m_buzzAction,
                .subactionPath = m_handPaths[i],
                .amplitude = m_buzzAmplitudes[i],
            };
            m_session.vibrateOutput(buzzOptions);

            // Decay the amplitude
            m_buzzAmplitudes[i] *= 0.5f;
            if (m_buzzAmplitudes[i] < 0.01f)
                m_buzzAmplitudes[i] = 0.0f;
        }
    }
}
