/*
See LICENSES/Apple-Sample-Code.txt for the full license text.

Abstract:
Implementation of renderer class which performs Metal setup and per-frame rendering
*/

/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#import "AAPLRenderer.h"
#include "KDGpuRenderer.h"

#include <simd/simd.h>

@implementation AAPLRenderer {
    // renderer global ivars
    CAMetalLayer *_layer;
    vector_uint2 _viewportSize;
    KDGpuRenderer *_renderer;
}

- (nonnull instancetype)initWithMetalLayer:(nonnull CAMetalLayer *)layer
{
    self = [super init];
    if (self) {
        _layer = layer;
        KDGpu::SurfaceOptions options;
        options.layer = layer;

        NSBundle *mainBundle = [NSBundle mainBundle];

        // Get the path to the Resources folder
        NSString *resourcesPath = [mainBundle resourcePath];
        const char *cResourcesPath = [resourcesPath cStringUsingEncoding:NSUTF8StringEncoding];

        _renderer = new KDGpuRenderer(options, cResourcesPath);
    }
    return self;
}

- (void)render
{
    _renderer->frame();
}

- (void)drawableResize:(CGSize)drawableSize
{
    _viewportSize.x = drawableSize.width;
    _viewportSize.y = drawableSize.height;
    _renderer->resize(_viewportSize.x, _viewportSize.y);
}

@end
