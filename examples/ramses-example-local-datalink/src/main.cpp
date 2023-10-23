//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/ramses-client.h"

#include "ramses/renderer/RamsesRenderer.h"
#include "ramses/renderer/DisplayConfig.h"
#include "ramses/renderer/RendererSceneControl.h"
#include "ramses/renderer/IRendererSceneControlEventHandler.h"
#include "ramses/renderer/IRendererEventHandler.h"
#include "ramses/client/DataObject.h"
#include "ramses/client/ramses-utils.h"
#include <unordered_set>
#include <cmath>
#include <chrono>
#include <memory>
#include <thread>
#include <unordered_set>
#include <array>

/**
 * @example ramses-example-local-datalink/src/main.cpp
 * @brief Local Data Link Example
 */

class RendererAndSceneStateEventHandler : public ramses::RendererEventHandlerEmpty, public ramses::RendererSceneControlEventHandlerEmpty
{
public:
    RendererAndSceneStateEventHandler(ramses::RendererSceneControl& sceneControlApi, ramses::RamsesRenderer& renderer)
        : m_sceneControlApi(sceneControlApi)
        , m_renderer(renderer)
    {
    }

    void dataProviderCreated(ramses::sceneId_t sceneId, ramses::dataProviderId_t /*dataProviderId*/) override
    {
        scenesWithCreatedProviderOrConsumer.insert(sceneId);
    }

    void dataConsumerCreated(ramses::sceneId_t sceneId, ramses::dataConsumerId_t /*dataConsumerId*/) override
    {
        scenesWithCreatedProviderOrConsumer.insert(sceneId);
    }

    void sceneStateChanged(ramses::sceneId_t sceneId, ramses::RendererSceneState state) override
    {
        if (state == ramses::RendererSceneState::Ready)
            mappedScenes.insert(sceneId);
    }

    void waitForSceneReady(ramses::Scene& scene)
    {
        const std::chrono::steady_clock::time_point timeoutTS = std::chrono::steady_clock::now() + std::chrono::seconds{ 5 };
        while (mappedScenes.count(scene.getSceneId()) == 0 && std::chrono::steady_clock::now() < timeoutTS)
        {
            scene.flush();
            m_renderer.doOneLoop();
            m_renderer.dispatchEvents(*this);
            m_sceneControlApi.dispatchEvents(*this);
        }
    }

    void waitForDataProviderOrConsumerCreated(ramses::sceneId_t sceneId)
    {
        while (scenesWithCreatedProviderOrConsumer.find(sceneId) == scenesWithCreatedProviderOrConsumer.end())
        {
            m_renderer.doOneLoop();
            m_sceneControlApi.dispatchEvents(*this);
        }
    }

    void windowClosed(ramses::displayId_t /*displayId*/) override
    {
        m_windowClosed = true;
    }

    bool isWindowClosed() const
    {
        return m_windowClosed;
    }

private:
    ramses::RendererSceneControl& m_sceneControlApi;
    ramses::RamsesRenderer& m_renderer;
    std::unordered_set<ramses::sceneId_t> mappedScenes;
    std::unordered_set<ramses::sceneId_t> scenesWithCreatedProviderOrConsumer;
    bool m_windowClosed = false;
};

// Helper struct for triangle scene
struct TriangleSceneInfo
{
public:
    // Node that will be used as transformation provider
    ramses::Node* translateNode;
    // Textures that will be used as texture provider
    std::array<ramses::Texture2D*, 2> textures;
    // Data object that will be used as data consumer
    ramses::DataObject* colorData;
    // Provider scene
    ramses::Scene* scene;
};

// Helper struct for quad scene
struct QuadSceneInfo
{
    // Consumer node that will use transformation from provider node
    ramses::Node* consumerNode;
    // Consumer texture sampler, that will use texture from provider
    ramses::TextureSampler* textureSampler;
    // Additional rotation chained after consumed transformation
    ramses::Node* rotateNode;
    // Data object that will be used as data provider
    ramses::DataObject* colorData;
    // Consumer scene
    ramses::Scene* scene;
};

