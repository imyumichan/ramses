//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/Renderer.h"
#include "RendererAPI/IRenderBackend.h"
#include "RendererAPI/ISystemCompositorController.h"
#include "RendererAPI/IEmbeddedCompositor.h"
#include "RendererAPI/ISurface.h"
#include "RendererAPI/IWindow.h"
#include "RendererAPI/IDevice.h"
#include "RendererAPI/IWindowEventsPollingManager.h"
#include "RendererLib/RendererCachedScene.h"
#include "RendererLib/DisplayController.h"
#include "RendererLib/RendererLogContext.h"
#include "RendererLib/DisplayConfig.h"
#include "RendererLib/RendererScenes.h"
#include "RendererLib/DisplayEventHandler.h"
#include "RendererLib/SceneExpirationMonitor.h"
#include "Platform_Base/PlatformFactory_Base.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    const Vector4 Renderer::DefaultClearColor = { 0.f, 0.f, 0.f, 1.f };

    Renderer::Renderer(IPlatformFactory& platformFactory, const RendererScenes& rendererScenes, RendererEventCollector& eventCollector, const FrameTimer& frameTimer, SceneExpirationMonitor& expirationMonitor, RendererStatistics& rendererStatistics)
        : m_platformFactory(platformFactory)
        , m_systemCompositorController(nullptr)
        , m_rendererScenes(rendererScenes)
        , m_displayHandlerManager(eventCollector)
        , m_statistics(rendererStatistics)
        , m_frameTimer(frameTimer)
        , m_expirationMonitor(expirationMonitor)
    {
        m_platformFactory.createPerRendererComponents();
        m_systemCompositorController = platformFactory.getSystemCompositorController();
        m_windowEventsPollingManager = platformFactory.getWindowEventsPollingManager();
    }

    Renderer::~Renderer()
    {
        assert(m_displays.empty());
        m_platformFactory.destroyPerRendererComponents();
    }

    void Renderer::registerOffscreenBuffer(DisplayHandle display, DeviceResourceHandle bufferDeviceHandle, UInt32 width, UInt32 height, Bool isInterruptible)
    {
        assert(m_displays.find(display) != m_displays.cend());
        assert(!hasAnyBufferWithInterruptedRendering());
        auto& displayInfo = m_displays.find(display)->second;
        displayInfo.buffersSetup.registerDisplayBuffer(bufferDeviceHandle, { 0, 0, width, height }, DefaultClearColor, true, isInterruptible);
        // no need to re-render OB as long as no scene is assigned to it, OB is cleared at creation time
        displayInfo.buffersSetup.setDisplayBufferToBeRerendered(bufferDeviceHandle, false);
    }

    void Renderer::unregisterOffscreenBuffer(DisplayHandle display, DeviceResourceHandle bufferDeviceHandle)
    {
        assert(m_displays.find(display) != m_displays.cend());
        assert(!hasAnyBufferWithInterruptedRendering());
        auto& displayInfo = m_displays[display];
        displayInfo.buffersSetup.unregisterDisplayBuffer(bufferDeviceHandle);
        m_statistics.untrackOffscreenBuffer(display, bufferDeviceHandle);

        displayInfo.screenshots.erase(bufferDeviceHandle);
    }

    const IDisplayController& Renderer::getDisplayController(DisplayHandle display) const
    {
        assert(m_displays.find(display) != m_displays.cend());
        return *m_displays.find(display)->second.displayController;
    }

    IDisplayController& Renderer::getDisplayController(DisplayHandle display)
    {
        // non-const version of getDisplayController cast to its const version to avoid duplicating code
        return const_cast<IDisplayController&>((const_cast<const Renderer&>(*this)).getDisplayController(display));
    }

    UInt32 Renderer::getDisplayControllerCount() const
    {
        return static_cast<UInt32>(m_displays.size());
    }

    DisplayEventHandler& Renderer::getDisplayEventHandler(DisplayHandle display)
    {
        return m_displayHandlerManager.getHandler(display);
    }

    void Renderer::setWarpingMeshData(DisplayHandle display, const WarpingMeshData& meshData)
    {
        auto& displayController = getDisplayController(display);
        assert(displayController.isWarpingEnabled());
        displayController.setWarpingMeshData(meshData);
        // re-render framebuffer of the display
        auto& displayInfo = m_displays.find(display)->second;
        displayInfo.buffersSetup.setDisplayBufferToBeRerendered(displayInfo.frameBufferDeviceHandle, true);
    }

    void Renderer::addDisplayController(IDisplayController& display, DisplayHandle displayHandle)
    {
        assert(m_displays.find(displayHandle) == m_displays.cend());
        assert(!hasAnyBufferWithInterruptedRendering());

        DisplayInfo& displayInfo = m_displays[displayHandle];
        displayInfo.displayController = &display;
        displayInfo.frameBufferDeviceHandle = display.getDisplayBuffer();
        displayInfo.buffersSetup.registerDisplayBuffer(displayInfo.frameBufferDeviceHandle, { 0, 0, display.getDisplayWidth(), display.getDisplayHeight() }, DefaultClearColor, false, false);
        displayInfo.couldRenderLastFrame = true;

        auto profileRenderer = new FrameProfileRenderer(display.getRenderBackend().getDevice(), display.getDisplayWidth(), display.getDisplayHeight());
        m_frameProfileRenderer.put(displayHandle, profileRenderer);
    }

    void Renderer::createDisplayContext(const DisplayConfig& displayConfig, DisplayHandle display)
    {
        LOG_TRACE(CONTEXT_PROFILING, "Renderer::createDisplayContext start creating display");

        DisplayEventHandler& displayHandler = m_displayHandlerManager.createHandler(display);
        IDisplayController* displayController = createDisplayControllerFromConfig(displayConfig, displayHandler);
        if (!displayController)
        {
            LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::createDisplayContext: failed to create a display controller");
            return;
        }

        addDisplayController(*displayController, display);
        setClearColor(display, displayController->getDisplayBuffer(), displayConfig.getClearColor());

        LOG_TRACE(CONTEXT_PROFILING, "RamsesRenderer::createDisplayContext finished creating display");
    }

    void Renderer::destroyDisplayContext(DisplayHandle display)
    {
        IDisplayController& displayController = getDisplayController(display);
        removeDisplayController(display);

        IRenderBackend& renderBackend = displayController.getRenderBackend();

        if (m_systemCompositorController != nullptr)
        {
            // ivi systemcompositor case
            const WaylandIviSurfaceId surfaceId = renderBackend.getSurface().getWindow().getWaylandIviSurfaceID();
            systemCompositorDestroyIviSurface(surfaceId);
        }

        delete &displayController;
        m_platformFactory.destroyRenderBackend(renderBackend);
        m_displayHandlerManager.destroyHandler(display);
    }

    void Renderer::removeDisplayController(DisplayHandle display)
    {
        assert(m_displays.find(display) != m_displays.cend());
        DisplayInfo& displayInfo = m_displays.find(display)->second;
        assert(!hasAnyBufferWithInterruptedRendering());

        IDisplayController& displayController = *displayInfo.displayController;
        displayController.validateRenderingStatusHealthy();

        m_displays.erase(display);
        assert(m_frameProfileRenderer.contains(display));
        auto renderer = *m_frameProfileRenderer.get(display);
        delete renderer;
        m_frameProfileRenderer.remove(display);
    }

    const DisplaySetup& Renderer::getDisplaySetup(DisplayHandle displayHandle) const
    {
        assert(m_displays.count(displayHandle) > 0);
        return m_displays.find(displayHandle)->second.buffersSetup;
    }

    Bool Renderer::hasDisplayController(DisplayHandle display) const
    {
        return m_displays.find(display) != m_displays.cend();
    }

    void Renderer::systemCompositorListIviSurfaces() const
    {
        if(nullptr != m_systemCompositorController)
        {
            m_systemCompositorController->listIVISurfaces();
        }
        else
        {
            // TODO Violin refactor this code (comment is old, but still relevant)
            // This happens in the (frequent) case that the system compositor, a highly platform-specific feature,
            // was not created for whatever reason (there are a lot of reasons, such as socket locks, embedded
            // compositor dependency problems, wrong wayland version, ...)
            LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::systemCompositorListIviSurfaces: system compositor is not available");
        }
    }

    void Renderer::systemCompositorSetIviSurfaceVisibility(WaylandIviSurfaceId surfaceId, Bool visibility) const
    {
        if(nullptr != m_systemCompositorController)
        {
            m_systemCompositorController->setSurfaceVisibility(surfaceId, visibility);
        }
        else
        {
            // See above ^^^^
            LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::systemCompositorSetSurfaceVisibility: system compositor is not available");
        }
    }

    void Renderer::systemCompositorSetIviSurfaceOpacity(WaylandIviSurfaceId surfaceId, Float opacity) const
    {
        if(nullptr != m_systemCompositorController)
        {
            m_systemCompositorController->setSurfaceOpacity(surfaceId, opacity);
        }
        else
        {
            // See above ^^^
            LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::systemCompositorSetOpacity: system compositor is not available");
        }
    }

    void Renderer::systemCompositorSetIviSurfaceDestRectangle(WaylandIviSurfaceId surfaceId, Int32 x, Int32 y, Int32 width, Int32 height) const
    {
        if(nullptr != m_systemCompositorController)
        {
            m_systemCompositorController->setSurfaceDestinationRectangle(surfaceId, x, y, width, height);
        }
        else
        {
            // See above ^^
            LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::systemCompositorSetRectangle: system compositor is not available");
        }
    }

    void Renderer::systemCompositorSetIviLayerVisibility(WaylandIviLayerId layerId, Bool visibility) const
    {
        if(nullptr != m_systemCompositorController)
        {
            m_systemCompositorController->setLayerVisibility(layerId, visibility);
        }
        else
        {
            // See above ^^^^
            LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::systemCompositorSetLayerVisibility: system compositor is not available");
        }
    }

    void Renderer::systemCompositorScreenshot(const String& fileName, int32_t screenIviId) const
    {
        if(nullptr != m_systemCompositorController)
        {
            m_systemCompositorController->doScreenshot(fileName, screenIviId);
        }
        else
        {
            // See above ^
            LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::systemCompositorScreenshot: system compositor is not available");
        }
    }

    Bool Renderer::systemCompositorAddIviSurfaceToIviLayer(WaylandIviSurfaceId surfaceId, WaylandIviLayerId layerId) const
    {
        if(nullptr != m_systemCompositorController)
        {
            return m_systemCompositorController->addSurfaceToLayer(surfaceId, layerId);
        }
        else
        {
            // See above ^
            LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::systemCompositorAddSurfaceToLayer: system compositor is not available");
        }

        return true;
    }

    void Renderer::systemCompositorRemoveIviSurfaceFromIviLayer(WaylandIviSurfaceId surfaceId, WaylandIviLayerId layerId) const
    {
        if(nullptr != m_systemCompositorController)
        {
            m_systemCompositorController->removeSurfaceFromLayer(surfaceId, layerId);
        }
        else
        {
            // See above ^
            LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::systemCompositorRemoveSurfaceFromLayer: system compositor is not available");
        }
    }

    void Renderer::systemCompositorDestroyIviSurface(WaylandIviSurfaceId surfaceId) const
    {
        if(nullptr != m_systemCompositorController)
        {
            m_systemCompositorController->destroySurface(surfaceId);
        }
        else
        {
            // See above ^
            LOG_ERROR(CONTEXT_RENDERER, "RamsesRenderer::systemCompositorDestroySurface: system compositor is not available");
        }
    }

    void Renderer::handleDisplayEvents(DisplayHandle displayHandle)
    {
        auto& displayInfo = m_displays.find(displayHandle)->second;
        IDisplayController& display = *displayInfo.displayController;

        display.handleWindowEvents();
        const Bool canRenderFrame = display.canRenderNewFrame();
        if (canRenderFrame != displayInfo.couldRenderLastFrame)
        {
            displayInfo.couldRenderLastFrame = canRenderFrame;
            if (canRenderFrame)
            {
                LOG_INFO(CONTEXT_RENDERER, "      Renderer::doOneRenderLoop start rendering frames on display " << displayHandle.asMemoryHandle());
            }
            else
            {
                LOG_INFO(CONTEXT_RENDERER, "      Renderer::doOneRenderLoop will skip frames because window is not ready for rendering on display " << displayHandle.asMemoryHandle());
            }
        }
    }

    void Renderer::renderToFramebuffer(DisplayHandle displayHandle, DisplayHandle& activeDisplay)
    {
        auto& displayInfo = m_displays.find(displayHandle)->second;
        assert(displayInfo.couldRenderLastFrame);
        IDisplayController& display = *displayInfo.displayController;
        const DisplayBufferInfo& displayBufferInfo = displayInfo.buffersSetup.getDisplayBuffer(displayInfo.frameBufferDeviceHandle);
        if (!displayBufferInfo.needsRerender)
        {
            // notify clients even if nothing rendered but frame was consumed
            display.getEmbeddedCompositingManager().notifyClients();
            return;
        }

        ActivateDisplayContext(displayHandle, activeDisplay, display);

        display.clearBuffer(displayInfo.frameBufferDeviceHandle, displayBufferInfo.clearColor);

        m_tempScenesRendered.clear();
        const auto& assignedScenes = displayBufferInfo.scenes;
        for (const auto& sceneInfo : assignedScenes)
        {
            if (sceneInfo.shown)
            {
                const RendererCachedScene& scene = m_rendererScenes.getScene(sceneInfo.sceneId);
                display.renderScene(scene, displayInfo.frameBufferDeviceHandle, displayBufferInfo.viewport);
                onSceneWasRendered(scene);
                m_tempScenesRendered.push_back(sceneInfo.sceneId);
            }
        }
        LOG_TRACE_F(CONTEXT_PROFILING, ([&](StringOutputStream& logStream)
        {
            logStream << "Renderer::renderToFramebuffer (display " << displayHandle.asMemoryHandle() << ") rendered scenes:";
            for (auto sceneId : m_tempScenesRendered)
                logStream << " " << sceneId;
        }));

        display.executePostProcessing();

        auto profileRenderer = *m_frameProfileRenderer.get(displayHandle);
        profileRenderer->renderStatistics(m_profilerStatistics);

        processScheduledScreenshots(displayInfo.frameBufferDeviceHandle, display, activeDisplay);

        m_tempDisplaysToSwapBuffers.push_back(displayHandle);
        displayInfo.buffersSetup.setDisplayBufferToBeRerendered(displayInfo.frameBufferDeviceHandle, false);
    }

    void Renderer::renderToOffscreenBuffers(DisplayHandle displayHandle, DisplayHandle& activeDisplay)
    {
        auto& displayInfo = m_displays.find(displayHandle)->second;
        assert(displayInfo.couldRenderLastFrame);
        IDisplayController& display = *displayInfo.displayController;

        const auto& displayBuffersToRender = displayInfo.buffersSetup.getNonInterruptibleOffscreenBuffersToRender();
        if (displayBuffersToRender.empty())
            return;

        ActivateDisplayContext(displayHandle, activeDisplay, display);

        for (const auto displayBuffer : displayBuffersToRender)
        {
            const auto& displayBufferInfo = displayInfo.buffersSetup.getDisplayBuffer(displayBuffer);
            display.clearBuffer(displayBuffer, displayBufferInfo.clearColor);

            m_tempScenesRendered.clear();
            const auto& assignedScenes = displayBufferInfo.scenes;
            for (const auto& sceneInfo : assignedScenes)
            {
                if (sceneInfo.shown)
                {
                    const RendererCachedScene& scene = m_rendererScenes.getScene(sceneInfo.sceneId);
                    display.renderScene(scene, displayBuffer, displayBufferInfo.viewport);
                    onSceneWasRendered(scene);
                    m_tempScenesRendered.push_back(sceneInfo.sceneId);
                }
            }
            LOG_TRACE_F(CONTEXT_PROFILING, ([&](StringOutputStream& logStream)
            {
                logStream << "Renderer::renderToOffscreenBuffers (display " << displayHandle.asMemoryHandle() << ") OB" << displayBuffer.asMemoryHandle() << " rendered scenes:";
                for (auto sceneId : m_tempScenesRendered)
                    logStream << " " << sceneId;
            }));

            processScheduledScreenshots(displayBuffer, display, activeDisplay);

            m_statistics.offscreenBufferSwapped(displayHandle, displayBuffer, false);
            displayInfo.buffersSetup.setDisplayBufferToBeRerendered(displayBuffer, false);
        }
    }

    void Renderer::renderToInterruptibleOffscreenBuffers(DisplayHandle displayHandle, DisplayHandle& activeDisplay, Bool& interrupted)
    {
        auto& displayInfo = m_displays.find(displayHandle)->second;
        assert(displayInfo.couldRenderLastFrame);
        IDisplayController& display = *displayInfo.displayController;

        const auto& displayBuffersToRender = displayInfo.buffersSetup.getInterruptibleOffscreenBuffersToRender(m_rendererInterruptState.getInterruptedDisplayBuffer());
        if (displayBuffersToRender.empty())
            return;

        ActivateDisplayContext(displayHandle, activeDisplay, display);

        for (const auto displayBuffer : displayBuffersToRender)
        {
            const auto& displayBufferInfo = displayInfo.buffersSetup.getDisplayBuffer(displayBuffer);

            if (!m_rendererInterruptState.isInterrupted(displayHandle, displayBuffer))
            {
                // remove buffer from list of buffers to re-render as soon as we start rendering into it (even if it gets interrupted later on)
                displayInfo.buffersSetup.setDisplayBufferToBeRerendered(displayBuffer, false);
                // clear buffer only if we just started rendering into it
                display.clearBuffer(displayBuffer, displayBufferInfo.clearColor);
                LOG_TRACE(CONTEXT_PROFILING, "Renderer::renderToInterruptibleOffscreenBuffers (display " << displayHandle.asMemoryHandle() << ") interruptible OB " << displayBuffer.asMemoryHandle() << " cleared");
            }

            const auto& assignedScenes = displayBufferInfo.scenes;
            for (const auto& sceneInfo : assignedScenes)
            {
                if (!sceneInfo.shown)
                    continue;

                // if there was any rendering interrupted, skip till get to the interrupted scene
                const SceneId sceneId = sceneInfo.sceneId;
                if (m_rendererInterruptState.isInterrupted() && !m_rendererInterruptState.isInterrupted(displayHandle, displayBuffer, sceneId))
                    continue;

                const RendererCachedScene& scene = m_rendererScenes.getScene(sceneId);
                const SceneRenderExecutionIterator interruptState = display.renderScene(scene, displayBuffer, displayBufferInfo.viewport, m_rendererInterruptState.getExecutorState(), &m_frameTimer);

                if (RendererInterruptState::IsInterrupted(interruptState))
                {
                    m_rendererInterruptState = RendererInterruptState{ displayHandle, displayBuffer, sceneId, interruptState };
                    LOG_TRACE(CONTEXT_PROFILING, "Renderer::renderToInterruptibleOffscreenBuffers interrupted rendering to OB " << displayBuffer.asMemoryHandle() << " on display " << displayHandle.asMemoryHandle() << ", scene " << sceneId.getValue());
                    interrupted = true;
                    m_statistics.offscreenBufferInterrupted(displayHandle, displayBuffer);
                    break;
                }
                m_rendererInterruptState = RendererInterruptState{};

                onSceneWasRendered(scene);
                LOG_TRACE(CONTEXT_PROFILING, "Renderer::renderToInterruptibleOffscreenBuffers scene fully rendered to interruptible OB " << displayBuffer.asMemoryHandle() << " on display " << displayHandle.asMemoryHandle() << ", scene " << sceneId.getValue());
            }

            if (m_rendererInterruptState.isInterrupted())
                break;

            processScheduledScreenshots(displayBuffer, display, activeDisplay);

            displayInfo.displayController->getRenderBackend().getDevice().swapDoubleBufferedRenderTarget(displayBuffer);
            m_statistics.offscreenBufferSwapped(displayHandle, displayBuffer, true);
            LOG_TRACE(CONTEXT_PROFILING, "Renderer::renderToInterruptibleOffscreenBuffers interruptible OB " << displayBuffer.asMemoryHandle() << " swapped");

            //re-render framebuffer in next frame to reflect (finished!) changes in OB
            displayInfo.buffersSetup.setDisplayBufferToBeRerendered(displayInfo.frameBufferDeviceHandle, true);
        }
    }

    void Renderer::doOneRenderLoop()
    {
        LOG_TRACE(CONTEXT_PROFILING, "Renderer::doOneRenderLoop begin");

        if (!m_skipUnmodifiedBuffers)
        {
            //mark all displays as modified (and hence re-render everything)
            for (auto& display : m_displays)
            {
                auto& displayBufferSetup = display.second.buffersSetup;
                for (const auto& buffer : displayBufferSetup.getDisplayBuffers())
                    displayBufferSetup.setDisplayBufferToBeRerendered(buffer.first, true);
            }
        }

        DisplayHandle activeDisplay;
        m_tempDisplaysToSwapBuffers.clear();
        m_tempDisplaysToRender.clear();

        m_profilerStatistics.startRegion(FrameProfilerStatistics::ERegion::HandleDisplayEvents);

        if(nullptr != m_windowEventsPollingManager)
            m_windowEventsPollingManager->pollWindowsTillAnyCanRender();

        for (const auto& displayIt : m_displays)
        {
            handleDisplayEvents(displayIt.first);
            if (displayIt.second.couldRenderLastFrame)
                m_tempDisplaysToRender.push_back(displayIt.first);
        }
        m_profilerStatistics.endRegion(FrameProfilerStatistics::ERegion::HandleDisplayEvents);

        m_profilerStatistics.startRegion(FrameProfilerStatistics::ERegion::DrawScenes);
        // FRAMEBUFFER AND OFFSCREEN BUFFERS
        for (auto displayHandle : m_tempDisplaysToRender)
        {
            LOG_TRACE(CONTEXT_PROFILING, "Renderer::doOneRenderLoop begin frame to offscreen buffers on display " << displayHandle.asMemoryHandle());
            renderToOffscreenBuffers(displayHandle, activeDisplay);
            LOG_TRACE(CONTEXT_PROFILING, "Renderer::doOneRenderLoop finished frame to offscreen buffers on display " << displayHandle.asMemoryHandle());

            LOG_TRACE(CONTEXT_PROFILING, "Renderer::doOneRenderLoop begin frame to backbuffer on display " << displayHandle.asMemoryHandle());
            renderToFramebuffer(displayHandle, activeDisplay);
            LOG_TRACE(CONTEXT_PROFILING, "Renderer::doOneRenderLoop finished frame to backbuffer on display " << displayHandle.asMemoryHandle());
        }

        // INTERRUPTIBLE OFFSCREEN BUFFERS
        // Interruptible OBs cannot be reordered to minimize context switches as the interruption state relies on unmodified order of rendering
        for (auto displayHandle : m_tempDisplaysToRender)
        {
            LOG_TRACE(CONTEXT_PROFILING, "Renderer::doOneRenderLoop begin frame to interruptible offscreen buffers on display " << displayHandle.asMemoryHandle());

            if (!m_rendererInterruptState.isInterrupted() || m_rendererInterruptState.isInterrupted(displayHandle))
            {
                Bool interrupted = false;
                renderToInterruptibleOffscreenBuffers(displayHandle, activeDisplay, interrupted);
                if (interrupted)
                    break;
            }

            LOG_TRACE(CONTEXT_PROFILING, "Renderer::doOneRenderLoop finished frame to interruptible offscreen buffers on display " << displayHandle.asMemoryHandle());
        }
        m_profilerStatistics.endRegion(FrameProfilerStatistics::ERegion::DrawScenes);

        // SWAP BUFFERS
        m_profilerStatistics.startRegion(FrameProfilerStatistics::ERegion::SwapBuffersAndNotifyClients);
        ReorderDisplaysToStartWith(m_tempDisplaysToSwapBuffers, activeDisplay);
        for (auto displayHandle : m_tempDisplaysToSwapBuffers)
        {
            IDisplayController& displayController = getDisplayController(displayHandle);
            ActivateDisplayContext(displayHandle, activeDisplay, displayController);
            displayController.swapBuffers();
            m_statistics.framebufferSwapped(displayHandle);
            displayController.getEmbeddedCompositingManager().notifyClients();
            LOG_TRACE(CONTEXT_PROFILING, "Renderer::doOneRenderLoop swapBuffers on display " << displayHandle.asMemoryHandle());
        }
        m_profilerStatistics.endRegion(FrameProfilerStatistics::ERegion::SwapBuffersAndNotifyClients);

        LOG_TRACE(CONTEXT_PROFILING, "Renderer::doOneRenderLoop end");
    }

    void Renderer::onSceneWasRendered(const RendererCachedScene& scene)
    {
        scene.markAllRenderOncePassesAsRendered();
        m_expirationMonitor.onRendered(scene.getSceneId());
        m_statistics.sceneRendered(scene.getSceneId());
    }

    void Renderer::ActivateDisplayContext(DisplayHandle displayToActivate, DisplayHandle& activeDisplay, IDisplayController& dispController)
    {
        if (displayToActivate != activeDisplay)
        {
            dispController.enableContext();
            activeDisplay = displayToActivate;
        }
    }

    void Renderer::ReorderDisplaysToStartWith(std::vector<DisplayHandle>& displays, DisplayHandle displayToStartWith)
    {
        const auto displayToStartWithIt = std::find(displays.begin(), displays.end(), displayToStartWith);
        if (displayToStartWithIt != displays.end())
            std::iter_swap(displays.begin(), displayToStartWithIt);
    }

    void Renderer::assignSceneToDisplayBuffer(SceneId sceneId, DisplayHandle displayHandle, DeviceResourceHandle buffer, Int32 globalSceneOrder)
    {
        assert(m_displays.find(displayHandle) != m_displays.cend());
        assert(m_rendererScenes.hasScene(sceneId));

        auto& displayInfo = m_displays.find(displayHandle)->second;
        DisplayHandle currentDisplaySceneIsAssignedTo;
        getBufferSceneIsAssignedTo(sceneId, &currentDisplaySceneIsAssignedTo);
        assert(!currentDisplaySceneIsAssignedTo.isValid() || currentDisplaySceneIsAssignedTo == displayHandle);
        displayInfo.buffersSetup.assignSceneToDisplayBuffer(sceneId, buffer, globalSceneOrder);
    }

    void Renderer::unassignScene(SceneId sceneId)
    {
        assert(m_rendererScenes.hasScene(sceneId));
        const DisplayHandle displayHandle = getDisplaySceneIsAssignedTo(sceneId);
        assert(displayHandle.isValid());
        auto& displayInfo = m_displays.find(displayHandle)->second;
        displayInfo.buffersSetup.unassignScene(sceneId);
    }

    void Renderer::setSceneShown(SceneId sceneId, Bool show)
    {
        assert(m_rendererScenes.hasScene(sceneId));
        DisplayHandle displayHandle;
        const auto displayBuffer = getBufferSceneIsAssignedTo(sceneId, &displayHandle);
        assert(displayBuffer.isValid());
        UNUSED(displayBuffer);
        auto& displayInfo = m_displays.find(displayHandle)->second;
        displayInfo.buffersSetup.setSceneShown(sceneId, show);
    }

    void Renderer::markBufferWithSceneAsModified(SceneId sceneId)
    {
        DisplayHandle displayHandle;
        const auto displayBuffer = getBufferSceneIsAssignedTo(sceneId, &displayHandle);
        assert(displayHandle.isValid());
        assert(displayBuffer.isValid());
        auto& displayInfo = m_displays.find(displayHandle)->second;
        displayInfo.buffersSetup.setDisplayBufferToBeRerendered(displayBuffer, true);
    }

    void Renderer::setSkippingOfUnmodifiedBuffers(Bool enable)
    {
        m_skipUnmodifiedBuffers = enable;
    }

    DisplayHandle Renderer::getDisplaySceneIsAssignedTo(SceneId sceneId) const
    {
        DisplayHandle display;
        getBufferSceneIsAssignedTo(sceneId, &display);
        return display;
    }

    DeviceResourceHandle Renderer::getBufferSceneIsAssignedTo(SceneId sceneId, DisplayHandle* displayHandleOut) const
    {
        for (const auto& displayInfoPair : m_displays)
        {
            const auto& buffers = displayInfoPair.second.buffersSetup;
            const auto displayBuffer = buffers.findDisplayBufferSceneIsAssignedTo(sceneId);
            if (displayBuffer.isValid())
            {
                if (displayHandleOut != nullptr)
                    *displayHandleOut = displayInfoPair.first;
                return displayBuffer;
            }
        }

        return DeviceResourceHandle::Invalid();
    }

    Bool Renderer::isSceneAssignedToInterruptibleOffscreenBuffer(SceneId sceneId) const
    {
        DisplayHandle displayHandle;
        const auto displayBuffer = getBufferSceneIsAssignedTo(sceneId, &displayHandle);
        assert(displayHandle.isValid());
        assert(displayBuffer.isValid());

        const auto& displayBuffers = m_displays.find(displayHandle)->second.buffersSetup;
        return displayBuffers.getDisplayBuffer(displayBuffer).isInterruptible;
    }

    Int32 Renderer::getSceneGlobalOrder(SceneId sceneId) const
    {
        DisplayHandle displayHandle;
        const auto displayBuffer = getBufferSceneIsAssignedTo(sceneId, &displayHandle);
        assert(displayHandle.isValid());
        assert(displayBuffer.isValid());

        const auto& displayBuffers = m_displays.find(displayHandle)->second.buffersSetup;
        const auto& assignedScenes = displayBuffers.getDisplayBuffer(displayBuffer).scenes;
        const auto it = std::find_if(assignedScenes.cbegin(), assignedScenes.cend(), [sceneId](const auto& a) { return a.sceneId == sceneId; });
        assert(it != assignedScenes.cend());
        return it->globalSceneOrder;
    }

    RendererStatistics& Renderer::getStatistics()
    {
        return m_statistics;
    }

    FrameProfilerStatistics& Renderer::getProfilerStatistics()
    {
        return m_profilerStatistics;
    }

    MemoryStatistics& Renderer::getMemoryStatistics()
    {
        return m_memoryStatistics;
    }

    Bool Renderer::hasSystemCompositorController() const
    {
        return nullptr != m_systemCompositorController;
    }

    IDisplayController* Renderer::createDisplayControllerFromConfig(const DisplayConfig& config, DisplayEventHandler& displayEventHandler)
    {
        IRenderBackend* renderBackend = m_platformFactory.createRenderBackend(config, displayEventHandler);
        if (nullptr == renderBackend)
        {
            return nullptr;
        }

        if (m_systemCompositorController != nullptr)
        {
            const WaylandIviSurfaceId rendererSurfaceIVIID(config.getWaylandIviSurfaceID());
            const WaylandIviLayerId rendererLayerIVIID(config.getWaylandIviLayerID());

            if (!systemCompositorAddIviSurfaceToIviLayer(rendererSurfaceIVIID, rendererLayerIVIID))
            {
                return nullptr;
            }

            systemCompositorSetIviSurfaceVisibility(rendererSurfaceIVIID, config.getStartVisibleIvi());
            systemCompositorSetIviSurfaceDestRectangle(rendererSurfaceIVIID, config.getWindowPositionX(), config.getWindowPositionY(), config.getDesiredWindowWidth(), config.getDesiredWindowHeight());
        }

        // Enable the context of the render backend that was just created
        // The initialization of display controller below needs active context
        renderBackend->getSurface().enable();

        const UInt32 postProcessorEffects = config.isWarpingEnabled() ? EPostProcessingEffect_Warping : EPostProcessingEffect_None;
        const UInt32 numSamples = (config.getAntialiasingMethod() == EAntiAliasingMethod_MultiSampling) ? config.getAntialiasingSampleCount() : 1u;
        IDisplayController* displayController = new DisplayController(*renderBackend, numSamples, postProcessorEffects);

        displayController->setViewPosition(config.getCameraPosition());
        displayController->setViewRotation(config.getCameraRotation());

        const ProjectionParams& params = config.getProjectionParams();
        displayController->setProjectionParams(params);

        return displayController;
    }

    void Renderer::setClearColor(DisplayHandle displayHandle, DeviceResourceHandle bufferDeviceHandle, const Vector4& clearColor)
    {
        assert(m_displays.find(displayHandle) != m_displays.cend());
        auto& displayInfo = m_displays.find(displayHandle)->second;
        displayInfo.buffersSetup.setClearColor(bufferDeviceHandle, clearColor);
    }

    void Renderer::scheduleScreenshot(DisplayHandle display, DeviceResourceHandle renderTargetHandle, ScreenshotInfo&& screenshot)
    {
        assert(hasDisplayController(display));
        auto& displayInfo = m_displays.find(display)->second;

        if (displayInfo.screenshots.count(renderTargetHandle))
            LOG_WARN(CONTEXT_RENDERER, "Renderer::scheduleScreenshot: will overwrite previous screenshot request that was not executed yet (display=" << display << ", buffer=" << renderTargetHandle << ")");

        displayInfo.screenshots[renderTargetHandle] = std::move(screenshot);

        // re-render all buffers that the screenshot might depend on
        auto& displayBufferSetup = m_displays.find(display)->second.buffersSetup;
        for (const auto& buffer : displayBufferSetup.getDisplayBuffers())
            displayBufferSetup.setDisplayBufferToBeRerendered(buffer.first, true);
    }

    void Renderer::processScheduledScreenshots(DeviceResourceHandle renderTargetHandle, IDisplayController& controller, DisplayHandle displayHandle)
    {
        assert(m_displays.count(displayHandle));
        auto& displayInfo = m_displays[displayHandle];
        auto it = displayInfo.screenshots.find(renderTargetHandle);
        if (it == displayInfo.screenshots.end())
            return;

        ScreenshotInfo& screenshot = it->second;
        assert(screenshot.rectangle.width > 0u && screenshot.rectangle.height > 0u);
        if (!screenshot.pixelData.empty())
            return;

        controller.readPixels(renderTargetHandle, screenshot.rectangle.x, screenshot.rectangle.y, screenshot.rectangle.width, screenshot.rectangle.height, screenshot.pixelData);
        assert(!screenshot.pixelData.empty());
    }

    std::vector<std::pair<DeviceResourceHandle, ScreenshotInfo>> Renderer::dispatchProcessedScreenshots(DisplayHandle display)
    {
        auto displayIt = m_displays.find(display);
        if (displayIt == m_displays.end())
            return {};
        auto& displayInfo = displayIt->second;

        std::vector<std::pair<DeviceResourceHandle, ScreenshotInfo>> result;
        for (auto& it : displayInfo.screenshots)
        {
            const auto rtHandle = it.first;
            auto& screenshot = it.second;
            if (!screenshot.pixelData.empty())
                result.emplace_back(rtHandle, std::move(screenshot));
        }

        for (const auto& it : result)
            displayInfo.screenshots.erase(it.first);

        return result;
    }

    Bool Renderer::hasAnyBufferWithInterruptedRendering() const
    {
        return m_rendererInterruptState.isInterrupted();
    }

    void Renderer::resetRenderInterruptState()
    {
        LOG_TRACE(CONTEXT_PROFILING, "Renderer::resetRenderInterruptState");
        m_rendererInterruptState = RendererInterruptState{};
    }

    FrameProfileRenderer& Renderer::getFrameProfileRenderer(DisplayHandle display)
    {
        assert(m_frameProfileRenderer.contains(display));
        return **m_frameProfileRenderer.get(display);
    }

    void Renderer::updateSystemCompositorController() const
    {
        if (nullptr != m_systemCompositorController)
        {
            LOG_TRACE(CONTEXT_PROFILING, "Renderer::updateSystemCompositorController update system compositor controller");
            m_systemCompositorController->update();
        }
    }
}
