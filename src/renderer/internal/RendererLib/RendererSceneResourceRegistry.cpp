//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/RendererSceneResourceRegistry.h"

namespace ramses::internal
{
    RendererSceneResourceRegistry::RendererSceneResourceRegistry() = default;

    RendererSceneResourceRegistry::~RendererSceneResourceRegistry()
    {
        assert(m_renderBuffers.size() == 0u);
        assert(m_renderTargets.size() == 0u);
        assert(m_blitPasses.size() == 0u);
        assert(m_dataBuffers.size() == 0u);
        assert(m_textureBuffers.size() == 0u);
        assert(m_vertexArrays.size() == 0u);
    }

    void RendererSceneResourceRegistry::addRenderBuffer(RenderBufferHandle handle, DeviceResourceHandle deviceHandle, uint32_t size, bool writeOnly)
    {
        assert(!m_renderBuffers.contains(handle));
        m_renderBuffers.put(handle, { deviceHandle, size, writeOnly });
    }

    void RendererSceneResourceRegistry::removeRenderBuffer(RenderBufferHandle handle)
    {
        assert(m_renderBuffers.contains(handle));
        m_renderBuffers.remove(handle);
    }

    DeviceResourceHandle RendererSceneResourceRegistry::getRenderBufferDeviceHandle(RenderBufferHandle handle) const
    {
        assert(m_renderBuffers.contains(handle));
        return m_renderBuffers.get(handle)->deviceHandle;
    }

    uint32_t RendererSceneResourceRegistry::getRenderBufferByteSize(RenderBufferHandle handle) const
    {
        assert(m_renderBuffers.contains(handle));
        return m_renderBuffers.get(handle)->size;
    }

    void RendererSceneResourceRegistry::getAllRenderBuffers(RenderBufferHandleVector& renderBuffers) const
    {
        assert(renderBuffers.empty());
        renderBuffers.reserve(m_renderBuffers.size());
        for(const auto& renderBuffer : m_renderBuffers)
        {
            renderBuffers.push_back(renderBuffer.key);
        }
    }

    void RendererSceneResourceRegistry::addRenderTarget(RenderTargetHandle handle, DeviceResourceHandle deviceHandle)
    {
        assert(!m_renderTargets.contains(handle));
        m_renderTargets.put(handle, { deviceHandle });
    }

    void RendererSceneResourceRegistry::removeRenderTarget(RenderTargetHandle handle)
    {
        assert(m_renderTargets.contains(handle));
        m_renderTargets.remove(handle);
    }

    DeviceResourceHandle RendererSceneResourceRegistry::getRenderTargetDeviceHandle(RenderTargetHandle handle) const
    {
        return *m_renderTargets.get(handle);
    }

    void RendererSceneResourceRegistry::getAllRenderTargets(RenderTargetHandleVector& renderTargets) const
    {
        assert(renderTargets.empty());
        renderTargets.reserve(m_renderTargets.size());
        for(const auto& renderTarget : m_renderTargets)
        {
            renderTargets.push_back(renderTarget.key);
        }
    }

    void RendererSceneResourceRegistry::addBlitPass(BlitPassHandle handle, DeviceResourceHandle srcRenderTargetDeviceHandle, DeviceResourceHandle dstRenderTargetDeviceHandle)
    {
        assert(!m_blitPasses.contains(handle));
        BlitPassEntry bpEntry;
        bpEntry.sourceRenderTargetDeviceHandle = srcRenderTargetDeviceHandle;
        bpEntry.destinationRenderTargetDeviceHandle = dstRenderTargetDeviceHandle;
        m_blitPasses.put(handle, bpEntry);
    }

    void RendererSceneResourceRegistry::removeBlitPass(BlitPassHandle handle)
    {
        assert(m_blitPasses.contains(handle));
        m_blitPasses.remove(handle);
    }

    void RendererSceneResourceRegistry::getBlitPassDeviceHandles(BlitPassHandle handle, DeviceResourceHandle& srcRenderTargetDeviceHandle, DeviceResourceHandle& dstRenderTargetDeviceHandle) const
    {
        const BlitPassEntry* const bpEntry = m_blitPasses.get(handle);
        assert(bpEntry != nullptr);
        srcRenderTargetDeviceHandle = bpEntry->sourceRenderTargetDeviceHandle;
        dstRenderTargetDeviceHandle = bpEntry->destinationRenderTargetDeviceHandle;
    }

    void RendererSceneResourceRegistry::getAllBlitPasses(BlitPassHandleVector& blitPasses) const
    {
        assert(blitPasses.empty());
        blitPasses.reserve(m_blitPasses.size());
        for(const auto& blitPass : m_blitPasses)
        {
            blitPasses.push_back(blitPass.key);
        }
    }

    void RendererSceneResourceRegistry::addDataBuffer(DataBufferHandle handle, DeviceResourceHandle deviceHandle, EDataBufferType dataBufferType, uint32_t size)
    {
        assert(!m_dataBuffers.contains(handle));
        m_dataBuffers.put(handle, { deviceHandle, size, dataBufferType });
    }

