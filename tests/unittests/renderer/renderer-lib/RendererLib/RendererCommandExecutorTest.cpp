//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ResourceDeviceHandleAccessorMock.h"
#include "RendererMock.h"
#include "DisplayControllerMock.h"
#include "RenderBackendMock.h"
#include "internal/RendererLib/RendererCommandExecutor.h"
#include "internal/RendererLib/RendererLogContext.h"
#include "internal/RendererLib/RendererResourceManager.h"
#include "internal/RendererLib/RendererCommandBuffer.h"
#include "internal/RendererLib/SceneStateExecutor.h"
#include "internal/RendererLib/RendererScenes.h"
#include "internal/RendererLib/FrameTimer.h"
#include "internal/RendererLib/SceneExpirationMonitor.h"
#include "internal/RendererLib/RendererSceneControlLogic.h"
#include "internal/RendererLib/RendererCommands.h"
#include "internal/RendererLib/RendererEventCollector.h"
#include "ComponentMocks.h"
#include "RendererSceneUpdaterFacade.h"
#include "RendererSceneControlLogicMock.h"
#include "SceneReferenceLogicMock.h"
#include "SystemCompositorControllerMock.h"
#include "EmbeddedCompositingManagerMock.h"
#include "RendererSceneEventSenderMock.h"
#include "RendererSceneUpdaterMock.h"
#include "internal/Core/Utils/ThreadLocalLog.h"

namespace ramses::internal {

    class ARendererCommandExecutor : public ::testing::Test
    {
    public:
        ARendererCommandExecutor()
            : m_rendererScenes(m_rendererEventCollector)
            , m_expirationMonitor(m_rendererScenes, m_rendererEventCollector, m_rendererStatistics)
            , m_renderer(m_displayHandle, m_rendererScenes, m_rendererEventCollector, m_expirationMonitor, m_rendererStatistics)
            , m_sceneStateExecutor(m_renderer, m_sceneEventSender, m_rendererEventCollector)
            , m_commandExecutor(m_renderer, m_commandBuffer, m_sceneUpdater, m_sceneControlLogic, m_rendererEventCollector, m_frameTimer)
        {
            // caller is expected to have a display prefix for logs
            ThreadLocalLog::SetPrefix(1);

            // enable SC
            ON_CALL(m_renderer.m_platform, getSystemCompositorController()).WillByDefault(Return(&m_renderer.m_platform.systemCompositorControllerMock));
        }

        void doCommandExecutorLoop()
        {
            m_renderer.getProfilerStatistics().markFrameFinished(std::chrono::microseconds{ 0u });
            m_commandExecutor.executePendingCommands();
        }

        void addDisplayController()
        {
            ASSERT_FALSE(m_renderer.hasDisplayController());
            m_renderer.createDisplayContext({});
        }

        void removeDisplayController()
        {
            ASSERT_TRUE(m_renderer.hasDisplayController());
            EXPECT_CALL(m_renderer.m_platform.renderBackendMock.windowMock, getWaylandIviSurfaceID());
            EXPECT_CALL(m_renderer.m_platform.systemCompositorControllerMock, destroySurface(_));
            EXPECT_CALL(m_renderer.m_platform, destroyRenderBackend());
            m_renderer.destroyDisplayContext();
        }

        StrictMock<DisplayControllerMock>& getDisplayControllerMock()
        {
            return *m_renderer.m_displayController;
        }

        RendererEventVector consumeRendererEvents()
        {
            RendererEventVector events;
            RendererEventVector dummy;
            m_rendererEventCollector.appendAndConsumePendingEvents(events, dummy);
            return events;
        }

        RendererEventVector consumeSceneControlEvents()
        {
            RendererEventVector events;
            RendererEventVector dummy;
            m_rendererEventCollector.appendAndConsumePendingEvents(dummy, events);
            return events;
        }

    protected:
        DisplayHandle                                 m_displayHandle{ 1u };
        RendererEventCollector                        m_rendererEventCollector;
        RendererScenes                                m_rendererScenes;
        SceneExpirationMonitor                        m_expirationMonitor;
        StrictMock<SceneReferenceLogicMock>           m_sceneRefLogic;
        RendererStatistics                            m_rendererStatistics;
        StrictMock<RendererMockWithStrictMockDisplay> m_renderer;
        StrictMock<RendererSceneEventSenderMock>      m_sceneEventSender;
        RendererCommandBuffer                         m_commandBuffer;
        SceneStateExecutor                            m_sceneStateExecutor;
        FrameTimer                                    m_frameTimer;
        StrictMock<RendererSceneUpdaterMock>          m_sceneUpdater;
        StrictMock<RendererSceneControlLogicMock>     m_sceneControlLogic;
        RendererCommandExecutor                       m_commandExecutor;
    };