std::unique_ptr<TriangleSceneInfo> createTriangleSceneContent(ramses::RamsesClient& client, ramses::sceneId_t sceneId)
{
    auto sceneInfo = std::make_unique<TriangleSceneInfo>();

    sceneInfo->scene = client.createScene(sceneId, "triangle scene");

    // every scene needs a render pass with camera
    auto* camera = sceneInfo->scene->createPerspectiveCamera("my camera");
    camera->setViewport(0, 0, 1280u, 480u);
    camera->setFrustum(19.f, 1280.f / 480.f, 0.1f, 1500.f);
    ramses::RenderPass* renderPass = sceneInfo->scene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlag::None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = sceneInfo->scene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    // prepare triangle geometry: vertex position array and index array
    const std::array<ramses::vec3f, 3u> vertexPositionsData{ ramses::vec3f{-0.25f, -0.125f, 0.f}, ramses::vec3f{0.25f, -0.125f, 0.f}, ramses::vec3f{0.f, 0.125f, 0.f} };
    ramses::ArrayResource* vertexPositions = sceneInfo->scene->createArrayResource(4u, vertexPositionsData.data());

    const std::array<uint16_t, 3u> indexData{ 0, 1, 2 };
    ramses::ArrayResource* indices = sceneInfo->scene->createArrayResource(3u, indexData.data());

    // create an appearance for red triangle
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-local-datalink.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-local-datalink.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    ramses::Effect* effect = sceneInfo->scene->createEffect(effectDesc, "glsl shader");
    ramses::Appearance* appearance = sceneInfo->scene->createAppearance(*effect, "triangle appearance");
    appearance->setCullingMode(ramses::ECullMode::Disabled);
    appearance->setDepthFunction(ramses::EDepthFunc::Always);

    ramses::Geometry* geometry = sceneInfo->scene->createGeometry(*effect, "triangle geometry");
    geometry->setIndices(*indices);

    std::optional<ramses::AttributeInput> positionsInput = effect->findAttributeInput("a_position");
    std::optional<ramses::UniformInput> colorInput = effect->findUniformInput("color");
    assert(positionsInput.has_value() && colorInput.has_value());
    geometry->setInputBuffer(*positionsInput, *vertexPositions);

    appearance->setInputValue(*colorInput, ramses::vec4f{ 1.0f, 0.0f, 0.3f, 1.0f });
    //bind input to data object
    sceneInfo->colorData = sceneInfo->scene->createDataObject(ramses::EDataType::Vector4F, "colorData");
    sceneInfo->colorData->setValue(ramses::vec4f{ 1.0f, 0.0f, 0.3f, 1.0f });
    appearance->bindInput(*colorInput, *sceneInfo->colorData);

    ramses::Node* rootTranslation = sceneInfo->scene->createNode("root scene translation node");
    rootTranslation->setTranslation({0.0f, 0.0f, -1.0f});

    sceneInfo->translateNode = sceneInfo->scene->createNode("triangle translation node");
    sceneInfo->translateNode->setParent(*rootTranslation);

    // create a mesh node to define the triangle with chosen appearance
    ramses::MeshNode* meshNode = sceneInfo->scene->createMeshNode("triangle mesh node");
    meshNode->setAppearance(*appearance);
    meshNode->setGeometry(*geometry);
    meshNode->setParent(*sceneInfo->translateNode);
    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroup->addMeshNode(*meshNode);

    // Create provided texture
    sceneInfo->textures[0] = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-example-local-datalink-texture.png", *sceneInfo->scene);
    sceneInfo->textures[1] = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-example-local-datalink-texture2.png", *sceneInfo->scene);

    return sceneInfo;
}

