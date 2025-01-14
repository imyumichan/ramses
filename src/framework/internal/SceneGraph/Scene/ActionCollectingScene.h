//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/Scene/ResourceChangeCollectingScene.h"
#include "internal/SceneGraph/Scene/SceneActionCollectionCreator.h"
#include "internal/SceneReferencing/SceneReferenceAction.h"

namespace ramses::internal
{
    class ActionCollectingScene : public ResourceChangeCollectingScene
    {
    public:
        explicit ActionCollectingScene(const SceneInfo& sceneInfo = SceneInfo());

        void                        preallocateSceneSize            (const SceneSizeInformation& sizeInfo) override;

        // Renderable allocation
        RenderableHandle            allocateRenderable              (NodeHandle nodeHandle, RenderableHandle handle) override;
        void                        releaseRenderable               (RenderableHandle renderableHandle) override;

        // Renderable data (stuff required for rendering)
        void                        setRenderableDataInstance       (RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance) override;
        void                        setRenderableStartIndex         (RenderableHandle renderableHandle, uint32_t startIndex) override;
        void                        setRenderableIndexCount         (RenderableHandle renderableHandle, uint32_t indexCount) override;
        void                        setRenderableVisibility         (RenderableHandle renderableHandle, EVisibilityMode visibility) override;
        void                        setRenderableRenderState        (RenderableHandle renderableHandle, RenderStateHandle stateHandle) override;
        void                        setRenderableInstanceCount      (RenderableHandle renderableHandle, uint32_t instanceCount) override;
        void                        setRenderableStartVertex        (RenderableHandle renderableHandle, uint32_t startVertex) override;
        void                        setRenderableUniformsDataInstanceAndState (RenderableHandle renderableHandle, DataInstanceHandle newDataInstance, RenderStateHandle stateHandle);

        // Render state
        RenderStateHandle           allocateRenderState             (RenderStateHandle stateHandle) override;
        void                        releaseRenderState              (RenderStateHandle stateHandle) override;
        void                        setRenderStateBlendFactors      (RenderStateHandle stateHandle, EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha) override;
        void                        setRenderStateBlendOperations   (RenderStateHandle stateHandle, EBlendOperation operationColor, EBlendOperation operationAlpha) override;
        void                        setRenderStateBlendColor        (RenderStateHandle stateHandle, const glm::vec4& color) override;
        void                        setRenderStateCullMode          (RenderStateHandle stateHandle, ECullMode cullMode) override;
        void                        setRenderStateDrawMode          (RenderStateHandle stateHandle, EDrawMode drawMode) override;
        void                        setRenderStateDepthFunc         (RenderStateHandle stateHandle, EDepthFunc func) override;
        void                        setRenderStateDepthWrite        (RenderStateHandle stateHandle, EDepthWrite flag) override;
        void                        setRenderStateScissorTest       (RenderStateHandle stateHandle, EScissorTest flag, const RenderState::ScissorRegion& region) override;
        void                        setRenderStateStencilFunc       (RenderStateHandle stateHandle, EStencilFunc func, uint8_t ref, uint8_t mask) override;
        void                        setRenderStateStencilOps        (RenderStateHandle stateHandle, EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass) override;
        void                        setRenderStateColorWriteMask    (RenderStateHandle stateHandle, ColorWriteMask colorMask) override;

        // Camera
        CameraHandle                allocateCamera                  (ECameraProjectionType type, NodeHandle nodeHandle, DataInstanceHandle dataInstance, CameraHandle handle) override;
        void                        releaseCamera                   (CameraHandle cameraHandle) override;

        // Creation/Deletion
        NodeHandle                  allocateNode                    (uint32_t childrenCount, NodeHandle handle) override;
        void                        releaseNode                     (NodeHandle nodeHandle) override;

        TransformHandle             allocateTransform               (NodeHandle nodeHandle, TransformHandle handle) override;
        void                        releaseTransform                (TransformHandle transform) override;

        // Parent-child relationship
        void                        addChildToNode                  (NodeHandle parent, NodeHandle child) override;
        void                        removeChildFromNode             (NodeHandle parent, NodeHandle child) override;

