/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "openxr_api.h"

#if defined(PLATFORM_ANDROID)
#include <android_native_app_glue.h>
#endif

namespace KDXr {

OpenXrApi::OpenXrApi()
    : XrApi()
    , m_openxrResourceManager{ std::make_unique<OpenXrResourceManager>() }
{
    m_resourceManager = m_openxrResourceManager.get();
}

OpenXrApi::~OpenXrApi()
{
}

#if defined(PLATFORM_ANDROID)
void OpenXrApi::initializeAndroid(android_app *androidApp)
{
    PFN_xrInitializeLoaderKHR xrInitializeLoaderKHR;
    xrGetInstanceProcAddr(
            XR_NULL_HANDLE, "xrInitializeLoaderKHR", (PFN_xrVoidFunction *)&xrInitializeLoaderKHR);
    if (xrInitializeLoaderKHR != NULL) {
        XrLoaderInitInfoAndroidKHR loaderInitializeInfoAndroid;
        memset(&loaderInitializeInfoAndroid, 0, sizeof(loaderInitializeInfoAndroid));
        loaderInitializeInfoAndroid.type = XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR;
        loaderInitializeInfoAndroid.next = NULL;
        loaderInitializeInfoAndroid.applicationVM = androidApp->activity->vm;
        loaderInitializeInfoAndroid.applicationContext = androidApp->activity->clazz;
        xrInitializeLoaderKHR((XrLoaderInitInfoBaseHeaderKHR *)&loaderInitializeInfoAndroid);
    }
}
#endif

} // namespace KDXr