std::unique_ptr<QuadSceneInfo> createQuadSceneContent(ramses::RamsesClient& client, ramses::sceneId_t sceneId)
{
    auto sceneInfo = std::make_unique<QuadSceneInfo>();

    sceneInfo->scene = client.createScene(sceneId, "quad scene");

    // every scene needs a render pass with camera
    auto* camera = sceneInfo->scene->createPerspectiveCamera("my camera");
    camera->setViewport(0, 0, 1280u, 480u);
    camera->setFrustum(19.f, 1280.f / 480.f, 0.1f, 1500.f);
    ramses::RenderPass* renderPass = sceneInfo->scene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlag::None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = sceneInfo->scene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    // prepare triangle geometry: vertex position array and index array
    const std::array<ramses::vec3f, 4u> vertexPositionsArray{
        ramses::vec3f{-0.1f, -0.1f, 0.0f},
        ramses::vec3f{0.1f, -0.1f, 0.0f},
        ramses::vec3f{.1f, 0.1f, 0.0f},
        ramses::vec3f{-0.1f, 0.1f, 0.0f} };
    ramses::ArrayResource* vertexPositions = sceneInfo->scene->createArrayResource(4u, vertexPositionsArray.data());
    const std::array<ramses::vec2f, 4u> texCoordsArray{
        ramses::vec2f{1.0f, 1.0f},
        ramses::vec2f{0.0f, 1.0f},
        ramses::vec2f{0.0f, 0.0f},
        ramses::vec2f{1.0f, 0.0f} };
    ramses::ArrayResource* texCoords = sceneInfo->scene->createArrayResource(4u, texCoordsArray.data());
    const std::array<uint16_t, 6u> indicesArray{ 0, 1, 2, 2, 3, 0 };
    ramses::ArrayResource* indices = sceneInfo->scene->createArrayResource(6u, indicesArray.data());

    // create an appearance for red triangle
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-local-datalink-texturing.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-local-datalink-texturing.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    ramses::Effect* effect = sceneInfo->scene->createEffect(effectDesc, "glsl shader");
    ramses::Appearance* appearance = sceneInfo->scene->createAppearance(*effect, "quad appearance");
    appearance->setDepthFunction(ramses::EDepthFunc::Always);
    appearance->setCullingMode(ramses::ECullMode::Disabled);

    ramses::Geometry* geometry = sceneInfo->scene->createGeometry(*effect, "quad geometry");
    std::optional<ramses::AttributeInput> positionsInput = effect->findAttributeInput("a_position");
    std::optional<ramses::AttributeInput> texCoordsInput = effect->findAttributeInput("a_texcoord");
    std::optional<ramses::UniformInput>   colorInput     = effect->findUniformInput("color");
    std::optional<ramses::UniformInput>   textureInput   = effect->findUniformInput("textureSampler");
    assert(positionsInput.has_value() && texCoordsInput.has_value() && colorInput.has_value() && textureInput.has_value());
    geometry->setIndices(*indices);
    geometry->setInputBuffer(*positionsInput, *vertexPositions);
    geometry->setInputBuffer(*texCoordsInput, *texCoords);
    appearance->setInputValue(*colorInput, ramses::vec4f{ 1.0f, 1.0f, 1.0f, 1.0f });

    ramses::Texture2D* fallbackTexture = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-example-local-datalink-texture-fallback.png", *sceneInfo->scene);
    sceneInfo->textureSampler = sceneInfo->scene->createTextureSampler(ramses::ETextureAddressMode::Clamp, ramses::ETextureAddressMode::Clamp,
        ramses::ETextureSamplingMethod::Linear, ramses::ETextureSamplingMethod::Linear, *fallbackTexture);
    appearance->setInputTexture(*textureInput, *sceneInfo->textureSampler);

    sceneInfo->consumerNode = sceneInfo->scene->createNode("quad root node");
    sceneInfo->rotateNode = sceneInfo->scene->createNode("");

    // create a mesh node to define the triangle with chosen appearance
    ramses::MeshNode* meshNode = sceneInfo->scene->createMeshNode("quad mesh node");
    meshNode->setAppearance(*appearance);
    meshNode->setGeometry(*geometry);
    meshNode->setParent(*sceneInfo->rotateNode);

    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroup->addMeshNode(*meshNode);
    sceneInfo->rotateNode->setParent(*sceneInfo->consumerNode);

    //create data object for providing color info (not used by any appearance)
    sceneInfo->colorData = sceneInfo->scene->createDataObject(ramses::EDataType::Vector4F, "colorData");
    sceneInfo->colorData->setValue(ramses::vec4f{ 1.0f, 1.0f, 1.0f, 1.0f });

    return sceneInfo;
}

