//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DynamicQuad_SceneResources.h"

#include "ramses/client/Scene.h"
#include "ramses/client/Effect.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/OrthographicCamera.h"
#include "ramses/client/Texture2DBuffer.h"
#include "ramses/client/ArrayBuffer.h"
#include "ramses/client/TextureSampler.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/AttributeInput.h"

#include "TestRandom.h"
#include <memory>

namespace ramses::internal
{
    DynamicQuad_SceneResources::DynamicQuad_SceneResources(ramses::Scene& scene, const ScreenspaceQuad& screenspaceQuad)
        : DynamicQuad_Base(scene, screenspaceQuad)
        , m_textureBuffer   (m_scene.createTexture2DBuffer(ramses::ETextureFormat::RGB8, DynamicTextureWidth, DynamicTextureHeight, 1u))
        , m_textureSampler  (m_scene.createTextureSampler(ramses::ETextureAddressMode::Repeat, ramses::ETextureAddressMode::Repeat, ramses::ETextureSamplingMethod::Linear_MipMapLinear, ramses::ETextureSamplingMethod::Linear, *m_textureBuffer))
        , m_indices         (m_scene.createArrayBuffer( ramses::EDataType::UInt16, 4))
        , m_texCoords       (m_scene.createArrayBuffer(ramses::EDataType::Vector2F, 8))
        , m_vertexPos       (m_scene.createArrayBuffer( ramses::EDataType::Vector3F, 12))
    {
        m_renderGroup.addMeshNode(m_meshNode);

        m_geometryBinding.setIndices(*m_indices);
        m_geometryBinding.setInputBuffer(*m_appearance.getEffect().findAttributeInput("a_position"), *m_vertexPos);
        m_geometryBinding.setInputBuffer(*m_appearance.getEffect().findAttributeInput("a_texcoord"), *m_texCoords);

        m_appearance.setInputTexture(*m_appearance.getEffect().findUniformInput("u_texture"), *m_textureSampler);

        recreate();

        m_meshNode.setAppearance(m_appearance);
        m_meshNode.setIndexCount(4);
        m_appearance.setDrawMode(ramses::EDrawMode::TriangleStrip);
        m_meshNode.setGeometry(m_geometryBinding);
    }

    DynamicQuad_SceneResources::~DynamicQuad_SceneResources()
    {
        if (nullptr != m_textureSampler)
        {
            m_scene.destroy(*m_textureSampler);
        }
        if (nullptr != m_indices)
        {
            m_scene.destroy(*m_indices);
        }
        if (nullptr != m_texCoords)
        {
            m_scene.destroy(*m_texCoords);
        }
        if (nullptr != m_vertexPos)
        {
            m_scene.destroy(*m_vertexPos);
        }
    }

    void DynamicQuad_SceneResources::markSceneObjectsDestroyed()
    {
        m_textureSampler = nullptr;
        m_indices = nullptr;
        m_texCoords = nullptr;
        m_vertexPos = nullptr;
    }

    void DynamicQuad_SceneResources::recreate()
    {
        const std::array<uint16_t, 4> indicesData = { 0, 1, 3, 2 };

        // Vertex positions in normalized screen space, i.e. fraction of the screen (0.0f == bottom/left, 1.0f == top/right)
        const std::array<ramses::vec3f, 4u> vertexPositionsData
        {
            m_screenspaceQuad.getVertex(EScreenspaceQuadVertex::BottomLeft, 10u),
            m_screenspaceQuad.getVertex(EScreenspaceQuadVertex::BottomRight, 10u),
            m_screenspaceQuad.getVertex(EScreenspaceQuadVertex::TopRight, 10u),
            m_screenspaceQuad.getVertex(EScreenspaceQuadVertex::TopLeft, 10u)
        };

        std::array<ramses::vec2f, 4u> vertexTexcoordsData
        {
            ramses::vec2f{ 0.f, 0.f },
            ramses::vec2f{ 1.f, 0.f },
            ramses::vec2f{ 1.f, 1.f },
            ramses::vec2f{ 0.f, 1.f }
        };

        for (auto& tc : vertexTexcoordsData)
        {
            tc[0] += 0.01f * static_cast<float>(TestRandom::Get(0, 10));
            tc[1] += 0.01f * static_cast<float>(TestRandom::Get(0, 10));
        }

        m_indices->updateData(0u, 4, indicesData.data());
        m_vertexPos->updateData(0u, 4, vertexPositionsData.data());
        m_texCoords->updateData(0u, 4, vertexTexcoordsData.data());

        std::vector<uint8_t> rawData;
        rawData.resize(DynamicTextureWidth * DynamicTextureHeight * 3);

        for (uint32_t x = 0; x < DynamicTextureWidth; ++x)
        {
            for (uint32_t y = 0; y < DynamicTextureHeight; ++y)
            {
                rawData[3 * (y * DynamicTextureWidth + x) + 0] = static_cast<uint8_t>(TestRandom::Get(128, 255));
                rawData[3 * (y * DynamicTextureWidth + x) + 1] = 0u;
                rawData[3 * (y * DynamicTextureWidth + x) + 2] = static_cast<uint8_t>(TestRandom::Get(0, 128));
            }
        }

        m_textureBuffer->updateData(0, 0, 0, DynamicTextureWidth, DynamicTextureHeight, rawData.data());
    }
}