    TEST_F(ARendererCommandExecutor, setSceneState)
    {
        constexpr SceneId sceneId{ 123 };
        m_commandBuffer.enqueueCommand(RendererCommand::SetSceneState{ sceneId, RendererSceneState::Ready });

        EXPECT_CALL(m_sceneControlLogic, setSceneState(sceneId, RendererSceneState::Ready));
        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, setSceneMappingIsNoop)
    {
        constexpr SceneId sceneId{ 123 };
        constexpr DisplayHandle display{ 2 };
        m_commandBuffer.enqueueCommand(RendererCommand::SetSceneMapping{ sceneId, display });
        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, setSceneDisplayBufferAssignment)
    {
        constexpr SceneId sceneId{ 123 };
        constexpr OffscreenBufferHandle ob{ 2 };
        m_commandBuffer.enqueueCommand(RendererCommand::SetSceneDisplayBufferAssignment{ sceneId, ob, -13 });

        EXPECT_CALL(m_sceneControlLogic, setSceneDisplayBufferAssignment(sceneId, ob, -13));
        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, readPixelsFromDisplayBuffer)
    {
        constexpr DisplayHandle display{ 1u };
        constexpr OffscreenBufferHandle buffer{ 2u };

        m_commandBuffer.enqueueCommand(RendererCommand::ReadPixels{ display, buffer, 1, 2, 3, 4, true, true, "file" });
        EXPECT_CALL(m_sceneUpdater, handleReadPixels(buffer, _)).WillOnce(Invoke([&](auto /*unused*/, auto&& info)
        {
            EXPECT_EQ(1u, info.rectangle.x);
            EXPECT_EQ(2u, info.rectangle.y);
            EXPECT_EQ(3u, info.rectangle.width);
            EXPECT_EQ(4u, info.rectangle.height);
            EXPECT_EQ(std::string("file"), info.filename);
            EXPECT_TRUE(info.fullScreen);
            EXPECT_TRUE(info.sendViaDLT);
        }));
        doCommandExecutorLoop();

        EXPECT_TRUE(consumeRendererEvents().empty()); // events are only added by the WindowedRenderer
    }

