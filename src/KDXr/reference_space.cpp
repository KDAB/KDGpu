/*
  This file is part of KDXr.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "reference_space.h"

#include <KDXr/xr_api.h>
#include <KDXr/resource_manager.h>
#include <KDXr/api/api_reference_space.h>

#include <KDXr/utils/logging.h>

namespace KDXr {

/**
    @class ReferenceSpace
    @brief ReferenceSpace is used to initialize the XR API.
    @ingroup public
    @headerfile reference_space.h <KDXr/reference_space.h>

    @sa ReferenceSpaceOptions
 */

/**
    @fn ReferenceSpace::handle()
    @brief Returns the handle used to retrieve the underlying XR API specific ReferenceSpace

    @return KDGpu::Handle<ReferenceSpace_t>
    @sa ResourceManager
 */

/**
    @fn ReferenceSpace::isValid()
    @brief Convenience function to check whether the object is actually referencing a valid API specific resource
 */

ReferenceSpace::ReferenceSpace()
{
}

ReferenceSpace::ReferenceSpace(const KDGpu::Handle<Session_t> &sessionHandle, XrApi *api, const ReferenceSpaceOptions &options)
    : m_api(api)
    , m_sessionHandle(sessionHandle)
{
    // Create an ReferenceSpace using the underlying API
    m_referenceSpace = m_api->resourceManager()->createReferenceSpace(m_sessionHandle, options);
}

ReferenceSpace::ReferenceSpace(const KDGpu::Handle<Session_t> &sessionHandle, XrApi *api, const ActionSpaceOptions &options)
    : m_api(api)
    , m_sessionHandle(sessionHandle)
{
    // Create an ReferenceSpace using the underlying API
    m_referenceSpace = m_api->resourceManager()->createReferenceSpace(m_sessionHandle, options);
}

ReferenceSpace::~ReferenceSpace()
{
    if (isValid())
        m_api->resourceManager()->deleteReferenceSpace(handle());
}

ReferenceSpace::ReferenceSpace(ReferenceSpace &&other)
{
    m_api = std::exchange(other.m_api, nullptr);
    m_sessionHandle = std::exchange(other.m_sessionHandle, {});
    m_referenceSpace = std::exchange(other.m_referenceSpace, {});
}

ReferenceSpace &ReferenceSpace::operator=(ReferenceSpace &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteReferenceSpace(handle());

        m_api = std::exchange(other.m_api, nullptr);
        m_sessionHandle = std::exchange(other.m_sessionHandle, {});
        m_referenceSpace = std::exchange(other.m_referenceSpace, {});
    }
    return *this;
}

LocateSpaceResult ReferenceSpace::locateSpace(const LocateSpaceOptions &options, SpaceState &state) const
{
    auto apiSpace = m_api->resourceManager()->getReferenceSpace(m_referenceSpace);
    return apiSpace->locateSpace(options, state);
}

} // namespace KDXr
