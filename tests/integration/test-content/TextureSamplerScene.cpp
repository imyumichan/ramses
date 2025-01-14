//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/TextureSamplerScene.h"
#include "ramses/client/MipLevelData.h"
#include "ramses/client/Scene.h"
#include "ramses/client/TextureSampler.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/Effect.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/Texture2DBuffer.h"
#include "ramses/client/RenderTargetDescription.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/OrthographicCamera.h"

namespace ramses
{
    class Texture2D;
}

namespace ramses::internal
{
    TextureSamplerScene::TextureSamplerScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
    {
        const uint8_t rgb8[] =
        {
            0xff,0xff,0xff,  0x00,0x00,0x00,
            0xff,0x00,0x00,  0x00,0x00,0xff,
        };
        const std::vector<MipLevelData> mipLevelData = { MipLevelData{ sizeof(rgb8), rgb8 } };

        if (state == EState::EState_ClientTexture)
        {
            const ramses::Texture2D* texture = m_scene.createTexture2D(ramses::ETextureFormat::RGB8, 2, 2, mipLevelData, false);
            m_sampler = m_scene.createTextureSampler(ramses::ETextureAddressMode::Repeat, ramses::ETextureAddressMode::Repeat, ramses::ETextureSamplingMethod::Nearest, ramses::ETextureSamplingMethod::Nearest, *texture);
        }
        else if (state == EState::EState_TextureBuffer)
        {
            ramses::Texture2DBuffer* texture = m_scene.createTexture2DBuffer(ramses::ETextureFormat::RGB8, 2, 2, 1);
            texture->updateData(0, 0, 0, 2, 2, rgb8);
            m_sampler = m_scene.createTextureSampler(ramses::ETextureAddressMode::Repeat, ramses::ETextureAddressMode::Repeat, ramses::ETextureSamplingMethod::Nearest, ramses::ETextureSamplingMethod::Nearest, *texture);
        }

        m_effect = getTestEffect("ramses-test-client-textured");

        const uint16_t indicesArray[] = { 0, 1, 2, 2, 1, 3 };
        const ramses::ArrayResource* indices = m_scene.createArrayResource(6u, indicesArray);
        const std::array<ramses::vec3f, 4u> vertexPositionsArray
        {
            ramses::vec3f{ -0.5f, -0.5f, 0.f },
            ramses::vec3f{  0.5f, -0.5f, 0.f },
            ramses::vec3f{ -0.5f,  0.5f, 0.f },
            ramses::vec3f{  0.5f,  0.5f, 0.f }
        };
        const ramses::ArrayResource* vertexPositions = m_scene.createArrayResource(4u, vertexPositionsArray.data());

        const std::array<ramses::vec2f, 4u> textureCoordsArray{ ramses::vec2f{0.f, 0.f}, ramses::vec2f{2.f, 0.f}, ramses::vec2f{0.f, 2.f}, ramses::vec2f{2.f, 2.f} };
        const ramses::ArrayResource* textureCoords = m_scene.createArrayResource(4u, textureCoordsArray.data());

        m_appearance = m_scene.createAppearance(*m_effect, "appearance");

        ramses::Geometry* geometry = m_scene.createGeometry(*m_effect, "triangle geometry");
        geometry->setIndices(*indices);
        geometry->setInputBuffer(*m_effect->findAttributeInput("a_position"), *vertexPositions);
        geometry->setInputBuffer(*m_effect->findAttributeInput("a_texcoord"), *textureCoords);

        if (state != EState_NoTextureSampler)
        {
            m_appearance->setInputTexture(*m_effect->findUniformInput("u_texture"), *m_sampler);
        }

        ramses::MeshNode* meshNode = m_scene.createMeshNode("quad");
        meshNode->setAppearance(*m_appearance);
        meshNode->setGeometry(*geometry);

        ramses::Node* transNode = m_scene.createNode();
        transNode->setTranslation({0.f, 0.f, -4.f});
        meshNode->setParent(*transNode);

        addMeshNodeToDefaultRenderGroup(*meshNode);

        setState(state);
    }

    void TextureSamplerScene::setState(uint32_t state)
    {
        const uint8_t rgb8[] =
        {
            0xff,0xff,0xff,  0xff,0x00,0xff,  0x00,0xff,0x00,
            0x00,0xff,0xff,  0xff,0xff,0x00,  0x00,0x00,0x00,
            0xff,0x00,0x00,  0xff,0x00,0xff,  0x00,0x00,0xff
        };

        switch (state)
        {
        case EState::EState_SetClientTexture:
        {
            const std::vector<MipLevelData> mipLevelData = { MipLevelData{ sizeof(rgb8), rgb8 } };
            const ramses::Texture2D* texture = m_scene.createTexture2D(ramses::ETextureFormat::RGB8, 3, 3, mipLevelData, false);
            m_sampler->setTextureData(*texture);
            break;
        }
        case EState::EState_SetTextureBuffer:
        {
            ramses::Texture2DBuffer* texture = m_scene.createTexture2DBuffer(ramses::ETextureFormat::RGB8, 3, 3, 1);
            texture->updateData(0, 0, 0, 3, 3, rgb8);
            m_sampler->setTextureData(*texture);
            break;
        }
        case EState::EState_SetRenderBuffer:
        {
            const ramses::RenderBuffer* buffer = m_scene.createRenderBuffer(16, 16, ramses::ERenderBufferFormat::RGBA8, ramses::ERenderBufferAccessMode::ReadWrite);
            ramses::RenderTargetDescription rtDesc;
            rtDesc.addRenderBuffer(*buffer);
            const auto rt = m_scene.createRenderTarget(rtDesc);
            auto rp = m_scene.createRenderPass();
            rp->setClearColor({0.f, 0.f, 1.f, 1.f});
            rp->setClearFlags(ramses::EClearFlag::All);
            rp->setRenderOrder(-1);
            auto camera = m_scene.createOrthographicCamera();
            camera->setViewport(0, 0, 16, 16);
            camera->setFrustum(-1, 1, -1, 1, 1, 10);
            rp->setCamera(*camera);
            rp->setRenderTarget(rt);
            m_sampler->setTextureData(*buffer);
            break;
        }

        case EState_SetTextureSampler:
        {
            const std::vector<MipLevelData> mipLevelData = { MipLevelData{ sizeof(rgb8), rgb8 } };

            const ramses::Texture2D* texture = m_scene.createTexture2D(ramses::ETextureFormat::RGB8, 3, 3, mipLevelData, false);
            m_sampler = m_scene.createTextureSampler(ramses::ETextureAddressMode::Repeat, ramses::ETextureAddressMode::Repeat, ramses::ETextureSamplingMethod::Nearest, ramses::ETextureSamplingMethod::Nearest, *texture);
            m_appearance->setInputTexture(*m_effect->findUniformInput("u_texture"), *m_sampler);
            break;
        }
        default:
            break;
        }

        m_scene.flush();
    }
}