        // Transformation
        void                        setTranslation                  (TransformHandle handle, const glm::vec3& translation) override;
        void                        setRotation                     (TransformHandle handle, const glm::vec4& rotation, ERotationType rotationType) override;
        void                        setScaling                      (TransformHandle handle, const glm::vec3& scaling) override;


        DataLayoutHandle            allocateDataLayout              (const DataFieldInfoVector& dataFields, const ResourceContentHash& effectHash, DataLayoutHandle handle) override;
        void                        releaseDataLayout               (DataLayoutHandle layoutHandle) override;

        DataInstanceHandle          allocateDataInstance            (DataLayoutHandle finishedLayoutHandle, DataInstanceHandle instanceHandle) override;
        void                        releaseDataInstance             (DataInstanceHandle containerHandle) override;

        void                        setDataFloatArray               (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const float* data) override;
        void                        setDataVector2fArray            (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec2* data) override;
        void                        setDataVector3fArray            (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec3* data) override;
        void                        setDataVector4fArray            (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec4* data) override;
        void                        setDataBooleanArray             (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const bool* data) override;
        void                        setDataIntegerArray             (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const int32_t* data) override;
        void                        setDataVector2iArray            (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec2* data) override;
        void                        setDataVector3iArray            (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec3* data) override;
        void                        setDataVector4iArray            (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec4* data) override;
        void                        setDataMatrix22fArray           (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat2* data) override;
        void                        setDataMatrix33fArray           (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat3* data) override;
        void                        setDataMatrix44fArray           (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat4* data) override;
        void                        setDataResource                 (DataInstanceHandle containerHandle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, uint32_t instancingDivisor, uint16_t offsetWithinElementInBytes, uint16_t stride) override;
        void                        setDataTextureSamplerHandle     (DataInstanceHandle containerHandle, DataFieldHandle field, TextureSamplerHandle samplerHandle) override;
        void                        setDataReference                (DataInstanceHandle containerHandle, DataFieldHandle field, DataInstanceHandle dataRef) override;

        // Texture sampler description
        TextureSamplerHandle        allocateTextureSampler          (const TextureSampler& sampler, TextureSamplerHandle handle) override;
        void                        releaseTextureSampler           (TextureSamplerHandle handle) override;

        // Render groups
        RenderGroupHandle           allocateRenderGroup             (uint32_t renderableCount, uint32_t nestedGroupCount, RenderGroupHandle groupHandle) override;
        void                        releaseRenderGroup              (RenderGroupHandle groupHandle) override;
        void                        addRenderableToRenderGroup      (RenderGroupHandle groupHandle, RenderableHandle renderableHandle, int32_t order) override;
        void                        removeRenderableFromRenderGroup (RenderGroupHandle groupHandle, RenderableHandle renderableHandle) override;
        void                        addRenderGroupToRenderGroup     (RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, int32_t order) override;
        void                        removeRenderGroupFromRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild) override;

        RenderPassHandle            allocateRenderPass              (uint32_t renderGroupCount, RenderPassHandle handle) override;
        void                        releaseRenderPass               (RenderPassHandle handle) override;
        void                        setRenderPassClearColor         (RenderPassHandle passHandle, const glm::vec4& clearColor) override;
        void                        setRenderPassClearFlag          (RenderPassHandle passHandle, ClearFlags clearFlag) override;
        void                        setRenderPassCamera             (RenderPassHandle passHandle, CameraHandle cameraHandle) override;
        void                        setRenderPassRenderTarget       (RenderPassHandle passHandle, RenderTargetHandle targetHandle) override;
        void                        setRenderPassRenderOrder        (RenderPassHandle passHandle, int32_t renderOrder) override;
        void                        setRenderPassEnabled            (RenderPassHandle passHandle, bool isEnabled) override;
        void                        setRenderPassRenderOnce         (RenderPassHandle passHandle, bool enable) override;
        void                        retriggerRenderPassRenderOnce   (RenderPassHandle passHandle) override;
        void                        addRenderGroupToRenderPass      (RenderPassHandle passHandle, RenderGroupHandle groupHandle, int32_t order) override;
        void                        removeRenderGroupFromRenderPass (RenderPassHandle passHandle, RenderGroupHandle groupHandle) override;

