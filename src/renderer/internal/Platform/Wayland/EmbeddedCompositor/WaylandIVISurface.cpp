//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandIVISurface.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/EmbeddedCompositor_Wayland.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandClient.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/INativeWaylandResource.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandSurface.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandClient.h"
#include "internal/Core/Utils/ThreadLocalLogForced.h"

namespace ramses::internal
{
    WaylandIVISurface::WaylandIVISurface(IWaylandClient&             client,
                                         INativeWaylandResource&     iviApplicationConnectionResource,
                                         WaylandIviSurfaceId         iviSurfaceId,
                                         IWaylandSurface*            surface,
                                         uint32_t                    id,
                                         EmbeddedCompositor_Wayland& compositor)
        : m_iviSurfaceId(iviSurfaceId)
        , m_compositor(compositor)
        , m_clientCredentials(client.getCredentials())
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandIVISurface::WaylandIVISurface");

        if (surface->hasIviSurface())
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandIVISurface::WaylandIVISurface: failed creating: " << iviSurfaceId
                    << ". The wayland surface already has a surface with " << surface->getIviSurfaceId() << "attached!  " << m_clientCredentials);

            iviApplicationConnectionResource.postError(IVI_APPLICATION_ERROR_IVI_ID, "surface already has a ivi-surface");
        }
        else
        {
            if (!m_compositor.hasSurfaceForStreamTexture(iviSurfaceId))
            {
                m_resource = client.resourceCreate(&ivi_surface_interface, iviApplicationConnectionResource.getVersion(), id);
                if (nullptr != m_resource)
                {
                    LOG_INFO(CONTEXT_RENDERER, "WaylandIVISurface::WaylandIVISurface: created successfully: " << iviSurfaceId
                            << "  " << m_clientCredentials);

                    m_resource->setImplementation(&m_iviSurfaceInterface, this, ResourceDestroyedCallback);
                    m_surface = surface;
                    m_surface->setIviSurface(this);

                    if (m_surface->getWaylandBuffer() != nullptr)
                    {
                        m_compositor.addToUpdatedStreamTextureSourceIds(iviSurfaceId);
                    }
                }
                else
                {
                    LOG_ERROR(CONTEXT_RENDERER, "WaylandIVISurface::WaylandIVISurface: failed creating " << iviSurfaceId
                            << ". Failed creating wayland resource  " << m_clientCredentials);
                    client.postNoMemory();
                }
            }
            else
            {

                const auto credentialsForOtherClient = m_compositor.findSurfaceForStreamTexture(iviSurfaceId).getClientCredentials();

                LOG_ERROR(CONTEXT_RENDERER, "WaylandIVISurface::WaylandIVISurface: failed creating " << iviSurfaceId
                        << " for  " << m_clientCredentials
                        << ". A wayland surface already exists with same ivi-surface id for " << credentialsForOtherClient);

                iviApplicationConnectionResource.postError(IVI_APPLICATION_ERROR_IVI_ID, "ivi-id is already in use");
            }
        }
    }

    bool WaylandIVISurface::wasSuccessfullyInitialized() const
    {
        return nullptr != m_resource;
    }

    WaylandIVISurface::~WaylandIVISurface()
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandIVISurface::~WaylandIVISurface " << m_iviSurfaceId
                << "  " << m_clientCredentials);

        if (m_surface != nullptr)
        {
            m_surface->setIviSurface(nullptr);
            m_compositor.removeFromUpdatedStreamTextureSourceIds(m_iviSurfaceId);
        }

        if (nullptr != m_resource)
        {
            // Remove ResourceDestroyedCallback
            m_resource->setImplementation(&m_iviSurfaceInterface, this, nullptr);
            m_resource->destroy();
            delete m_resource;
        }
    }

    void WaylandIVISurface::surfaceWasDeleted()
    {
        m_surface = nullptr;
        m_compositor.removeFromUpdatedStreamTextureSourceIds(m_iviSurfaceId);
    }

    void WaylandIVISurface::bufferWasSetToSurface(IWaylandBuffer* buffer)
    {
        if (buffer != nullptr)
        {
            m_compositor.addToUpdatedStreamTextureSourceIds(m_iviSurfaceId);
        }
        else
        {
            m_compositor.removeFromUpdatedStreamTextureSourceIds(m_iviSurfaceId);
        }
    }

    void WaylandIVISurface::resourceDestroyed()
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandIVISurface::resourceDestroyed " << m_iviSurfaceId);

        // wl_resource is destroyed outside by the Wayland library, so m_resource looses the ownership of the
        // Wayland resource, so that we don't call wl_resource_destroy.
        delete m_resource;
        m_resource = nullptr;
    }


    void WaylandIVISurface::IVISurfaceDestroyCallback([[maybe_unused]] wl_client* client, wl_resource* iviSurfaceResource)
    {
        auto* iviSurface = static_cast<WaylandIVISurface*>(wl_resource_get_user_data(iviSurfaceResource));
        delete iviSurface;
    }

    void WaylandIVISurface::ResourceDestroyedCallback(wl_resource* iviSurfaceResource)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandIVISurface::IVISurfaceDestroyedCallback");

        auto* iviSurface = static_cast<WaylandIVISurface*>(wl_resource_get_user_data(iviSurfaceResource));
        iviSurface->resourceDestroyed();
        delete iviSurface;
    }

    WaylandIviSurfaceId WaylandIVISurface::getIviId() const
    {
        return m_iviSurfaceId;
    }
};
