/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/vulkan/vulkan_graphics_api.h>
#include <KDGpu/surface_options.h>
class KDGpuRenderer
{
public:
    KDGpuRenderer(const KDGpu::SurfaceOptions &options, const std::string &pathToShaderFolder);

    void frame();
    void resize(uint32_t width, uint32_t height);

private:
    void createSwapchain();
    void createBuffers();
    void createPipeline();

    uint32_t _width{ 512 };
    uint32_t _height{ 512 };

    std::shared_ptr<spdlog::logger> _logger;
    std::string _pathToShaderFolder;
    std::unique_ptr<KDGpu::GraphicsApi> _api;
    KDGpu::Instance _instance;
    KDGpu::Surface _surface;
    KDGpu::Adapter *_adapter{};
    KDGpu::Device _device;
    KDGpu::Queue _queue;
    KDGpu::Swapchain _swapchain;
    std::vector<KDGpu::TextureView> _swapchainViews;
    KDGpu::Format _swapchainFormat;
    KDGpu::Texture _depthTexture;
    KDGpu::TextureView _depthTextureView;
    KDGpu::Format _depthTextureFormat;
    KDGpu::Buffer _vertexBuffer;
    KDGpu::Buffer _cameraUBOBuffer;
    KDGpu::ShaderModule _vertexShader;
    KDGpu::ShaderModule _fragmentShader;
    KDGpu::BindGroupLayout _bindGroupLayout;
    KDGpu::PipelineLayout _pipelineLayout;
    KDGpu::GraphicsPipeline _pipeline;
    KDGpu::BindGroup _bindGroup;
    KDGpu::GpuSemaphore _imageAvailableSemaphore;
    KDGpu::GpuSemaphore _renderCompleteSemaphore;
    KDGpu::Fence _frameInFlightFence;
};
