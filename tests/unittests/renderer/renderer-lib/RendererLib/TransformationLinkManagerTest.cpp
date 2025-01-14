//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "internal/RendererLib/SceneLinksManager.h"
#include "internal/RendererLib/TransformationLinkCachedScene.h"
#include "internal/RendererLib/RendererScenes.h"
#include "internal/RendererLib/RendererEventCollector.h"
#include "SceneAllocateHelper.h"
#include "internal/Core/Utils/ThreadLocalLog.h"
#include "TestEqualHelper.h"
#include "glm/gtx/transform.hpp"

using namespace testing;

namespace ramses::internal
{
    class ATransformationLinkManager : public ::testing::Test
    {
    public:
        ATransformationLinkManager()
            : rendererScenes(rendererEventCollector)
            , sceneLinksManager(rendererScenes.getSceneLinksManager())
            , transformationLinkManager(const_cast<TransformationLinkManager&>(sceneLinksManager.getTransformationLinkManager()))
            , providerSceneId(3u)
            , consumerSceneId(4u)
            , providerScene(rendererScenes.createScene(SceneInfo(providerSceneId)))
            , consumerScene(rendererScenes.createScene(SceneInfo(consumerSceneId)))
            , providerSceneAllocator(providerScene)
            , consumerSceneAllocator(consumerScene)
            , providerNode(8u)
            , consumerNode(12u)
            , providerTransform(9u)
            , providerSlotHandle(55u)
            , consumerSlotHandle(66u)
            , providerId(33u)
            , consumerId(44u)
        {
            // caller is expected to have a display prefix for logs
            ThreadLocalLog::SetPrefix(1);

            providerSceneAllocator.allocateNode(0u, providerNode);
            consumerSceneAllocator.allocateNode(0u, consumerNode);

            providerSceneAllocator.allocateDataSlot({ EDataSlotType::TransformationProvider, providerId, providerNode, DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, providerSlotHandle);
            expectRendererEvent(ERendererEventType::SceneDataSlotProviderCreated, providerSceneId, providerId, SceneId(0u), DataSlotId(0u));
            consumerSceneAllocator.allocateDataSlot({ EDataSlotType::TransformationConsumer, consumerId, consumerNode, DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, consumerSlotHandle);
            expectRendererEvent(ERendererEventType::SceneDataSlotConsumerCreated, SceneId(0u), DataSlotId(0u), consumerSceneId, consumerId);
        }

    protected:
        void expectRendererEvent(ERendererEventType event, SceneId providerSId, DataSlotId pId, SceneId consumerSId, DataSlotId cId)
        {
            RendererEventVector events;
            RendererEventVector dummy;
            rendererEventCollector.appendAndConsumePendingEvents(dummy, events);

            ASSERT_EQ(1u, events.size());
            EXPECT_EQ(event, events.front().eventType);
            EXPECT_EQ(providerSId, events.front().providerSceneId);
            EXPECT_EQ(consumerSId, events.front().consumerSceneId);
            EXPECT_EQ(pId, events.front().providerdataId);
            EXPECT_EQ(cId, events.front().consumerdataId);
        }

        void expectRendererEvent(ERendererEventType event, SceneId consumerSId, DataSlotId cId)
        {
            RendererEventVector events;
            RendererEventVector dummy;
            rendererEventCollector.appendAndConsumePendingEvents(dummy, events);
            ASSERT_EQ(1u, events.size());
            EXPECT_EQ(event, events.front().eventType);
            EXPECT_EQ(consumerSId, events.front().consumerSceneId);
            EXPECT_EQ(cId, events.front().consumerdataId);
        }

        void expectRendererEventUnlinkedAndDestroyedSlot(EDataSlotType destroyedSlotType, SceneId sceneIdOfDestroyedSlot, DataSlotId destroyedSlotId, SceneId consumerSId, DataSlotId cId, SceneId consumerSceneId2 = SceneId(0u), DataSlotId consumerId2 = DataSlotId(0u))
        {
            RendererEventVector events;
            RendererEventVector dummy;
            rendererEventCollector.appendAndConsumePendingEvents(dummy, events);

            const bool hasTwoLinksRemoved = (consumerId2.getValue() != 0u);
            if (hasTwoLinksRemoved)
            {
                ASSERT_EQ(3u, events.size());
            }
            else
            {
                ASSERT_EQ(2u, events.size());
            }

            uint32_t eventIdx = 0u;

            EXPECT_EQ(ERendererEventType::SceneDataUnlinkedAsResultOfClientSceneChange, events[eventIdx].eventType);
            EXPECT_EQ(consumerSId, events[eventIdx].consumerSceneId);
            EXPECT_EQ(cId, events[eventIdx].consumerdataId);

            if (hasTwoLinksRemoved)
            {
                ++eventIdx;
                EXPECT_EQ(ERendererEventType::SceneDataUnlinkedAsResultOfClientSceneChange, events[eventIdx].eventType);
                EXPECT_EQ(consumerSceneId2, events[eventIdx].consumerSceneId);
                EXPECT_EQ(consumerId2, events[eventIdx].consumerdataId);
            }

            ++eventIdx;
            if (destroyedSlotType == EDataSlotType::TransformationProvider)
            {
                EXPECT_EQ(ERendererEventType::SceneDataSlotProviderDestroyed, events[eventIdx].eventType);
                EXPECT_EQ(sceneIdOfDestroyedSlot, events[eventIdx].providerSceneId);
                EXPECT_EQ(destroyedSlotId, events[eventIdx].providerdataId);
            }
            else
            {
                EXPECT_EQ(ERendererEventType::SceneDataSlotConsumerDestroyed, events[eventIdx].eventType);
                EXPECT_EQ(sceneIdOfDestroyedSlot, events[eventIdx].consumerSceneId);
                EXPECT_EQ(destroyedSlotId, events[eventIdx].consumerdataId);
            }
        }

        RendererEventCollector rendererEventCollector;
        RendererScenes rendererScenes;
        SceneLinksManager& sceneLinksManager;
        TransformationLinkManager& transformationLinkManager;

        const SceneId providerSceneId;
        const SceneId consumerSceneId;
        TransformationLinkCachedScene& providerScene;
        TransformationLinkCachedScene& consumerScene;
        SceneAllocateHelper providerSceneAllocator;
        SceneAllocateHelper consumerSceneAllocator;

        const NodeHandle providerNode;
        const NodeHandle consumerNode;
        const TransformHandle providerTransform;

        const DataSlotHandle providerSlotHandle;
        const DataSlotHandle consumerSlotHandle;
        const DataSlotId providerId;
        const DataSlotId consumerId;
    };

    TEST_F(ATransformationLinkManager, reportsNoLinkForInvalidNode)
    {
        EXPECT_FALSE(transformationLinkManager.nodeHasDataLinkToProvider(providerSceneId, NodeHandle()));
    }

    TEST_F(ATransformationLinkManager, reportsNoLinkForNonlinkedNode)
    {
        EXPECT_FALSE(transformationLinkManager.nodeHasDataLinkToProvider(providerSceneId, providerNode));
    }

    TEST_F(ATransformationLinkManager, reportsExistingLinkForNodeAssociatedWithLink)
    {
        EXPECT_TRUE(transformationLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle));
        EXPECT_TRUE(transformationLinkManager.nodeHasDataLinkToProvider(consumerSceneId, consumerNode));
    }

