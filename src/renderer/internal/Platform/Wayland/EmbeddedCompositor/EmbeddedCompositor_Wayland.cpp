//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Wayland/EmbeddedCompositor/EmbeddedCompositor_Wayland.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandCompositorConnection.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandSurface.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandBuffer.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandBufferResource.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/LinuxDmabufGlobal.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/LinuxDmabufBuffer.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/TextureUploadingAdapter_Wayland.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/LinuxDmabuf.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandOutputParams.h"
#include "internal/Platform/EGL/Context_EGL.h"
#include "internal/RendererLib/DisplayConfig.h"
#include "internal/RendererLib/RendererLogContext.h"
#include "internal/Core/Utils/ThreadLocalLogForced.h"
#include "internal/Core/Utils/Warnings.h"
#include "internal/PlatformAbstraction/PlatformTime.h"
#include <algorithm>
#include <unistd.h>

namespace ramses::internal
{
    EmbeddedCompositor_Wayland::EmbeddedCompositor_Wayland(const DisplayConfig& displayConfig, Context_EGL &context)
        : m_waylandEmbeddedSocketName(displayConfig.getWaylandSocketEmbedded())
        , m_waylandEmbeddedSocketGroup(displayConfig.getWaylandSocketEmbeddedGroup())
        , m_waylandEmbeddedSocketPermissions(displayConfig.getWaylandSocketEmbeddedPermissions())
        , m_waylandEmbeddedSocketFD(displayConfig.getWaylandSocketEmbeddedFD())
        , m_context(context)
        , m_compositorGlobal(*this)
        , m_waylandOutputGlobal({ displayConfig.getDesiredWindowWidth(), displayConfig.getDesiredWindowHeight() })
        , m_iviApplicationGlobal(*this)
        , m_linuxDmabufGlobal(*this)
    {
        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::EmbeddedCompositor_Wayland(): Created EmbeddedCompositor_Wayland...(not initialized yet)");
    }

    EmbeddedCompositor_Wayland::~EmbeddedCompositor_Wayland()
    {
        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::~EmbeddedCompositor_Wayland(): Destroying EmbeddedCompositor_Wayland");

        m_compositorGlobal.destroy();
        m_shellGlobal.destroy();
        m_waylandOutputGlobal.destroy();
        m_iviApplicationGlobal.destroy();
        m_linuxDmabufGlobal.destroy();
    }

    bool EmbeddedCompositor_Wayland::init()
    {
        if (!m_serverDisplay.init(m_waylandEmbeddedSocketName, m_waylandEmbeddedSocketGroup, m_waylandEmbeddedSocketPermissions, m_waylandEmbeddedSocketFD))
        {
            return false;
        }

        if (!m_compositorGlobal.init(m_serverDisplay))
        {
            return false;
        }

        if (!m_shellGlobal.init(m_serverDisplay))
        {
            return false;
        }

        if(!m_waylandOutputGlobal.init(m_serverDisplay))
            return false;

        if (!m_iviApplicationGlobal.init(m_serverDisplay))
        {
            return false;
        }

        // Not all EGL implementations support the extensions necessary for dmabuf import
        if (!m_linuxDmabufGlobal.init(m_serverDisplay, m_context))
        {
            LOG_WARN(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::init(): EGL_EXT_image_dma_buf_import not supported, skipping zwp_linux_dmabuf_v1.");
        }

        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::init(): Embedded compositor created successfully!");

        return true;
    }

    wl_display* EmbeddedCompositor_Wayland::getEmbeddedCompositingDisplay() const
    {
        return m_serverDisplay.get();
    }

    void EmbeddedCompositor_Wayland::handleRequestsFromClients()
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::handleRequestsFromClients(): handling pending events and requests from clients");

        m_serverDisplay.dispatchEventLoop();
    }

    bool EmbeddedCompositor_Wayland::hasUpdatedStreamTextureSources() const
    {
        return !m_updatedStreamTextureSourceIds.empty();
    }

