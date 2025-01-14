//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Types.h"
#include "internal/RendererLib/PlatformInterface/IEmbeddedCompositingManager.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/RendererLib/StagingInfo.h"
#include "internal/RendererLib/BufferLinks.h"
#include "internal/RendererLib/FrameTimer.h"
#include "internal/RendererLib/IRendererSceneStateControl.h"
#include "internal/RendererLib/IRendererSceneUpdater.h"
#include "internal/RendererLib/IRendererResourceManager.h"
#include "internal/SceneGraph/Scene/EScenePublicationMode.h"
#include "AsyncEffectUploader.h"
#include <unordered_map>

namespace ramses::internal
{
    class RendererCachedScene;
    class Renderer;
    class SceneStateExecutor;
    class IBinaryShaderCache;
    class RendererEventCollector;
    class RendererScenes;
    class DisplayConfig;
    class SceneExpirationMonitor;
    struct SceneUpdate;
    class DataReferenceLinkManager;
    class TransformationLinkManager;
    class TextureLinkManager;
    class ISceneReferenceLogic;
    class IRenderBackend;
    class IPlatform;
    class IThreadAliveNotifier;

    class RendererSceneUpdater : public IRendererSceneUpdater, public IRendererSceneStateControl
    {
        friend class RendererLogger;

    public:
        RendererSceneUpdater(
            DisplayHandle display,
            IPlatform& platform,
            Renderer& renderer,
            RendererScenes& rendererScenes,
            SceneStateExecutor& sceneStateExecutor,
            RendererEventCollector& eventCollector,
            FrameTimer& frameTimer,
            SceneExpirationMonitor& expirationMonitor,
            IThreadAliveNotifier& notifier);
        ~RendererSceneUpdater() override;

        // IRendererSceneUpdater
        void handleSceneUpdate(SceneId sceneId, SceneUpdate&& sceneUpdate) override;
        void createDisplayContext(const DisplayConfig& displayConfig, IBinaryShaderCache* binaryShaderCache) override;
        void destroyDisplayContext() final override;
        void handleScenePublished(SceneId sceneId, EScenePublicationMode mode) override;
        void handleSceneUnpublished(SceneId sceneId) override;
        void handleSceneReceived(const SceneInfo& sceneInfo) override;
        bool handleBufferCreateRequest(OffscreenBufferHandle buffer, uint32_t width, uint32_t height, uint32_t sampleCount, bool isDoubleBuffered, EDepthBufferType depthStencilBufferType) override;
        bool handleDmaBufferCreateRequest(OffscreenBufferHandle buffer, uint32_t width, uint32_t height, DmaBufferFourccFormat dmaBufferFourccFormat, DmaBufferUsageFlags dmaBufferUsageFlags, DmaBufferModifiers dmaBufferModifiers) override;
        bool handleBufferDestroyRequest(OffscreenBufferHandle buffer) override;
        bool handleBufferCreateRequest(StreamBufferHandle buffer, WaylandIviSurfaceId source) override;
        bool handleBufferDestroyRequest(StreamBufferHandle buffer) override;
        bool handleExternalBufferCreateRequest(ExternalBufferHandle buffer) override;
        bool handleExternalBufferDestroyRequest(ExternalBufferHandle buffer) override;
        void handleSetClearFlags(OffscreenBufferHandle buffer, ClearFlags clearFlags) override;
        void handleSetClearColor(OffscreenBufferHandle buffer, const glm::vec4& clearColor) override;
        void handleSetExternallyOwnedWindowSize(uint32_t width, uint32_t height) override;
        void handleReadPixels(OffscreenBufferHandle buffer, ScreenshotInfo&& screenshotInfo) override;
        void handlePickEvent(SceneId sceneId, glm::vec2 coordsNormalizedToBufferSize) override;
        void handleSceneDataLinkRequest(SceneId providerSceneId, DataSlotId providerId, SceneId consumerSceneId, DataSlotId consumerId) override;
        void handleBufferToSceneDataLinkRequest(OffscreenBufferHandle buffer, SceneId consumerSceneId, DataSlotId consumerId) override;
        void handleBufferToSceneDataLinkRequest(StreamBufferHandle buffer, SceneId consumerSceneId, DataSlotId consumerId) override;
        void handleBufferToSceneDataLinkRequest(ExternalBufferHandle externalBuffer, SceneId consumerSceneId, DataSlotId consumerId) override;
        void handleDataUnlinkRequest(SceneId consumerSceneId, DataSlotId consumerId) override;
        void setLimitFlushesForceApply(size_t limitForPendingFlushesForceApply) override;
        void setLimitFlushesForceUnsubscribe(size_t limitForPendingFlushesForceUnsubscribe) override;
        void setSkippingOfUnmodifiedScenes(bool enable) override;
        void logRendererInfo(ERendererLogTopic topic, bool verbose, NodeHandle nodeFilter) const override;

