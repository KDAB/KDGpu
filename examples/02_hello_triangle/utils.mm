/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGui/window.h>
#include <KDGui/platform/cocoa/cocoa_platform_window.h>
#include <KDFoundation/logging.h>

#import <QuartzCore/CAMetalLayer.h>
#import <Cocoa/Cocoa.h>

CAMetalLayer *createMetalLayer(KDGui::Window *window)
{
    CAMetalLayer *layer = [CAMetalLayer layer];
    if (layer == nil) {
        spdlog::critical("Failed to create Metal layer");
        return nullptr;
    }
    auto cocoaWindow = dynamic_cast<KDGui::CocoaPlatformWindow *>(window->platformWindow());
    NSView *view = ((NSWindow *)cocoaWindow->nativeWindow()).contentView;
    [view setWantsLayer:YES];
    [view setLayer:layer];

    return layer;
}
