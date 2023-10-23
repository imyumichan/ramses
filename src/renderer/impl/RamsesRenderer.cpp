//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/renderer/RamsesRenderer.h"
#include "impl/RamsesRendererImpl.h"
#include "impl/RamsesFrameworkImpl.h"
#include "impl/RamsesFrameworkTypesImpl.h"
#include "impl/APILoggingMacros.h"

namespace ramses
{
    RamsesRenderer::RamsesRenderer(std::unique_ptr<internal::RamsesRendererImpl> impl)
        : m_impl{ std::move(impl) }
    {
    }

    RamsesRenderer::~RamsesRenderer() = default;

    bool RamsesRenderer::doOneLoop()
    {
        const bool status = m_impl->doOneLoop();
        LOG_HL_RENDERER_API_NOARG(status);
        return status;
    }

    bool RamsesRenderer::startThread()
    {
        const bool status = m_impl->startThread();
        LOG_HL_RENDERER_API_NOARG(status);
        return status;
    }

    bool RamsesRenderer::stopThread()
    {
        const bool status = m_impl->stopThread();
        LOG_HL_RENDERER_API_NOARG(status);
        return status;
    }

    bool RamsesRenderer::isThreadRunning() const
    {
        return m_impl->isThreadRunning();
    }

    bool RamsesRenderer::setFramerateLimit(displayId_t displayId, float fpsLimit)
    {
        const bool status = m_impl->setFramerateLimit(displayId, fpsLimit);
        LOG_HL_RENDERER_API2(status, displayId, fpsLimit);
        return status;
    }

    float RamsesRenderer::getFramerateLimit(displayId_t displayId) const
    {
        return m_impl->getFramerateLimit(displayId);
    }

    bool RamsesRenderer::setLoopMode(ELoopMode loopMode)
    {
        const bool status = m_impl->setLoopMode(loopMode);
        LOG_HL_RENDERER_API1(status, loopMode);
        return status;
    }

    ELoopMode RamsesRenderer::getLoopMode() const
    {
        return m_impl->getLoopMode();
    }

    displayId_t RamsesRenderer::createDisplay(const DisplayConfig& config)
    {
        const displayId_t displayId = m_impl->createDisplay(config);
        LOG_HL_RENDERER_API1(displayId, LOG_API_GENERIC_OBJECT_STRING(config));
        return displayId;
    }

    bool RamsesRenderer::destroyDisplay(displayId_t displayId)
    {
        const bool status = m_impl->destroyDisplay(displayId);
        LOG_HL_RENDERER_API1(status, displayId);
        return status;
    }

    displayBufferId_t RamsesRenderer::getDisplayFramebuffer(displayId_t displayId) const
    {
        return m_impl->getDisplayFramebuffer(displayId);
    }

    RendererSceneControl* RamsesRenderer::getSceneControlAPI()
    {
        return m_impl->getSceneControlAPI();
    }

    bool RamsesRenderer::dispatchEvents(IRendererEventHandler& rendererEventHandler)
    {
        const bool status = m_impl->dispatchEvents(rendererEventHandler);
        LOG_HL_RENDERER_API1(status, LOG_API_GENERIC_OBJECT_STRING(rendererEventHandler));
        return status;
    }

    bool RamsesRenderer::setExternallyOwnedWindowSize(displayId_t display, uint32_t width, uint32_t height)
    {
        const bool status = m_impl->setExternallyOwnedWindowSize(display, width, height);
        LOG_HL_RENDERER_API3(status, display, width, height);
        return status;
    }