    TEST_F(ATransformationLinkManager, reportsNoLinkToProviderForProviderNode)
    {
        EXPECT_TRUE(transformationLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle));
        EXPECT_FALSE(transformationLinkManager.nodeHasDataLinkToProvider(providerSceneId, providerNode));
    }

    TEST_F(ATransformationLinkManager, retrievesWorldTransformationFromLinkedProviderNode)
    {
        EXPECT_TRUE(transformationLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle));
        providerSceneAllocator.allocateTransform(providerNode, providerTransform);
        const glm::vec3 transVec(1.f, 2.f, 3.f);
        providerScene.setTranslation(providerTransform, transVec);

        const auto expectedMat = glm::translate(transVec);
        const auto transMat = transformationLinkManager.getLinkedTransformationFromDataProvider(ETransformationMatrixType_World, consumerSceneId, consumerNode);

        expectMatrixFloatEqual(expectedMat, transMat);
    }

    TEST_F(ATransformationLinkManager, retrievesObjectTransformationFromLinkedProviderNode)
    {
        EXPECT_TRUE(transformationLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle));
        providerSceneAllocator.allocateTransform(providerNode, providerTransform);
        const glm::vec3 transVec(1.f, 2.f, 3.f);
        providerScene.setTranslation(providerTransform, transVec);

        const auto expectedMat = glm::translate(-transVec);
        const auto transMat = transformationLinkManager.getLinkedTransformationFromDataProvider(ETransformationMatrixType_Object, consumerSceneId, consumerNode);

        expectMatrixFloatEqual(expectedMat, transMat);
    }

    TEST_F(ATransformationLinkManager, reportsNoLinksForSceneWithRemovedLink)
    {
        EXPECT_TRUE(transformationLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle));
        EXPECT_TRUE(transformationLinkManager.removeDataLink(consumerSceneId, consumerSlotHandle));
        EXPECT_FALSE(transformationLinkManager.nodeHasDataLinkToProvider(consumerSceneId, consumerNode));
    }

    TEST_F(ATransformationLinkManager, reportsNoLinksForConsumerSceneIfDestroyed)
    {
        EXPECT_TRUE(transformationLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle));
        rendererScenes.destroyScene(consumerSceneId);
        EXPECT_FALSE(transformationLinkManager.nodeHasDataLinkToProvider(consumerSceneId, consumerNode));
    }

    TEST_F(ATransformationLinkManager, reportsNoLinksForConsumerSceneIfProviderSceneDestroyed)
    {
        EXPECT_TRUE(transformationLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle));
        rendererScenes.destroyScene(providerSceneId);
        EXPECT_FALSE(transformationLinkManager.nodeHasDataLinkToProvider(consumerSceneId, consumerNode));
    }

    TEST_F(ATransformationLinkManager, canLinkUnlinkAndLinkAgain)
    {
        EXPECT_TRUE(transformationLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle));
        EXPECT_TRUE(transformationLinkManager.removeDataLink(consumerSceneId, consumerSlotHandle));
        EXPECT_TRUE(transformationLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle));

        EXPECT_TRUE(transformationLinkManager.nodeHasDataLinkToProvider(consumerSceneId, consumerNode));
    }

    TEST_F(ATransformationLinkManager, removesLinkForSceneWithRemovedProviderSlot)
    {
        EXPECT_TRUE(transformationLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle));
        providerScene.releaseDataSlot(providerSlotHandle);
        expectRendererEventUnlinkedAndDestroyedSlot(EDataSlotType::TransformationProvider, providerSceneId, providerId, consumerSceneId, consumerId);
        EXPECT_FALSE(transformationLinkManager.nodeHasDataLinkToProvider(consumerSceneId, consumerNode));
    }

    TEST_F(ATransformationLinkManager, removesLinkForSceneWithRemovedConsumerSlot)
    {
        EXPECT_TRUE(transformationLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle));
        consumerScene.releaseDataSlot(consumerSlotHandle);
        expectRendererEventUnlinkedAndDestroyedSlot(EDataSlotType::TransformationConsumer, consumerSceneId, consumerId, consumerSceneId, consumerId);
        EXPECT_FALSE(transformationLinkManager.nodeHasDataLinkToProvider(consumerSceneId, consumerNode));
    }

    TEST_F(ATransformationLinkManager, createsTwoLinksSameProviderDifferentConsumers)
    {
        const NodeHandle consumerNode2 = consumerSceneAllocator.allocateNode();
        const DataSlotId consumerId2(999u);
        const DataSlotHandle slotHandle(43u);

        consumerSceneAllocator.allocateDataSlot({ EDataSlotType::TransformationConsumer, consumerId2, consumerNode2, DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, slotHandle);
        expectRendererEvent(ERendererEventType::SceneDataSlotConsumerCreated, SceneId(0u), DataSlotId(0u), consumerSceneId, consumerId2);

        EXPECT_TRUE(transformationLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle));
        EXPECT_TRUE(transformationLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, slotHandle));
        EXPECT_TRUE(transformationLinkManager.nodeHasDataLinkToProvider(consumerSceneId, consumerNode));
        EXPECT_TRUE(transformationLinkManager.nodeHasDataLinkToProvider(consumerSceneId, consumerNode2));
    }

    TEST_F(ATransformationLinkManager, removesAllLinksToProviderOnProviderSlotDestruction)
    {
        const NodeHandle consumerNode2 = consumerSceneAllocator.allocateNode();
        const DataSlotId consumerId2(999u);
        const DataSlotHandle slotHandle(43u);
        consumerSceneAllocator.allocateDataSlot({ EDataSlotType::TransformationConsumer, consumerId2, consumerNode2, DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, slotHandle);
        expectRendererEvent(ERendererEventType::SceneDataSlotConsumerCreated, SceneId(0u), DataSlotId(0u), consumerSceneId, consumerId2);

        EXPECT_TRUE(transformationLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle));
        EXPECT_TRUE(transformationLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, slotHandle));

        providerScene.releaseDataSlot(providerSlotHandle);
        expectRendererEventUnlinkedAndDestroyedSlot(EDataSlotType::TransformationProvider, providerSceneId, providerId, consumerSceneId, consumerId, consumerSceneId, consumerId2);

        EXPECT_FALSE(transformationLinkManager.nodeHasDataLinkToProvider(consumerSceneId, consumerNode));
        EXPECT_FALSE(transformationLinkManager.nodeHasDataLinkToProvider(consumerSceneId, consumerNode2));
    }

    TEST_F(ATransformationLinkManager, removingAllLinksToProviderOnProviderSlotDestructionDoesNotAffectOtherLinks)
    {
        const NodeHandle providerNode2 = providerSceneAllocator.allocateNode();
        const DataSlotId providerId2(999u);
        const DataSlotHandle slotHandle(43u);
        providerSceneAllocator.allocateDataSlot({ EDataSlotType::TransformationProvider, providerId2, providerNode2, DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, slotHandle);
        expectRendererEvent(ERendererEventType::SceneDataSlotProviderCreated, providerSceneId, providerId2, SceneId(0u), DataSlotId(0u));

        const NodeHandle consumerNode2 = consumerSceneAllocator.allocateNode();
        const DataSlotId consumerId2(888u);
        const DataSlotHandle slotHandle2(654u);
        consumerSceneAllocator.allocateDataSlot({ EDataSlotType::TransformationConsumer, consumerId2, consumerNode2, DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, slotHandle2);
        expectRendererEvent(ERendererEventType::SceneDataSlotConsumerCreated, SceneId(0u), DataSlotId(0u), consumerSceneId, consumerId2);

        EXPECT_TRUE(transformationLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle));
        EXPECT_TRUE(transformationLinkManager.createDataLink(providerSceneId, slotHandle, consumerSceneId, slotHandle2));

        providerScene.releaseDataSlot(providerSlotHandle);
        expectRendererEventUnlinkedAndDestroyedSlot(EDataSlotType::TransformationProvider, providerSceneId, providerId, consumerSceneId, consumerId);

        EXPECT_FALSE(transformationLinkManager.nodeHasDataLinkToProvider(consumerSceneId, consumerNode));
        EXPECT_TRUE(transformationLinkManager.nodeHasDataLinkToProvider(consumerSceneId, consumerNode2));
    }

    void markNodeTransformationClean(const TransformationLinkCachedScene& scene, NodeHandle node)
    {
        scene.updateMatrixCache(ETransformationMatrixType_World, node);
        EXPECT_FALSE(scene.isMatrixCacheDirty(ETransformationMatrixType_World, node));
    }

    TEST_F(ATransformationLinkManager, canPropagateTransformationDirtinessToConsumerScene)
    {
        EXPECT_TRUE(transformationLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle));
        markNodeTransformationClean(consumerScene, consumerNode);

        transformationLinkManager.propagateTransformationDirtinessToConsumers(providerSceneId, providerNode);
        EXPECT_TRUE(consumerScene.isMatrixCacheDirty(ETransformationMatrixType_World, consumerNode));
    }

    TEST_F(ATransformationLinkManager, propagatesTransformationDirtinessToConsumerWhenCreatingLink)
    {
        markNodeTransformationClean(consumerScene, consumerNode);

        EXPECT_TRUE(transformationLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle));
        EXPECT_TRUE(consumerScene.isMatrixCacheDirty(ETransformationMatrixType_World, consumerNode));
    }

    TEST_F(ATransformationLinkManager, propagatesTransformationDirtinessToConsumerWhenRemovingLink)
    {
        EXPECT_TRUE(transformationLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle));
        markNodeTransformationClean(consumerScene, consumerNode);

        EXPECT_TRUE(transformationLinkManager.removeDataLink(consumerSceneId, consumerSlotHandle));
        EXPECT_TRUE(consumerScene.isMatrixCacheDirty(ETransformationMatrixType_World, consumerNode));
    }

    TEST_F(ATransformationLinkManager, doesNotPropagateTransformationDirtinessToConsumerWhenRemovingLinkFailed)
    {
        EXPECT_TRUE(transformationLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle));
        markNodeTransformationClean(consumerScene, consumerNode);

        const NodeHandle consumerNode2 = consumerSceneAllocator.allocateNode();
        const DataSlotId consumerId2(888u);
        const DataSlotHandle slotHandle2(654u);
        consumerSceneAllocator.allocateDataSlot({ EDataSlotType::TransformationConsumer, consumerId2, consumerNode2, DataInstanceHandle::Invalid(), ResourceContentHash::Invalid(), TextureSamplerHandle() }, slotHandle2);
        expectRendererEvent(ERendererEventType::SceneDataSlotConsumerCreated, SceneId(0u), DataSlotId(0u), consumerSceneId, consumerId2);

        EXPECT_FALSE(transformationLinkManager.removeDataLink(consumerSceneId, slotHandle2));
        EXPECT_FALSE(consumerScene.isMatrixCacheDirty(ETransformationMatrixType_World, consumerNode));
    }

    TEST_F(ATransformationLinkManager, propagatesTransformationDirtinessToConsumerWhenNodeTransformationModified)
    {
        EXPECT_TRUE(transformationLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle));
        markNodeTransformationClean(consumerScene, consumerNode);

        providerSceneAllocator.allocateTransform(providerNode, providerTransform);
        providerScene.setTranslation(providerTransform, glm::vec3(1.f));
        EXPECT_TRUE(consumerScene.isMatrixCacheDirty(ETransformationMatrixType_World, consumerNode));
    }

    TEST_F(ATransformationLinkManager, propagatesTransformationDirtinessToConsumerWhenProviderSceneDestroyed)
    {
        EXPECT_TRUE(transformationLinkManager.createDataLink(providerSceneId, providerSlotHandle, consumerSceneId, consumerSlotHandle));
        markNodeTransformationClean(consumerScene, consumerNode);
        rendererScenes.destroyScene(providerSceneId);

        EXPECT_TRUE(consumerScene.isMatrixCacheDirty(ETransformationMatrixType_World, consumerNode));
    }
}