    WaylandIviSurfaceIdSet EmbeddedCompositor_Wayland::dispatchUpdatedStreamTextureSourceIds()
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::dispatchUpdatedStreamTextureSourceIds(): count of pending updates for dispatching :" << m_updatedStreamTextureSourceIds.size());
        WaylandIviSurfaceIdSet result = m_updatedStreamTextureSourceIds;
        m_updatedStreamTextureSourceIds.clear();
        return result;
    }

    WaylandIviSurfaceIdSet EmbeddedCompositor_Wayland::dispatchNewStreamTextureSourceIds()
    {
        auto result = m_newStreamTextureSourceIds;
        m_newStreamTextureSourceIds.clear();
        return result;
    }

    WaylandIviSurfaceIdSet EmbeddedCompositor_Wayland::dispatchObsoleteStreamTextureSourceIds()
    {
        auto result = m_obsoleteStreamTextureSourceIds;
        m_obsoleteStreamTextureSourceIds.clear();
        return result;
    }

    void EmbeddedCompositor_Wayland::logInfos(RendererLogContext& context) const
    {
        WaylandEGLExtensionProcs eglExt(m_context.getEglDisplay());
        context << m_surfaces.size() << " wayland surface(s)" << RendererLogContext::NewLine;
        context.indent();
        for (auto surface: m_surfaces)
        {
            surface->logInfos(context, eglExt);
        }
        context.unindent();
        context << m_waylandBuffers.size() << " wayland buffer(s)" << RendererLogContext::NewLine;
        context.indent();
        for (auto* buffer: m_waylandBuffers)
        {
            buffer->logInfos(context, eglExt);
        }
        context.unindent();
        context << m_regions.size() << " wayland region(s) - [not supported]" << RendererLogContext::NewLine;
    }

    void EmbeddedCompositor_Wayland::logPeriodicInfo(StringOutputStream& sos) const
    {
        sos << "EC: ";
        for (auto surface: m_surfaces)
        {
            auto* buf = surface->getWaylandBuffer();
            auto* res = (buf != nullptr) ? static_cast<const void*>(buf->getResource().getLowLevelHandle()) : nullptr;
            sos << surface->getIviSurfaceId().getValue() << "[pid:" << surface->getClientCredentials().getProcessId() << " res:" << res << "];";
        }
        sos << "\n";
    }

    void EmbeddedCompositor_Wayland::addWaylandSurface(IWaylandSurface& waylandSurface)
    {
        m_surfaces.push_back(&waylandSurface);
        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::addWaylandSurface: Client created surface. Count surfaces :" << m_surfaces.size());
    }

    void EmbeddedCompositor_Wayland::removeWaylandSurface(IWaylandSurface& waylandSurface)
    {
        LOG_INFO(CONTEXT_SMOKETEST, "embedded-compositing client surface destroyed");
        // It's safe to call remove even if surface has not been mapped and
        // therefore not been added into any list, since link got initialized at
        // construction in compositor_create_surface().
        for(auto surface = m_surfaces.begin(); surface != m_surfaces.end(); ++surface)
        {
            if(*surface == &waylandSurface)
            {
                m_surfaces.erase(surface);
                break;
            }
        }

        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::removeWaylandSurface() Client destroyed surface, showing fallback texture for " << waylandSurface.getIviSurfaceId()
                 << ". Count surfaces :" << m_surfaces.size());
    }

    void EmbeddedCompositor_Wayland::addWaylandCompositorConnection(IWaylandCompositorConnection& waylandCompositorConnection)
    {
        m_compositorConnections.put(&waylandCompositorConnection);
        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::addWaylandCompositorConnection: embedded-compositing connection created. Count connections :" << m_compositorConnections.size());
    }

    IWaylandSurface* EmbeddedCompositor_Wayland::findWaylandSurfaceByIviSurfaceId(WaylandIviSurfaceId iviSurfaceId) const
    {
        for (auto surface: m_surfaces)
        {
            if (surface->getIviSurfaceId() == iviSurfaceId)
            {
                return surface;
            }
        }
        return nullptr;
    }

    void EmbeddedCompositor_Wayland::endFrame(bool notifyClients)
    {
        if (notifyClients)
        {
            LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::endFrame(): will send surface frame callbacks to clients");
            const auto time = static_cast<uint32_t>(PlatformTime::GetMillisecondsAbsolute());

            for (auto surface: m_surfaces)
            {
                surface->sendFrameCallbacks(time);
                surface->resetNumberOfCommitedFrames();
            }
        }

        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::endFrame(): flusing clients");

        m_serverDisplay.flushClients();
    }

    uint32_t EmbeddedCompositor_Wayland::uploadCompositingContentForStreamTexture(WaylandIviSurfaceId streamTextureSourceId, DeviceResourceHandle textureHandle, ITextureUploadingAdapter& textureUploadingAdapter)
    {
        assert(streamTextureSourceId.isValid());
        IWaylandSurface* waylandClientSurface = findWaylandSurfaceByIviSurfaceId(streamTextureSourceId);
        assert(nullptr != waylandClientSurface);

        LOG_DEBUG(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::uploadCompositingContentForStreamTexture() " << streamTextureSourceId);
        LOG_INFO(CONTEXT_SMOKETEST, "embedded-compositing client surface found for existing streamtexture: " << streamTextureSourceId);

        UploadCompositingContentForWaylandSurface(waylandClientSurface, textureHandle, textureUploadingAdapter);
        return waylandClientSurface->getNumberOfCommitedFrames();
    }

    void EmbeddedCompositor_Wayland::UploadCompositingContentForWaylandSurface(IWaylandSurface* waylandSurface, DeviceResourceHandle textureHandle, ITextureUploadingAdapter& textureUploadingAdapter)
    {
        IWaylandBuffer* waylandBuffer = waylandSurface->getWaylandBuffer();
        assert(nullptr != waylandBuffer);

        WaylandBufferResource& waylandBufferResource = waylandBuffer->getResource();

        const auto* sharedMemoryBufferData = static_cast<const std::byte*>(waylandBufferResource.bufferGetSharedMemoryData());
        LinuxDmabufBufferData* linuxDmabufBuffer = LinuxDmabufBuffer::fromWaylandBufferResource(waylandBufferResource);

        const bool surfaceBufferTypeChanged = waylandSurface->dispatchBufferTypeChanged();

        if(surfaceBufferTypeChanged)
        {
            //reset swizzle
            const TextureSwizzleArray swizzle = {ETextureChannelColor::Red, ETextureChannelColor::Green, ETextureChannelColor::Blue, ETextureChannelColor::Alpha};
            std::byte dummyData { 0u };
            textureUploadingAdapter.uploadTexture2D(textureHandle, 1u, 1u, EPixelStorageFormat::R8, &dummyData, swizzle);

            LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::uploadCompositingContentForWaylandSurface(): resetting swizzle for " << waylandSurface->getIviSurfaceId());
        }

        if (nullptr != sharedMemoryBufferData)
        {
            const TextureSwizzleArray swizzle = {ETextureChannelColor::Blue, ETextureChannelColor::Green, ETextureChannelColor::Red, ETextureChannelColor::Alpha};
            textureUploadingAdapter.uploadTexture2D(textureHandle, waylandBufferResource.getWidth(), waylandBufferResource.getHeight(), EPixelStorageFormat::RGBA8, sharedMemoryBufferData, swizzle);
        }
        else if (nullptr != linuxDmabufBuffer)
        {
            const auto success = static_cast<TextureUploadingAdapter_Wayland&>(textureUploadingAdapter).uploadTextureFromLinuxDmabuf(textureHandle, linuxDmabufBuffer);

            if(!success)
            {
                StringOutputStream message;
                message << "Failed creating EGL image from dma buffer for " << waylandSurface->getIviSurfaceId();

                waylandBufferResource.postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INVALID_WL_BUFFER, message.release());
            }
        }
        else
        {
            static_cast<TextureUploadingAdapter_Wayland&>(textureUploadingAdapter).uploadTextureFromWaylandResource(textureHandle, waylandBufferResource.getLowLevelHandle());
        }
    }

    bool EmbeddedCompositor_Wayland::isContentAvailableForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const
    {
        const IWaylandSurface* waylandClientSurface = findWaylandSurfaceByIviSurfaceId(streamTextureSourceId);
        if(waylandClientSurface)
        {
            return nullptr != waylandClientSurface->getWaylandBuffer();
        }

        return false;
    }

    uint64_t EmbeddedCompositor_Wayland::getNumberOfCommitedFramesForWaylandIviSurfaceSinceBeginningOfTime(WaylandIviSurfaceId waylandSurfaceId) const
    {
        const IWaylandSurface* waylandClientSurface = findWaylandSurfaceByIviSurfaceId(waylandSurfaceId);
        if (waylandClientSurface)
        {
            return waylandClientSurface->getNumberOfCommitedFramesSinceBeginningOfTime();
        }
        return 0;
    }

    bool EmbeddedCompositor_Wayland::isBufferAttachedToWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const
    {
        const IWaylandSurface* waylandClientSurface = findWaylandSurfaceByIviSurfaceId(waylandSurfaceId);
        if (waylandClientSurface)
        {
            return waylandClientSurface->hasPendingBuffer() || waylandClientSurface->getWaylandBuffer() != nullptr;
        }
        return false;
    }

    uint32_t EmbeddedCompositor_Wayland::getNumberOfCompositorConnections() const
    {
        return m_compositorConnections.size();
    }

    bool EmbeddedCompositor_Wayland::hasSurfaceForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const
    {
        for (const auto surface: m_surfaces)
        {
            if (surface->getIviSurfaceId() == streamTextureSourceId)
            {
                return true;
            }
        }

        return false;
    }

    const IWaylandSurface& EmbeddedCompositor_Wayland::findSurfaceForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const
    {
        const auto it = std::find_if(std::cbegin(m_surfaces), std::cend(m_surfaces), [&](const auto surface){ return surface->getIviSurfaceId() == streamTextureSourceId;});
        return **it;
    }

    std::string EmbeddedCompositor_Wayland::getTitleOfWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const
    {
        const IWaylandSurface* waylandClientSurface = findWaylandSurfaceByIviSurfaceId(waylandSurfaceId);
        if (waylandClientSurface)
        {
            return waylandClientSurface->getSurfaceTitle();
        }
        return {};
    }

    IWaylandBuffer* EmbeddedCompositor_Wayland::findWaylandBuffer(WaylandBufferResource& bufferResource)
    {
        for (auto i: m_waylandBuffers)
        {
            if (i->getResource().getLowLevelHandle() == bufferResource.getLowLevelHandle())
            {
                return i;
            }
        }
        return nullptr;
    }

    IWaylandBuffer& EmbeddedCompositor_Wayland::getOrCreateBuffer(WaylandBufferResource& bufferResource)
    {
        IWaylandBuffer* buffer = findWaylandBuffer(bufferResource);
        if (nullptr == buffer)
        {
            buffer = new WaylandBuffer(bufferResource, *this);
            m_waylandBuffers.put(buffer);
        }
        return *buffer;
    }

    bool EmbeddedCompositor_Wayland::isRealCompositor() const
    {
        return true;
    }

    void EmbeddedCompositor_Wayland::handleBufferDestroyed(IWaylandBuffer& buffer)
    {
        if (!m_waylandBuffers.remove(&buffer))
        {
            LOG_ERROR(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::handleBufferDestroyed m_waylandBuffers.remove failed");
            assert(false);
        }

        for (auto surface: m_surfaces)
        {
            surface->bufferDestroyed(buffer);
        }
    }

    void EmbeddedCompositor_Wayland::removeWaylandCompositorConnection(IWaylandCompositorConnection& waylandCompositorConnection)
    {
        const bool removed = m_compositorConnections.remove(&waylandCompositorConnection);
        assert(removed);

        LOG_INFO(CONTEXT_SMOKETEST, "EmbeddedCompositor_Wayland::removeWaylandCompositorConnection: embedded-compositing connection removed. Count connections :"
                 << m_compositorConnections.size() << ", was removed " << removed);
    }

    void EmbeddedCompositor_Wayland::removeFromUpdatedStreamTextureSourceIds(WaylandIviSurfaceId id)
    {
        if (m_newStreamTextureSourceIds.count(id) != 0u)
        {
            m_newStreamTextureSourceIds.erase(id);
        }
        else if (m_knownStreamTextureSoruceIds.count(id) != 0u)
        {
            m_obsoleteStreamTextureSourceIds.insert(id);
        }

        m_updatedStreamTextureSourceIds.erase(id);
        m_knownStreamTextureSoruceIds.erase(id);
    }

    void EmbeddedCompositor_Wayland::addToUpdatedStreamTextureSourceIds(WaylandIviSurfaceId id)
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::addToUpdatedStreamTextureSourceIds: new texture data for stream texture with " << id);
        m_updatedStreamTextureSourceIds.insert(id);

        if (m_knownStreamTextureSoruceIds.count(id) == 0u)
        {
            m_newStreamTextureSourceIds.insert(id);
            m_knownStreamTextureSoruceIds.insert(id);
        }
    }

    void EmbeddedCompositor_Wayland::addWaylandRegion(IWaylandRegion& waylandRegion)
    {
        m_regions.put(&waylandRegion);
    }

    void EmbeddedCompositor_Wayland::removeWaylandRegion(IWaylandRegion& waylandRegion)
    {
        [[maybe_unused]] const bool removed = m_regions.remove(&waylandRegion);
        assert(removed);
    }
}