        BlitPassHandle              allocateBlitPass                (RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle) override;
        void                        releaseBlitPass                 (BlitPassHandle passHandle) override;
        void                        setBlitPassRenderOrder          (BlitPassHandle passHandle, int32_t renderOrder) override;
        void                        setBlitPassEnabled              (BlitPassHandle passHandle, bool isEnabled) override;
        void                        setBlitPassRegions              (BlitPassHandle passHandle, const PixelRectangle& sourceRegion, const PixelRectangle& destinationRegion) override;

        PickableObjectHandle        allocatePickableObject          (DataBufferHandle geometryHandle, NodeHandle nodeHandle, PickableObjectId id, PickableObjectHandle pickableHandle) override;
        void                        releasePickableObject           (PickableObjectHandle pickableHandle) override;
        void                        setPickableObjectId             (PickableObjectHandle pickableHandle, PickableObjectId id) override;
        void                        setPickableObjectCamera         (PickableObjectHandle pickableHandle, CameraHandle cameraHandle) override;
        void                        setPickableObjectEnabled        (PickableObjectHandle pickableHandle, bool isEnabled) override;

        DataSlotHandle              allocateDataSlot                (const DataSlot& dataSlot, DataSlotHandle handle) override;
        void                        setDataSlotTexture              (DataSlotHandle handle, const ResourceContentHash& texture) override;
        void                        releaseDataSlot                 (DataSlotHandle handle) override;

        // Render targets
        RenderTargetHandle          allocateRenderTarget            (RenderTargetHandle targetHandle) override;
        void                        releaseRenderTarget             (RenderTargetHandle targetHandle) override;

        // Render buffers
        RenderBufferHandle          allocateRenderBuffer            (const RenderBuffer& renderBuffer, RenderBufferHandle handle) override;
        void                        releaseRenderBuffer             (RenderBufferHandle handle) override;
        void                        addRenderTargetRenderBuffer     (RenderTargetHandle targetHandle, RenderBufferHandle bufferHandle) override;

        // Data buffers
        DataBufferHandle            allocateDataBuffer              (EDataBufferType dataBufferType, EDataType dataType, uint32_t maximumSizeInBytes, DataBufferHandle handle) override;
        void                        releaseDataBuffer               (DataBufferHandle handle) override;
        void                        updateDataBuffer                (DataBufferHandle handle, uint32_t offsetInBytes, uint32_t dataSizeInBytes, const std::byte* data) override;

        TextureBufferHandle         allocateTextureBuffer           (EPixelStorageFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle) override;
        void                        releaseTextureBuffer            (TextureBufferHandle handle) override;
        void                        updateTextureBuffer             (TextureBufferHandle handle, uint32_t mipLevel, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const std::byte* data) override;

        SceneReferenceHandle        allocateSceneReference          (SceneId sceneId, SceneReferenceHandle handle) override;
        void                        releaseSceneReference           (SceneReferenceHandle handle) override;
        void                        requestSceneReferenceState      (SceneReferenceHandle handle, RendererSceneState state) override;
        void                        requestSceneReferenceFlushNotifications(SceneReferenceHandle handle, bool enable) override;
        void                        setSceneReferenceRenderOrder    (SceneReferenceHandle handle, int32_t renderOrder) override;

        [[nodiscard]] const SceneActionCollection& getSceneActionCollection() const;
        SceneActionCollection& getSceneActionCollection();

        void linkData(SceneReferenceHandle providerScene, DataSlotId providerId, SceneReferenceHandle consumerScene, DataSlotId consumerId);
        void unlinkData(SceneReferenceHandle consumerScene, DataSlotId consumerId);

        [[nodiscard]] const SceneReferenceActionVector& getSceneReferenceActions() const;
        void resetSceneReferenceActions();

    private:
        SceneActionCollection m_collection;
        SceneActionCollectionCreator m_creator;
        SceneReferenceActionVector m_sceneReferenceActions;
    };
}