    TEST_F(ARendererCommandExecutor, createOffscreenBuffer)
    {
        constexpr DisplayHandle display{ 1 };
        constexpr OffscreenBufferHandle buffer{ 2 };

        m_commandBuffer.enqueueCommand(RendererCommand::CreateOffscreenBuffer{ display, buffer, 1u, 2u, 3u, true, EDepthBufferType::DepthStencil });
        EXPECT_CALL(m_sceneUpdater, handleBufferCreateRequest(buffer, 1u, 2u, 3u, true, EDepthBufferType::DepthStencil)).WillOnce(Return(true));
        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, createDmaOffscreenBuffer)
    {
        constexpr DisplayHandle display{ 1 };
        constexpr OffscreenBufferHandle buffer{ 2 };
        constexpr DmaBufferFourccFormat fourccFormat{ 555u };
        constexpr DmaBufferUsageFlags usageFlags{ 666u };
        constexpr DmaBufferModifiers modifiers{ 777u };

        m_commandBuffer.enqueueCommand(RendererCommand::CreateDmaOffscreenBuffer{ display, buffer, 1u, 2u, fourccFormat, usageFlags, modifiers });
        EXPECT_CALL(m_sceneUpdater, handleDmaBufferCreateRequest(buffer, 1u, 2u, fourccFormat, usageFlags, modifiers)).WillOnce(Return(true));
        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, destroysOffscreenBufferAndGenerateEvent)
    {
        constexpr DisplayHandle display{ 1 };
        constexpr OffscreenBufferHandle buffer{ 2 };

        m_commandBuffer.enqueueCommand(RendererCommand::DestroyOffscreenBuffer{ display, buffer });
        EXPECT_CALL(m_sceneUpdater, handleBufferDestroyRequest(buffer)).WillOnce(Return(true));
        doCommandExecutorLoop();

        const RendererEventVector events = consumeRendererEvents();
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::OffscreenBufferDestroyed, events.front().eventType);
        EXPECT_EQ(display, events.front().displayHandle);
        EXPECT_EQ(buffer, events.front().offscreenBuffer);
        EXPECT_EQ(-1, events.front().dmaBufferFD);
        EXPECT_EQ(0u, events.front().dmaBufferStride);
    }

    TEST_F(ARendererCommandExecutor, destroysOffscreenBufferAndGenerateEventOnFail)
    {
        constexpr DisplayHandle display{ 1 };
        constexpr OffscreenBufferHandle buffer{ 2 };

        m_commandBuffer.enqueueCommand(RendererCommand::DestroyOffscreenBuffer{ display, buffer });
        EXPECT_CALL(m_sceneUpdater, handleBufferDestroyRequest(buffer)).WillOnce(Return(false));
        doCommandExecutorLoop();

        const RendererEventVector events = consumeRendererEvents();
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::OffscreenBufferDestroyFailed, events.front().eventType);
        EXPECT_EQ(display, events.front().displayHandle);
        EXPECT_EQ(buffer, events.front().offscreenBuffer);
    }

    TEST_F(ARendererCommandExecutor, triggersLinkOffscreenBufferToConsumer)
    {
        const OffscreenBufferHandle buffer(1u);
        const SceneId consumerScene(2u);
        const DataSlotId consumerData(3u);
        m_commandBuffer.enqueueCommand(RendererCommand::LinkOffscreenBuffer{ buffer, consumerScene, consumerData });

        EXPECT_CALL(m_sceneUpdater, handleBufferToSceneDataLinkRequest(buffer, consumerScene, consumerData));
        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, createStreamBuffer)
    {
        constexpr StreamBufferHandle buffer{ 123u };
        constexpr DisplayHandle display{ 1 };
        constexpr WaylandIviSurfaceId source{ 2 };

        m_commandBuffer.enqueueCommand(RendererCommand::CreateStreamBuffer{ display, buffer, source });
        EXPECT_CALL(m_sceneUpdater, handleBufferCreateRequest(buffer, source));
        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, destroysStreamBuffer)
    {
        constexpr StreamBufferHandle buffer{ 123u };
        constexpr DisplayHandle display{ 1 };

        m_commandBuffer.enqueueCommand(RendererCommand::DestroyStreamBuffer{ display, buffer });
        EXPECT_CALL(m_sceneUpdater, handleBufferDestroyRequest(buffer));
        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, linkStreamBufferToConsumer)
    {
        constexpr StreamBufferHandle buffer{ 1u };
        constexpr SceneId consumerScene{ 2u };
        constexpr DataSlotId consumerData{ 3u };
        m_commandBuffer.enqueueCommand(RendererCommand::LinkStreamBuffer{ buffer, consumerScene, consumerData });

        EXPECT_CALL(m_sceneUpdater, handleBufferToSceneDataLinkRequest(buffer, consumerScene, consumerData));
        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, createExternalBuffer)
    {
        constexpr ExternalBufferHandle buffer{ 123u };
        constexpr DisplayHandle display{ 1 };

        m_commandBuffer.enqueueCommand(RendererCommand::CreateExternalBuffer{ display, buffer });
        EXPECT_CALL(m_sceneUpdater, handleExternalBufferCreateRequest(buffer));
        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, destroysExternalBuffer)
    {
        constexpr ExternalBufferHandle buffer{ 123u };
        constexpr DisplayHandle display{ 1 };

        m_commandBuffer.enqueueCommand(RendererCommand::DestroyExternalBuffer{ display, buffer });
        EXPECT_CALL(m_sceneUpdater, handleExternalBufferDestroyRequest(buffer));
        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, linkExternalBufferToConsumer)
    {
        constexpr ExternalBufferHandle buffer{ 1u };
        constexpr SceneId consumerScene{ 2u };
        constexpr DataSlotId consumerData{ 3u };
        m_commandBuffer.enqueueCommand(RendererCommand::LinkExternalBuffer{ buffer, consumerScene, consumerData });

        EXPECT_CALL(m_sceneUpdater, handleBufferToSceneDataLinkRequest(buffer, consumerScene, consumerData));
        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, createsAndDestroysDisplay)
    {
        const DisplayHandle displayHandle(33u);
        const DisplayConfig displayConfig;

        m_commandBuffer.enqueueCommand(RendererCommand::CreateDisplay{ displayHandle, displayConfig, nullptr });
        EXPECT_CALL(m_sceneUpdater, createDisplayContext(displayConfig, nullptr));
        doCommandExecutorLoop();

        m_commandBuffer.enqueueCommand(RendererCommand::DestroyDisplay{ displayHandle });
        EXPECT_CALL(m_sceneUpdater, destroyDisplayContext());
        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, linkData)
    {
        constexpr SceneId sceneProviderId(1u);
        constexpr SceneId sceneConsumerId(2u);
        constexpr DataSlotId providerSlotId(3u);
        constexpr DataSlotId consumerSlotId(4u);

        m_commandBuffer.enqueueCommand(RendererCommand::LinkData{ sceneProviderId, providerSlotId, sceneConsumerId, consumerSlotId });
        EXPECT_CALL(m_sceneUpdater, handleSceneDataLinkRequest(sceneProviderId, providerSlotId, sceneConsumerId, consumerSlotId));
        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, unlinkData)
    {
        constexpr SceneId sceneConsumerId(2u);
        constexpr DataSlotId consumerSlotId(4u);

        m_commandBuffer.enqueueCommand(RendererCommand::UnlinkData{ sceneConsumerId, consumerSlotId });
        EXPECT_CALL(m_sceneUpdater, handleDataUnlinkRequest(sceneConsumerId, consumerSlotId));
        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, processesSceneActions)
    {
        const SceneId sceneId(7u);
        const NodeHandle node(33u);

        SceneUpdate actions;
        SceneActionCollectionCreator creator(actions.actions);
        creator.allocateNode(0u, node);
        actions.flushInfos.flushCounter = 1u;
        actions.flushInfos.containsValidInformation = true;
        m_commandBuffer.enqueueCommand(RendererCommand::UpdateScene{ sceneId, std::move(actions) });

        EXPECT_CALL(m_sceneUpdater, handleSceneUpdate(sceneId, _));
        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, setsLayerVisibility)
    {
        m_commandBuffer.enqueueCommand(RendererCommand::SCSetIviLayerVisibility{ WaylandIviLayerId(23u), true });
        m_commandBuffer.enqueueCommand(RendererCommand::SCSetIviLayerVisibility{ WaylandIviLayerId(25u), false });

        EXPECT_CALL(m_renderer.m_platform.systemCompositorControllerMock, setLayerVisibility(WaylandIviLayerId(23u),true));
        EXPECT_CALL(m_renderer.m_platform.systemCompositorControllerMock, setLayerVisibility(WaylandIviLayerId(25u),false));

        m_commandExecutor.executePendingCommands();
    }

    TEST_F(ARendererCommandExecutor, setsSurfaceVisibility)
    {
        m_commandBuffer.enqueueCommand(RendererCommand::SCSetIviSurfaceVisibility{ WaylandIviSurfaceId(11u), true });
        m_commandBuffer.enqueueCommand(RendererCommand::SCSetIviSurfaceVisibility{ WaylandIviSurfaceId(22u), false });

        EXPECT_CALL(m_renderer.m_platform.systemCompositorControllerMock, setSurfaceVisibility(WaylandIviSurfaceId(11u),true));
        EXPECT_CALL(m_renderer.m_platform.systemCompositorControllerMock, setSurfaceVisibility(WaylandIviSurfaceId(22u),false));

        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, listsIviSurfaces)
    {
        m_commandBuffer.enqueueCommand(RendererCommand::SCListIviSurfaces{});

        EXPECT_CALL(m_renderer.m_platform.systemCompositorControllerMock, listIVISurfaces());

        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, setsSurfaceOpacity)
    {
        m_commandBuffer.enqueueCommand(RendererCommand::SCSetIviSurfaceOpacity{ WaylandIviSurfaceId(1u), 1.f });
        m_commandBuffer.enqueueCommand(RendererCommand::SCSetIviSurfaceOpacity{ WaylandIviSurfaceId(2u), 0.2f });

        EXPECT_CALL(m_renderer.m_platform.systemCompositorControllerMock, setSurfaceOpacity(WaylandIviSurfaceId(1u),1.f));
        EXPECT_CALL(m_renderer.m_platform.systemCompositorControllerMock, setSurfaceOpacity(WaylandIviSurfaceId(2u),0.2f));

        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, setsSurfaceRectangle)
    {
        m_commandBuffer.enqueueCommand(RendererCommand::SCSetIviSurfaceDestRectangle{ WaylandIviSurfaceId(111u), 0, 0, 640, 480 });
        m_commandBuffer.enqueueCommand(RendererCommand::SCSetIviSurfaceDestRectangle{ WaylandIviSurfaceId(222u), -1,-10, 1920, 1080 });

        EXPECT_CALL(m_renderer.m_platform.systemCompositorControllerMock, setSurfaceDestinationRectangle(WaylandIviSurfaceId(111u), 0, 0, 640, 480));
        EXPECT_CALL(m_renderer.m_platform.systemCompositorControllerMock, setSurfaceDestinationRectangle(WaylandIviSurfaceId(222u), -1,-10, 1920, 1080));

        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, callsSystemCompositorAddSurfaceToLayer)
    {
        m_commandBuffer.enqueueCommand(RendererCommand::SCAddIviSurfaceToIviLayer{ WaylandIviSurfaceId(222u), WaylandIviLayerId(111u) });

        EXPECT_CALL(m_renderer.m_platform.systemCompositorControllerMock, addSurfaceToLayer(WaylandIviSurfaceId(222u), WaylandIviLayerId(111u)));

        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, callsSystemCompositorRemoveSurfaceFromLayer)
    {
        m_commandBuffer.enqueueCommand(RendererCommand::SCRemoveIviSurfaceFromIviLayer{ WaylandIviSurfaceId(345u), WaylandIviLayerId(123u) });

        EXPECT_CALL(m_renderer.m_platform.systemCompositorControllerMock, removeSurfaceFromLayer(WaylandIviSurfaceId(345u), WaylandIviLayerId(123u)));

        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, callsSystemCompositorDestroySurface)
    {
        m_commandBuffer.enqueueCommand(RendererCommand::SCDestroyIviSurface{ WaylandIviSurfaceId(51u) });

        EXPECT_CALL(m_renderer.m_platform.systemCompositorControllerMock, destroySurface(WaylandIviSurfaceId(51u)));

        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, callsSystemCompositorScreenshot)
    {
        const std::string firstName("somefilename.png");
        const std::string otherName("somethingelse.png");

        m_commandBuffer.enqueueCommand(RendererCommand::SCScreenshot{ 1, firstName });
        m_commandBuffer.enqueueCommand(RendererCommand::SCScreenshot{ -1, otherName });

        EXPECT_CALL(m_renderer.m_platform.systemCompositorControllerMock, doScreenshot(std::string_view{firstName}, 1));
        EXPECT_CALL(m_renderer.m_platform.systemCompositorControllerMock, doScreenshot(std::string_view{otherName}, -1));

        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, setClearFlags)
    {
        constexpr DisplayHandle display{ 1 };
        constexpr OffscreenBufferHandle buffer{ 2 };

        m_commandBuffer.enqueueCommand(RendererCommand::SetClearFlags{ display, buffer, EClearFlag::Color });
        EXPECT_CALL(m_sceneUpdater, handleSetClearFlags(buffer, ClearFlags(EClearFlag::Color)));
        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, setClearColor)
    {
        constexpr DisplayHandle display{ 1 };
        constexpr OffscreenBufferHandle buffer{ 2 };
        constexpr glm::vec4 clearColor(1.f, 0.f, 0.2f, 0.3f);

        m_commandBuffer.enqueueCommand(RendererCommand::SetClearColor{ display, buffer, clearColor });
        EXPECT_CALL(m_sceneUpdater, handleSetClearColor(buffer, clearColor));
        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, resizeDisplayWindowExterally)
    {
        constexpr DisplayHandle display{ 1 };

        m_commandBuffer.enqueueCommand(RendererCommand::SetExterallyOwnedWindowSize{ display, 1u, 2u });
        EXPECT_CALL(m_sceneUpdater, handleSetExternallyOwnedWindowSize(1u, 2u));
        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, setFrameTimerLimits)
    {
        //default values
        ASSERT_EQ(PlatformTime::InfiniteDuration, m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::SceneResourcesUpload));
        ASSERT_EQ(PlatformTime::InfiniteDuration, m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::ResourcesUpload));
        ASSERT_EQ(PlatformTime::InfiniteDuration, m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::OffscreenBufferRender));

        m_commandBuffer.enqueueCommand(RendererCommand::SetLimits_FrameBudgets{ 4u, 1u, 3u });
        doCommandExecutorLoop();

        EXPECT_EQ(std::chrono::microseconds(4u), m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::SceneResourcesUpload));
        EXPECT_EQ(std::chrono::microseconds(1u), m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::ResourcesUpload));
        EXPECT_EQ(std::chrono::microseconds(3u), m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::OffscreenBufferRender));
    }

    TEST_F(ARendererCommandExecutor, forwardsPickEventToSceneUpdater)
    {
        const SceneId sceneId{ 123u };
        const glm::vec2 coords{ 0.1f, 0.2f };
        m_commandBuffer.enqueueCommand(RendererCommand::PickEvent{ sceneId, coords });
        EXPECT_CALL(m_sceneUpdater, handlePickEvent(sceneId, coords));
        doCommandExecutorLoop();
    }

    TEST_F(ARendererCommandExecutor, triggersLogRinfo)
    {
        m_commandBuffer.enqueueCommand(RendererCommand::LogInfo{ ERendererLogTopic::Displays, true, NodeHandle{ 6u } });
        EXPECT_CALL(m_sceneUpdater, logRendererInfo(ERendererLogTopic::Displays, true, NodeHandle{ 6u }));
        doCommandExecutorLoop();
    }
}