int main()
{
    //Ramses client
    ramses::RamsesFrameworkConfig config{ramses::EFeatureLevel_Latest};
    config.setRequestedRamsesShellType(ramses::ERamsesShellType::Console);  //needed for automated test of examples
    ramses::RamsesFramework framework(config);
    ramses::RamsesClient& client(*framework.createClient("ramses-local-datalink-example"));

    ramses::RendererConfig rendererConfig;
    ramses::RamsesRenderer& renderer(*framework.createRenderer(rendererConfig));

    ramses::DisplayConfig displayConfig;
    const ramses::displayId_t display = renderer.createDisplay(displayConfig);
    renderer.flush();
    auto& sceneControlAPI = *renderer.getSceneControlAPI();

    const ramses::sceneId_t triangleSceneId{1u};
    const ramses::sceneId_t quadSceneId{2u};
    const ramses::sceneId_t quadSceneId2{3u};
    std::unique_ptr<TriangleSceneInfo> triangleInfo = createTriangleSceneContent(client, triangleSceneId);
    std::unique_ptr<QuadSceneInfo> quadInfo = createQuadSceneContent(client, quadSceneId);
    std::unique_ptr<QuadSceneInfo> quadInfo2 = createQuadSceneContent(client, quadSceneId2);

    ramses::Scene* triangleScene = triangleInfo->scene;
    ramses::Scene* quadScene = quadInfo->scene;
    ramses::Scene* quadScene2 = quadInfo2->scene;
    ramses::Node* providerNode = triangleInfo->translateNode;
    ramses::Node* consumerNode = quadInfo->consumerNode;
    ramses::Node* consumerNode2 = quadInfo2->consumerNode;

    /// [Data Linking Example Client]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.

    // Transformation links
    ramses::dataProviderId_t transformationProviderId(14u);
    ramses::dataConsumerId_t transformationConsumerId(12u);
    ramses::dataConsumerId_t transformationConsumerId2(87u);

    triangleScene->createTransformationDataProvider(*providerNode, transformationProviderId);
    quadScene->createTransformationDataConsumer(*consumerNode, transformationConsumerId);
    quadScene2->createTransformationDataConsumer(*consumerNode2, transformationConsumerId2);

    //Data links
    ramses::dataProviderId_t dataProviderId(100u);
    ramses::dataConsumerId_t dataConsumerId(101u);

    triangleScene->createDataConsumer(*triangleInfo->colorData, dataConsumerId);
    quadScene->createDataProvider(*quadInfo->colorData, dataProviderId);

    //Texture links
    ramses::dataProviderId_t textureProviderId(200u);
    ramses::dataConsumerId_t textureConsumerId(201u);

    triangleScene->createTextureProvider(*triangleInfo->textures[0], textureProviderId);
    quadScene2->createTextureConsumer(*quadInfo2->textureSampler, textureConsumerId);

    /// [Data Linking Example Client]
    framework.connect();
    triangleScene->publish(ramses::EScenePublicationMode::LocalOnly);
    quadScene->publish(ramses::EScenePublicationMode::LocalOnly);
    quadScene2->publish(ramses::EScenePublicationMode::LocalOnly);

    const auto fbId = renderer.getDisplayFramebuffer(display);

    sceneControlAPI.setSceneMapping(triangleSceneId, display);
    sceneControlAPI.setSceneMapping(quadSceneId, display);
    sceneControlAPI.setSceneMapping(quadSceneId2, display);
    sceneControlAPI.flush();

    sceneControlAPI.setSceneDisplayBufferAssignment(triangleSceneId, fbId, 0);
    sceneControlAPI.setSceneDisplayBufferAssignment(quadSceneId, fbId, 1);
    sceneControlAPI.setSceneDisplayBufferAssignment(quadSceneId2, fbId, 2);

    sceneControlAPI.setSceneState(triangleSceneId, ramses::RendererSceneState::Rendered);
    sceneControlAPI.setSceneState(quadSceneId, ramses::RendererSceneState::Rendered);
    sceneControlAPI.setSceneState(quadSceneId2, ramses::RendererSceneState::Rendered);

    sceneControlAPI.flush();

    RendererAndSceneStateEventHandler eventHandler(sceneControlAPI, renderer);
    eventHandler.waitForSceneReady(*triangleScene);
    eventHandler.waitForSceneReady(*quadScene);
    eventHandler.waitForSceneReady(*quadScene2);
    eventHandler.waitForDataProviderOrConsumerCreated(triangleSceneId);
    eventHandler.waitForDataProviderOrConsumerCreated(quadSceneId);
    eventHandler.waitForDataProviderOrConsumerCreated(quadSceneId2);

    /// [Data Linking Example Renderer]
    // link transformation
    sceneControlAPI.linkData(triangleSceneId, transformationProviderId, quadSceneId, transformationConsumerId);
    sceneControlAPI.linkData(triangleSceneId, transformationProviderId, quadSceneId2, transformationConsumerId2);
    // link data
    sceneControlAPI.linkData(quadSceneId, dataProviderId, triangleSceneId, dataConsumerId);
    // link texture
    sceneControlAPI.linkData(triangleSceneId, textureProviderId, quadSceneId2, textureConsumerId);
    /// [Data Linking Example Renderer]

    sceneControlAPI.flush();

    // run animation
    uint32_t textureId = 0;
    uint64_t timeStamp = 0u;
    float rotationFactor = 0.f;
    while (!eventHandler.isWindowClosed())
    {
        renderer.dispatchEvents(eventHandler);
        if (timeStamp % 100 == 0)
        {
            textureId = (1 - textureId);
            triangleInfo->scene->updateTextureProvider(*triangleInfo->textures[textureId], textureProviderId);
        }

        triangleInfo->translateNode->setTranslation({std::sin(static_cast<float>(timeStamp) * 0.05f) * 0.2f, 0.0f, 0.0f});
        triangleInfo->scene->flush();
        rotationFactor += 1.f;
        quadInfo->rotateNode->setRotation({0.0f, 0.0f, rotationFactor}, ramses::ERotationType::Euler_XYZ);
        quadInfo->scene->flush();
        quadInfo2->rotateNode->setRotation({0.0f, rotationFactor, 0.0f}, ramses::ERotationType::Euler_XYZ);
        quadInfo2->scene->flush();

        quadInfo->colorData->setValue(ramses::vec4f{ std::sin(static_cast<float>(timeStamp) * 0.1f), 0.0f, 0.5f, 1.0f });

        renderer.doOneLoop();
        timeStamp++;
        std::this_thread::sleep_for(std::chrono::milliseconds{ 15u });
    }
}