    void RendererSceneResourceRegistry::removeDataBuffer(DataBufferHandle handle)
    {
        assert(m_dataBuffers.contains(handle));
        m_dataBuffers.remove(handle);
    }

    DeviceResourceHandle RendererSceneResourceRegistry::getDataBufferDeviceHandle(DataBufferHandle handle) const
    {
        assert(m_dataBuffers.contains(handle));
        return m_dataBuffers.get(handle)->deviceHandle;
    }

    EDataBufferType RendererSceneResourceRegistry::getDataBufferType(DataBufferHandle handle) const
    {
        assert(m_dataBuffers.contains(handle));
        return m_dataBuffers.get(handle)->dataBufferType;
    }

    void RendererSceneResourceRegistry::getAllDataBuffers(DataBufferHandleVector& dataBuffers) const
    {
        assert(dataBuffers.empty());
        dataBuffers.reserve(m_dataBuffers.size());
        for(const auto& db : m_dataBuffers)
        {
            dataBuffers.push_back(db.key);
        }
    }

    void RendererSceneResourceRegistry::addTextureBuffer(TextureBufferHandle handle, DeviceResourceHandle deviceHandle, EPixelStorageFormat format, uint32_t size)
    {
        assert(!m_textureBuffers.contains(handle));
        m_textureBuffers.put(handle, { deviceHandle, size, format });
    }

    void RendererSceneResourceRegistry::removeTextureBuffer(TextureBufferHandle handle)
    {
        assert(m_textureBuffers.contains(handle));
        m_textureBuffers.remove(handle);
    }

    DeviceResourceHandle RendererSceneResourceRegistry::getTextureBufferDeviceHandle(TextureBufferHandle handle) const
    {
        assert(m_textureBuffers.contains(handle));
        return m_textureBuffers.get(handle)->deviceHandle;
    }

    EPixelStorageFormat RendererSceneResourceRegistry::getTextureBufferFormat(TextureBufferHandle handle) const
    {
        assert(m_textureBuffers.contains(handle));
        return m_textureBuffers.get(handle)->format;
    }

    uint32_t RendererSceneResourceRegistry::getTextureBufferByteSize(TextureBufferHandle handle) const
    {
        assert(m_textureBuffers.contains(handle));
        return m_textureBuffers.get(handle)->size;
    }

    void RendererSceneResourceRegistry::getAllTextureBuffers(TextureBufferHandleVector& textureBuffers) const
    {
        assert(textureBuffers.empty());
        textureBuffers.reserve(m_textureBuffers.size());
        for (const auto& tb : m_textureBuffers)
        {
            textureBuffers.push_back(tb.key);
        }
    }

    void RendererSceneResourceRegistry::addVertexArray(RenderableHandle renderableHandle, DeviceResourceHandle deviceHandle)
    {
        assert(!m_vertexArrays.contains(renderableHandle));
        m_vertexArrays.put(renderableHandle, deviceHandle);
    }

    void RendererSceneResourceRegistry::removeVertexArray(RenderableHandle renderableHandle)
    {
        assert(m_vertexArrays.contains(renderableHandle));
        m_vertexArrays.remove(renderableHandle);
    }

    DeviceResourceHandle RendererSceneResourceRegistry::getVertexArrayDeviceHandle(RenderableHandle renderableHandle) const
    {
        assert(m_vertexArrays.contains(renderableHandle));
        return *m_vertexArrays.get(renderableHandle);
    }

    void RendererSceneResourceRegistry::getAllVertexArrayRenderables(RenderableVector& vertexArrayRenderables) const
    {
        assert(vertexArrayRenderables.empty());
        vertexArrayRenderables.reserve(m_vertexArrays.size());
        for (const auto& va : m_vertexArrays)
            vertexArrayRenderables.push_back(va.key);
    }

    uint32_t RendererSceneResourceRegistry::getSceneResourceMemoryUsage(ESceneResourceType resourceType) const
    {
        uint32_t result = 0;
        switch (resourceType)
        {
        case ESceneResourceType_RenderBuffer_WriteOnly:
            for (const auto& rbEntry : m_renderBuffers)
            {
                if(rbEntry.value.writeOnly)
                    result += rbEntry.value.size;
            }
            break;
        case ESceneResourceType_RenderBuffer_ReadWrite:
            for (const auto& rbEntry : m_renderBuffers)
            {
                if (!rbEntry.value.writeOnly)
                    result += rbEntry.value.size;
            }
            break;
        case ESceneResourceType_DataBuffer:
            for (const auto& dataBuffer : m_dataBuffers)
            {
                result += dataBuffer.value.size;
            }
            break;
        case ESceneResourceType_TextureBuffer:
            for (const auto& texBuffer : m_textureBuffers)
            {
                result += texBuffer.value.size;
            }
            break;
        default:
            assert(false && "Invalid scene resource type");
            break;
        }

        return result;
    }
}