    displayBufferId_t RamsesRenderer::createOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height, uint32_t sampleCount, EDepthBufferType depthBufferType)
    {
        const displayBufferId_t bufferId = m_impl->createOffscreenBuffer(display, width, height, sampleCount, false, depthBufferType);
        LOG_HL_RENDERER_API5(bufferId, display, width, height, sampleCount, depthBufferType);
        return bufferId;
    }

    bool RamsesRenderer::destroyOffscreenBuffer(displayId_t display, displayBufferId_t offscreenBuffer)
    {
        const bool status = m_impl->destroyOffscreenBuffer(display, offscreenBuffer);
        LOG_HL_RENDERER_API2(status, display, offscreenBuffer);
        return status;
    }

    externalBufferId_t RamsesRenderer::createExternalBuffer(displayId_t display)
    {
        const auto bufferId = m_impl->createExternalBuffer(display);
        LOG_HL_RENDERER_API1(bufferId, display);
        return bufferId;
    }

    bool RamsesRenderer::destroyExternalBuffer(displayId_t display, externalBufferId_t externalBuffer)
    {
        const auto status = m_impl->destroyExternalBuffer(display, externalBuffer);
        LOG_HL_RENDERER_API2(status, display, externalBuffer);
        return status;
    }

    streamBufferId_t RamsesRenderer::createStreamBuffer(displayId_t display, waylandIviSurfaceId_t surfaceId)
    {
        return m_impl->createStreamBuffer(display, surfaceId);
    }

    bool RamsesRenderer::destroyStreamBuffer(displayId_t display, streamBufferId_t bufferId)
    {
        return m_impl->destroyStreamBuffer(display, bufferId);
    }

    bool RamsesRenderer::setDisplayBufferClearFlags(displayId_t display, displayBufferId_t displayBuffer, ClearFlags clearFlags)
    {
        const bool status = m_impl->setDisplayBufferClearFlags(display, displayBuffer, clearFlags);
        LOG_HL_RENDERER_API3(status, display, displayBuffer, clearFlags);
        return status;
    }

    bool RamsesRenderer::setDisplayBufferClearColor(displayId_t display, displayBufferId_t displayBuffer, const vec4f& color)
    {
        const bool status = m_impl->setDisplayBufferClearColor(display, displayBuffer, color);
        LOG_HL_RENDERER_API6(status, display, displayBuffer, color.r, color.g, color.b, color.a);
        return status;
    }

    bool RamsesRenderer::readPixels(displayId_t displayId, displayBufferId_t displayBuffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        const bool status = m_impl->readPixels(displayId, displayBuffer, x, y, width, height);
        LOG_HL_RENDERER_API6(status, displayId, displayBuffer, x, y, width, height);
        return status;
    }

    bool RamsesRenderer::flush()
    {
        const bool result = m_impl->flush();
        LOG_HL_RENDERER_API_NOARG(result);
        return result;
    }

    bool RamsesRenderer::setSurfaceVisibility(uint32_t surfaceId, bool visibility)
    {
        const bool status = m_impl->systemCompositorSetIviSurfaceVisibility(surfaceId, visibility);
        LOG_HL_RENDERER_API2(status, surfaceId, visibility);
        return status;
    }

    bool RamsesRenderer::setSurfaceOpacity(uint32_t surfaceId, float opacity)
    {
        const bool status = m_impl->systemCompositorSetIviSurfaceOpacity(surfaceId, opacity);
        LOG_HL_RENDERER_API2(status, surfaceId, opacity);
        return status;
    }

    bool RamsesRenderer::setSurfaceRectangle(uint32_t surfaceId, int32_t x, int32_t y, int32_t width, int32_t height)
    {
        const bool status = m_impl->systemCompositorSetIviSurfaceRectangle(surfaceId, x, y, width, height);
        LOG_HL_RENDERER_API5(status, surfaceId, x, y, width, height);
        return status;
    }

    bool RamsesRenderer::setLayerVisibility(uint32_t layerId, bool visibility)
    {
        const bool status = m_impl->systemCompositorSetIviLayerVisibility(layerId, visibility);
        LOG_HL_RENDERER_API2(status, layerId, visibility);
        return status;
    }

    bool RamsesRenderer::takeSystemCompositorScreenshot(std::string_view fileName, int32_t screenIviId)
    {
        const bool status = m_impl->systemCompositorTakeScreenshot(fileName, screenIviId);
        LOG_HL_RENDERER_API2(status, fileName, screenIviId);
        return status;
    }

    bool RamsesRenderer::logRendererInfo()
    {
        const bool status = m_impl->logRendererInfo();
        LOG_HL_RENDERER_API_NOARG(status);
        return status;
    }

    bool RamsesRenderer::setFrameTimerLimits(uint64_t limitForSceneResourcesUpload, uint64_t limitForClientResourcesUpload, uint64_t limitForOffscreenBufferRender)
    {
        const bool status = m_impl->setFrameTimerLimits(limitForSceneResourcesUpload, limitForClientResourcesUpload, limitForOffscreenBufferRender);
        LOG_HL_RENDERER_API3(status, limitForSceneResourcesUpload, limitForClientResourcesUpload, limitForOffscreenBufferRender);
        return status;
    }

    bool RamsesRenderer::setPendingFlushLimits(uint32_t forceApplyFlushLimit, uint32_t forceUnsubscribeSceneLimit)
    {
        const bool status = m_impl->setPendingFlushLimits(forceApplyFlushLimit, forceUnsubscribeSceneLimit);
        LOG_HL_RENDERER_API2(status, forceApplyFlushLimit, forceUnsubscribeSceneLimit);
        return status;
    }

    displayBufferId_t RamsesRenderer::createInterruptibleOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height, EDepthBufferType depthBufferType)
    {
        const auto bufferId = m_impl->createOffscreenBuffer(display, width, height, 0u, true, depthBufferType);
        LOG_HL_RENDERER_API4(bufferId, display, width, height, depthBufferType);
        return bufferId;
    }

    displayBufferId_t RamsesRenderer::createDmaOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height, uint32_t bufferFourccFormat, uint32_t usageFlags, uint64_t modifier)
    {
        ramses::internal::DmaBufferFourccFormat bufferFormat { bufferFourccFormat };
        ramses::internal::DmaBufferUsageFlags bufferUsage { usageFlags };
        ramses::internal::DmaBufferModifiers bufferModifer { modifier };
        const auto bufferId = m_impl->createDmaOffscreenBuffer(display, width, height, bufferFormat, bufferUsage, bufferModifer);
        LOG_HL_RENDERER_API6(bufferId, display, width, height, bufferFourccFormat, usageFlags, modifier);
        return bufferId;
    }

    bool RamsesRenderer::getDmaOffscreenBufferFDAndStride(displayId_t display, displayBufferId_t displayBufferId, int& fd, uint32_t& stride) const
    {
        return m_impl->getDmaOffscreenBufferFDAndStride(display, displayBufferId, fd, stride);
    }

    bool RamsesRenderer::setSkippingOfUnmodifiedBuffers(bool enable)
    {
        const bool status = m_impl->setSkippingOfUnmodifiedBuffers(enable);
        LOG_HL_RENDERER_API1(status, enable);
        return status;
    }

    internal::RamsesRendererImpl& RamsesRenderer::impl()
    {
        return *m_impl;
    }

    const internal::RamsesRendererImpl& RamsesRenderer::impl() const
    {
        return *m_impl;
    }
}
