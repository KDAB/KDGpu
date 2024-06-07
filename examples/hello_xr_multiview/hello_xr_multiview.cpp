/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "hello_xr_multiview.h"
#include "projection_layer.h"

#include <KDGpuExample/engine.h>
#include <KDGpuExample/xr_compositor/xr_cylinder_imgui_layer.h>
#include <KDGpuExample/xr_compositor/xr_quad_imgui_layer.h>

#include <KDXr/session.h>

void HelloXrMultiview::onAttached()
{
    XrExampleEngineLayer::onAttached();
    if (!m_isInitialized)
        return;

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
    m_toggleRotateYAction = m_actionSet.createAction({ .name = "rotatey",
                                                       .localizedName = "RotateY",
                                                       .type = KDXr::ActionType::BooleanInput,
                                                       .subactionPaths = m_handPaths });
    m_toggleRotateZAction = m_actionSet.createAction({ .name = "rotatez",
                                                       .localizedName = "RotateZ",
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
                { .action = m_toggleRotateYAction, .binding = "/user/hand/right/input/b/click" },
                { .action = m_toggleRotateYAction, .binding = "/user/hand/left/input/y/click" },
                { .action = m_toggleRotateZAction, .binding = "/user/hand/left/input/x/click" },
                { .action = m_toggleRotateZAction, .binding = "/user/hand/right/input/a/click" },
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

void HelloXrMultiview::onDetached()
{
    m_palmPoseActionSpaces[0] = {};
    m_palmPoseActionSpaces[1] = {};

    m_buzzAction = {};
    m_palmPoseAction = {};
    m_translateAction = {};
    m_scaleAction = {};
    m_toggleRotateZAction = {};
    m_toggleRotateYAction = {};
    m_actionSet = {};

    clearCompositorLayers();
    m_quadImguiLayer = nullptr;
    m_projectionLayer = nullptr;
    XrExampleEngineLayer::onDetached();
}

void HelloXrMultiview::onInteractionProfileChanged()
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

void HelloXrMultiview::pollActions(KDXr::Time predictedDisplayTime)
{
    // Sync the action set
    const auto syncActionOptions = KDXr::SyncActionsOptions{ .actionSets = { { m_actionSet } } };
    const auto syncActionResult = m_session.syncActions(syncActionOptions);
    if (syncActionResult != KDXr::SyncActionsResult::Success) {
        SPDLOG_LOGGER_ERROR(m_logger, "Failed to sync action set.");
        return;
    }

    // Poll the actions and do something with the results
    processToggleRotateZAction();
    processToggleRotateYAction();
    processScaleAction();
    processTranslateAction();
    processPalmPoseAction(predictedDisplayTime);
    processHapticAction();
}

void HelloXrMultiview::processToggleRotateZAction()
{
    bool toggleAnimation{ false };
    for (uint32_t i = 0; i < 2; ++i) {
        // Query the toggle animation action
        const auto toggleAnimationResult = m_session.getBooleanState(
                { .action = m_toggleRotateZAction, .subactionPath = m_handPaths[i] },
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
            SPDLOG_LOGGER_ERROR(m_logger, "Failed to get toggle animation action state.");
        }
    }

    // If the toggle animation action was pressed, toggle the animation and buzz the controller
    if (toggleAnimation) {
        m_projectionLayer->rotateZ = !m_projectionLayer->rotateZ();
        m_buzzAmplitudes[m_buzzHand] = 1.0f;
        SPDLOG_LOGGER_INFO(m_logger, "Animation enabled = {}", m_projectionLayer->rotateZ());
    }
}
void HelloXrMultiview::processToggleRotateYAction()
{
    bool toggleAnimation{ false };
    for (uint32_t i = 0; i < 2; ++i) {
        // Query the toggle animation action
        const auto toggleAnimationResult = m_session.getBooleanState(
                { .action = m_toggleRotateYAction, .subactionPath = m_handPaths[i] },
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
            SPDLOG_LOGGER_ERROR(m_logger, "Failed to get toggle animation action state.");
        }
    }

    // If the toggle animation action was pressed, toggle the animation and rotate the triangle
    if (toggleAnimation) {
        m_projectionLayer->rotateY = !m_projectionLayer->rotateY();
        m_buzzAmplitudes[m_buzzHand] = 1.0f;
        SPDLOG_LOGGER_INFO(m_logger, "Animation enabled = {}", m_projectionLayer->rotateY());
    }
}

void HelloXrMultiview::processScaleAction()
{
    // Query the scale action from the left trigger value
    float scale = 1.0f;
    const auto scaleResult = m_session.getFloatState({ .action = m_scaleAction, .subactionPath = m_handPaths[0] }, m_scaleActionState);
    if (scaleResult == KDXr::GetActionStateResult::Success) {
        if (m_scaleActionState.active)
            scale = 1.0 + powf(m_scaleActionState.currentState, 2.0f);
        m_projectionLayer->scale = scale;
    } else {
        SPDLOG_LOGGER_ERROR(m_logger, "Failed to get scale action state.");
    }
}

void HelloXrMultiview::processTranslateAction()
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
        SPDLOG_LOGGER_ERROR(m_logger, "Failed to get translate action state.");
    }
}

void HelloXrMultiview::processPalmPoseAction(KDXr::Time predictedDisplayTime)
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
                    SPDLOG_LOGGER_ERROR(m_logger, "Failed to locate space for palm pose.");
                }
            }
        } else {
            SPDLOG_LOGGER_ERROR(m_logger, "Failed to get palm pose action state.");
        }
    }
}

void HelloXrMultiview::processHapticAction()
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