        // IRendererSceneStateControl
        void handleSceneSubscriptionRequest     (SceneId sceneId) override;
        void handleSceneUnsubscriptionRequest   (SceneId sceneId, bool indirect) override;
        void handleSceneMappingRequest          (SceneId sceneId) override;
        void handleSceneUnmappingRequest        (SceneId sceneId) override;
        void handleSceneShowRequest             (SceneId sceneId) override;
        void handleSceneHideRequest             (SceneId sceneId) override;
        bool handleSceneDisplayBufferAssignmentRequest(SceneId sceneId, OffscreenBufferHandle buffer, int32_t sceneRenderOrder) override;

        void updateScenes();

        void processScreenshotResults();
        [[nodiscard]] bool hasPendingFlushes(SceneId sceneId) const;
        void setSceneReferenceLogicHandler(ISceneReferenceLogic& sceneRefLogic);

    protected:
        virtual std::unique_ptr<IRendererResourceManager> createResourceManager(
            IRenderBackend& renderBackend,
            IEmbeddedCompositingManager& embeddedCompositingManager,
            const DisplayConfig& displayConfig,
            IBinaryShaderCache* binaryShaderCache);

        [[nodiscard]] bool hasResourceManager() const;

        std::chrono::milliseconds m_maximumWaitingTimeToForceMap{ 2000 };

    private:
        void destroyScene(SceneId sceneID);
        void unloadSceneResourcesAndUnrefSceneResources(SceneId sceneId);
        bool markClientAndSceneResourcesForReupload(SceneId sceneId);

        void updateScenePendingFlushes(SceneId sceneID, StagingInfo& stagingInfo);
        static void ApplySceneActions(RendererCachedScene& scene, PendingFlush& flushInfo);
        void applyPendingFlushes(SceneId sceneID, StagingInfo& stagingInfo);
        void processStagedResourceChanges(SceneId sceneID, StagingInfo& stagingInfo);

        [[nodiscard]] bool areResourcesFromPendingFlushesUploaded(SceneId sceneId) const;

        void consolidatePendingSceneActions(SceneId sceneID, SceneUpdate&& sceneUpdate);
        void consolidateResourceDataForMapping(SceneId sceneID);
        void referenceAndProvidePendingResourceData(SceneId sceneID);
        void requestAndUploadAndUnloadResources();
        void uploadUpdatedECStreams();
        void tryToApplyPendingFlushes();
        void processStagedResourceChangesFromAppliedFlushes();
        void handleECStreamAvailabilityChanges();
        void uploadAndUnloadVertexArrays();
        void updateScenesResourceCache();
        void updateScenesShaderAnimations();
        void updateScenesTransformationCache();
        void updateScenesDataLinks();
        void updateScenesStates();

        void resolveDataLinksForConsumerScenes(const DataReferenceLinkManager& dataRefLinkManager);
        void markScenesDependantOnModifiedConsumersAsModified(const DataReferenceLinkManager& dataRefLinkManager, const TransformationLinkManager &transfLinkManager, const TextureLinkManager& texLinkManager);
        void markScenesDependantOnModifiedOffscreenBuffersAsModified(const TextureLinkManager& texLinkManager);

        bool checkIfForceMapNeeded(SceneId sceneId);
        void logTooManyFlushesAndUnsubscribeIfRemoteScene(SceneId sceneId, std::size_t numPendingFlushes);
        void logMissingResources(const PendingData& pendingData, SceneId sceneId) const;
        void logMissingResources(const ResourceContentHashVector& neededResources, SceneId sceneId) const;
        [[nodiscard]] uint32_t getNumberOfPendingNonEmptyFlushes(SceneId sceneId) const;

        DisplayHandle m_display;

        IPlatform&                                        m_platform;
        Renderer&                                         m_renderer;
        RendererScenes&                                   m_rendererScenes;
        SceneStateExecutor&                               m_sceneStateExecutor;
        RendererEventCollector&                           m_rendererEventCollector;
        FrameTimer&                                       m_frameTimer;
        SceneExpirationMonitor&                           m_expirationMonitor;
        ISceneReferenceLogic*                             m_sceneReferenceLogic = nullptr;

        std::unique_ptr<IRendererResourceManager> m_displayResourceManager;
        std::unique_ptr<AsyncEffectUploader> m_asyncEffectUploader;

        struct SceneMapRequest
        {
            FrameTimer::Clock::time_point requestTimeStamp;
            FrameTimer::Clock::time_point lastLogTimeStamp;
        };
        std::unordered_map<SceneId, SceneMapRequest> m_scenesToBeMapped;

        // extracted from RendererSceneUpdater::updateScenesTransformationCache to avoid per frame allocation
        HashSet<SceneId> m_scenesNeedingTransformationCacheUpdate;

        bool m_skipUnmodifiedScenes = true;
        HashSet<SceneId> m_modifiedScenesToRerender;
        //used as caches for algorithms that mark scenes as modified
        std::vector<SceneId> m_offscreeenBufferModifiedScenesVisitingCache;
        OffscreenBufferLinkVector m_offscreenBufferConsumerSceneLinksCache;

        size_t m_maximumPendingFlushes = 120u;
        size_t m_maximumPendingFlushesToKillScene = 5 * m_maximumPendingFlushes;

        IThreadAliveNotifier& m_notifier;

        // keep as members to avoid runtime re-allocs
        StreamSourceUpdates m_streamUpdates;
        RenderableVector m_tempRenderablesWithUpdatedVertexArrays;
    };
}
