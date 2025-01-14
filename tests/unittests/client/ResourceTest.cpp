//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/ArrayResource.h"
#include "ramses/client/Texture2D.h"
#include "ramses/client/Texture3D.h"
#include "ramses/client/TextureCube.h"
#include "ramses/client/EffectDescription.h"
#include "ramses/client/TextureSwizzle.h"
#include "ramses/client/SceneObjectIterator.h"
#include "ramses/client/ramses-utils.h"

#include "impl/Texture2DImpl.h"
#include "impl/Texture3DImpl.h"
#include "impl/TextureCubeImpl.h"
#include "impl/ArrayResourceImpl.h"
#include "impl/EffectImpl.h"
#include "RamsesObjectTestTypes.h"
#include "ClientTestUtils.h"
#include "internal/SceneGraph/Resource/IResource.h"
#include "internal/PlatformAbstraction/PlatformMemory.h"
#include "internal/Components/ManagedResource.h"
#include "impl/RamsesClientImpl.h"
#include "UnsafeTestMemoryHelpers.h"

#include <thread>
#include <string_view>

namespace ramses::internal
{
    using namespace testing;

    class AResourceTestClient : public LocalTestClientWithScene, public ::testing::Test
    {
    public:
        AResourceTestClient()
        {
            m_oldLogLevel = ramses::internal::CONTEXT_HLAPI_CLIENT.getLogLevel();
            ramses::internal::CONTEXT_HLAPI_CLIENT.setLogLevel(ELogLevel::Trace);
        }
        ~AResourceTestClient() override {
            ramses::internal::CONTEXT_HLAPI_CLIENT.setLogLevel(m_oldLogLevel);
        }
        ramses::internal::ManagedResource getCreatedResource(const ramses::internal::ResourceContentHash& hash)
        {
            return client.impl().getClientApplication().getResource(hash);
        }
        ELogLevel m_oldLogLevel;
    };

    //##############################################################
    //##############   Texture tests ###############################
    //##############################################################

    TEST_F(AResourceTestClient, createTextureAndDestroyManually)
    {
        const uint8_t data[4 * 10 * 12] = {};
        const std::vector<MipLevelData> mipLevelData{ MipLevelData(sizeof(data), data) };
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::RGBA8, 10, 12, mipLevelData, false, {}, "name");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_TRUE(m_scene.destroy(*texture));
    }

    TEST_F(AResourceTestClient, createTextureWithMipMaps)
    {
        const uint8_t data[2 * 2 * 4] = {};
        std::vector<MipLevelData> mipLevelData;
        mipLevelData.emplace_back(2 * 2 * 4, data);
        mipLevelData.emplace_back(1 * 1 * 4, data);
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::RGBA8, 2, 2, mipLevelData, false, {}, "name");
        ASSERT_TRUE(nullptr != texture);
    }

    TEST_F(AResourceTestClient, createTextureAndCheckWidthHeight)
    {
        const uint8_t data[4 * 10 * 12] = {};
        const std::vector<MipLevelData> mipLevelData{ MipLevelData(sizeof(data), data) };
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::RGBA8, 10, 12, mipLevelData, false, {}, "name");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(10u, texture->getWidth());
        EXPECT_EQ(12u, texture->getHeight());
    }

    TEST_F(AResourceTestClient, createTextureAndCheckFormat)
    {
        const uint8_t data[3 * 10 * 12] = {};
        const std::vector<MipLevelData> mipLevelData{ MipLevelData(sizeof(data), data) };
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::RGB8, 10, 12, mipLevelData, false, {}, "name");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(ETextureFormat::RGB8, texture->getTextureFormat());
    }

    TEST_F(AResourceTestClient, createsTextureWithDefaultSwizzle)
    {
        const uint8_t data[3 * 10 * 12] = {};
        const std::vector<MipLevelData> mipLevelData{ MipLevelData(sizeof(data), data) };
        TextureSwizzle swizzle;
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::RGB8, 10, 12, mipLevelData, false, swizzle, "name");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(ETextureChannelColor::Red, swizzle.channelRed);
        EXPECT_EQ(ETextureChannelColor::Green, swizzle.channelGreen);
        EXPECT_EQ(ETextureChannelColor::Blue, swizzle.channelBlue);
        EXPECT_EQ(ETextureChannelColor::Alpha, swizzle.channelAlpha);
    }

    TEST_F(AResourceTestClient, createsTextureWithNonDefaultSwizzle)
    {
        TextureSwizzle swizzle = { ETextureChannelColor::Blue, ETextureChannelColor::Alpha, ETextureChannelColor::Red, ETextureChannelColor::Green };
        const uint8_t data[3 * 10 * 12] = {};
        const std::vector<MipLevelData> mipLevelData{ MipLevelData(sizeof(data), data) };
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::RGB8, 10, 12, mipLevelData, false, swizzle, "name");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(swizzle.channelRed, texture->getTextureSwizzle().channelRed);
        EXPECT_EQ(swizzle.channelGreen, texture->getTextureSwizzle().channelGreen);
        EXPECT_EQ(swizzle.channelBlue, texture->getTextureSwizzle().channelBlue);
        EXPECT_EQ(swizzle.channelAlpha, texture->getTextureSwizzle().channelAlpha);
    }

    TEST_F(AResourceTestClient, createTextureWithProvidedMipsButNotFullChain)
    {
        const uint8_t data[4 * 4 * 4] = {};

        std::vector<MipLevelData> mipLevelData;
        mipLevelData.emplace_back(static_cast<uint32_t>(sizeof(data)), data);
        mipLevelData.emplace_back(2 * 2 * 4, data);

        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::RGBA8, 4u, 4u, mipLevelData, false, {}, "name");
        ASSERT_TRUE(nullptr != texture);
    }

    TEST_F(AResourceTestClient, createTextureWithMoreMipsThanExpected)
    {
        const uint8_t data[1] = {};

        std::vector<MipLevelData> mipLevelData;
        mipLevelData.emplace_back(static_cast<uint32_t>(sizeof(data)), data);
        mipLevelData.emplace_back(static_cast<uint32_t>(sizeof(data)), data);

        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::RGBA8, 1u, 1u, mipLevelData, false, {}, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createTextureWithNullMipMapData)
    {
        const uint8_t data[4 * 4 * 4] = {};
        const uint8_t* dataNullPtr{nullptr};

        std::vector<MipLevelData> mipLevelData;
        mipLevelData.emplace_back(static_cast<uint32_t>(sizeof(data)), dataNullPtr);

        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::RGBA8, 4u, 4u, mipLevelData, false, {}, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createTextureWithZeroSizeMipMapData)
    {
        const uint8_t data[4 * 4 * 4] = {};

        std::vector<MipLevelData> mipLevelData;
        mipLevelData.emplace_back(0u, data);

        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::RGBA8, 4u, 4u, mipLevelData, false, {}, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createTextureWithNullOrZeroSizeLowerMipMapData)
    {
        const uint8_t data[2 * 2 * 4] = {};
        const uint8_t* dataNullPtr{nullptr};
        {
            std::vector<MipLevelData> mipLevelData;
            mipLevelData.emplace_back(static_cast<uint32_t>(sizeof(data)), data);
            mipLevelData.emplace_back(0u, data);
            Texture2D* texture = m_scene.createTexture2D(ETextureFormat::RGBA8, 2u, 2u, mipLevelData, false, {}, "name");
            EXPECT_EQ(nullptr, texture);
        }
        {
            std::vector<MipLevelData> mipLevelData;
            mipLevelData.emplace_back(static_cast<uint32_t>(sizeof(data)), data);
            mipLevelData.emplace_back(static_cast<uint32_t>(sizeof(data)), dataNullPtr);
            Texture2D* texture = m_scene.createTexture2D(ETextureFormat::RGBA8, 2u, 2u, mipLevelData, false, {}, "name");
            EXPECT_EQ(nullptr, texture);
        }
    }

    TEST_F(AResourceTestClient, createTextureWithWrongSizeOfLowerMipMapData)
    {
        const uint8_t data[2 * 2 * 4] = {};
        std::vector<MipLevelData> mipLevelData;
        mipLevelData.emplace_back(2 * 2 * 4, data);
        mipLevelData.emplace_back(1 * 1 * 2, data);
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::RGBA8, 2u, 2u, mipLevelData, false, {}, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createTextureOfZeroSize)
    {
        const uint8_t data[4 * 10 * 12] = {};
        const std::vector<MipLevelData> mipLevelData{ MipLevelData(sizeof(data), data) };
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::RGBA8, 0, 0, mipLevelData, false, {}, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createTextureWithNoMipData)
    {
        std::vector < MipLevelData> mipLevelData;
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::RGBA8, 1, 1, mipLevelData, false, {}, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createTextureWithoutName)
    {
        const uint8_t data[4 * 10 * 12] = {};
        const std::vector<MipLevelData> mipLevelData{ MipLevelData(sizeof(data), data) };
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::RGBA8, 10, 12, mipLevelData, false, {}, {});
        ASSERT_TRUE(nullptr != texture);
        EXPECT_TRUE(texture->getName().empty());
    }

    TEST_F(AResourceTestClient, createTextureCheckHashIsValid)
    {
        const uint8_t data[4 * 10 * 12] = {};
        const std::vector<MipLevelData> mipLevelData{ MipLevelData(sizeof(data), data) };
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::RGBA8, 10, 12, mipLevelData, false, {}, "name");
        ASSERT_TRUE(nullptr != texture);

        const ramses::internal::ResourceContentHash hash = texture->impl().getLowlevelResourceHash();
        EXPECT_TRUE(hash.isValid());
    }

    TEST_F(AResourceTestClient, createTextureCheckHashIsUnique)
    {
        uint8_t data[4 * 10 * 12] = {};
        data[20] = 48;
        const std::vector<MipLevelData> mipLevelData{ MipLevelData(sizeof(data), data) };
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::RGBA8, 10, 12, mipLevelData, false, {}, "name");
        ASSERT_TRUE(nullptr != texture);

        const ramses::internal::ResourceContentHash hash = texture->impl().getLowlevelResourceHash();
        uint8_t data2[4 * 10 * 12] = {};
        data[20] = 42;
        const std::vector<MipLevelData> mipLevelData2{ MipLevelData(sizeof(data2), data2) };
        Texture2D* texture2 = m_scene.createTexture2D(ETextureFormat::RGBA8, 10, 12, mipLevelData2, false, {}, "name");
        ASSERT_TRUE(nullptr != texture2);

        const ramses::internal::ResourceContentHash hash2 = texture2->impl().getLowlevelResourceHash();
        EXPECT_TRUE(hash != hash2);
    }

    template <typename... Ts>
    std::array<std::byte, sizeof...(Ts)> make_byte_array(Ts&&... args) noexcept
    {
        return {std::byte(std::forward<Ts>(args))...};
    }

    TEST_F(AResourceTestClient, createTextureRGBA_AndCheckTexels)
    {
        const auto data = make_byte_array(1, 2, 3, 4, 5, 6, 7, 8);
        const std::vector<MipLevelData> mipLevelData{ MipLevelData(static_cast<uint32_t>(data.size()), data.data()) };
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::RGBA8, 2, 1, mipLevelData, false, {}, {});
        ASSERT_TRUE(nullptr != texture);
        ramses::internal::ManagedResource res = getCreatedResource(texture->impl().getLowlevelResourceHash());

        ASSERT_EQ(data.size(), res->getDecompressedDataSize());
        EXPECT_EQ(data, res->getResourceData().span());
    }

    TEST_F(AResourceTestClient, createTextureRGB_AndCheckTexels)
    {
        const auto data = make_byte_array(1, 2, 3, 4, 5, 6);
        const std::vector<MipLevelData> mipLevelData{ MipLevelData(static_cast<uint32_t>(data.size()), data.data()) };
        Texture2D* texture = m_scene.createTexture2D(ETextureFormat::RGB8, 2, 1, mipLevelData, false, {}, {});
        ASSERT_TRUE(nullptr != texture);

        ramses::internal::ManagedResource res = getCreatedResource(texture->impl().getLowlevelResourceHash());

        ASSERT_EQ(data.size(), res->getDecompressedDataSize());
        EXPECT_EQ(data, res->getResourceData().span());
    }

    TEST_F(AResourceTestClient, createTextureRGBWithMips_AndCheckTexels)
    {
        const auto data0 = make_byte_array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
        const auto data1 = make_byte_array(13, 14, 15);
        const std::vector<MipLevelData> mipLevelData = { MipLevelData{ static_cast<uint32_t>(data0.size()), data0.data() }, MipLevelData{ static_cast<uint32_t>(data1.size()), data1.data() } };
        const Texture2D* texture = m_scene.createTexture2D(ETextureFormat::RGB8, 2, 2, mipLevelData, false, {}, {});
        ASSERT_TRUE(nullptr != texture);

        const ramses::internal::ManagedResource res = getCreatedResource(texture->impl().getLowlevelResourceHash());

        ASSERT_EQ(sizeof(data0) + sizeof(data1), res->getDecompressedDataSize());
        EXPECT_EQ(data0, res->getResourceData().span().subspan(0, sizeof(data0)));
        EXPECT_EQ(data1, res->getResourceData().span().subspan(sizeof(data0)));
    }

    //##############################################################
    //##############    Cube Texture tests #########################
    //##############################################################

    TEST_F(AResourceTestClient, createCubeTextureAndDestroyManually)
    {
        const std::byte data[4 * 10 * 10] = {};
        std::vector<CubeMipLevelData> mipLevelData = { CubeMipLevelData(sizeof(data), data, data, data, data, data, data) };
        TextureCube* texture = m_scene.createTextureCube(ETextureFormat::RGBA8, 10, mipLevelData, false);
        ASSERT_TRUE(nullptr != texture);
        EXPECT_TRUE(m_scene.destroy(*texture));
    }

    TEST_F(AResourceTestClient, createCubeTextureWithMipMaps)
    {
        const std::byte data[2 * 2 * 4] = {};
        std::vector<CubeMipLevelData> mipLevelData;
        mipLevelData.emplace_back(2 * 2 * 4, data, data, data, data, data, data);
        mipLevelData.emplace_back(1 * 1 * 4, data, data, data, data, data, data);
        TextureCube* texture = m_scene.createTextureCube(ETextureFormat::RGBA8, 2, mipLevelData, false, {}, "name");
        ASSERT_TRUE(nullptr != texture);
    }

    TEST_F(AResourceTestClient, createCubeTextureAndCheckWidthHeight)
    {
        const std::byte data[4 * 10 * 10] = {};
        std::vector<CubeMipLevelData> mipLevelData = { CubeMipLevelData(sizeof(data), data, data, data, data, data, data) };
        TextureCube* texture = m_scene.createTextureCube(ETextureFormat::RGBA8, 10, mipLevelData, false, {}, "name");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(10u, texture->getSize());
    }

    TEST_F(AResourceTestClient, createCubeTextureAndCheckFormat)
    {
        const std::byte data[3 * 10 * 10] = {};
        std::vector<CubeMipLevelData> mipLevelData = { CubeMipLevelData(sizeof(data), data, data, data, data, data, data) };
        TextureCube* texture = m_scene.createTextureCube(ETextureFormat::RGB8, 10, mipLevelData, false, {}, "name");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(ETextureFormat::RGB8, texture->getTextureFormat());
    }

    TEST_F(AResourceTestClient, createsCubeTextureWithDefaultSwizzle)
    {
        TextureSwizzle swizzle;
        const std::byte data[4 * 10 * 10] = {};
        std::vector<CubeMipLevelData> mipLevelData = { CubeMipLevelData(sizeof(data), data, data, data, data, data, data) };
        TextureCube* texture = m_scene.createTextureCube(ETextureFormat::RGBA8, 10, mipLevelData, false, swizzle, "name");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(ETextureChannelColor::Red, swizzle.channelRed);
        EXPECT_EQ(ETextureChannelColor::Green, swizzle.channelGreen);
        EXPECT_EQ(ETextureChannelColor::Blue, swizzle.channelBlue);
        EXPECT_EQ(ETextureChannelColor::Alpha, swizzle.channelAlpha);
    }

    TEST_F(AResourceTestClient, createsCubeTextureWithNonDefaultSwizzle)
    {
        TextureSwizzle swizzle = { ETextureChannelColor::Blue, ETextureChannelColor::Alpha, ETextureChannelColor::Red, ETextureChannelColor::Green };
        const std::byte data[4 * 10 * 10] = {};
        std::vector<CubeMipLevelData> mipLevelData = { CubeMipLevelData(sizeof(data), data, data, data, data, data, data) };
        TextureCube* texture = m_scene.createTextureCube(ETextureFormat::RGBA8, 10, mipLevelData, false, swizzle, "name");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(swizzle.channelRed, texture->getTextureSwizzle().channelRed);
        EXPECT_EQ(swizzle.channelGreen, texture->getTextureSwizzle().channelGreen);
        EXPECT_EQ(swizzle.channelBlue, texture->getTextureSwizzle().channelBlue);
        EXPECT_EQ(swizzle.channelAlpha, texture->getTextureSwizzle().channelAlpha);
    }

    TEST_F(AResourceTestClient, createCubeTextureOfZeroSize)
    {
        const std::byte data[4 * 10 * 10] = {};
        std::vector<CubeMipLevelData> mipLevelData = { CubeMipLevelData(sizeof(data), data, data, data, data, data, data) };
        TextureCube* texture = m_scene.createTextureCube(ETextureFormat::RGBA8, 0, mipLevelData, false, {}, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createCubeTextureOfZeroDataSize)
    {
        const std::byte data[4 * 10 * 10] = {};
        std::vector<CubeMipLevelData> mipLevelData = { CubeMipLevelData(0, data, data, data, data, data, data) };
        TextureCube* texture = m_scene.createTextureCube(ETextureFormat::RGBA8, 10, mipLevelData, false, {}, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createCubeTextureWithNoMipData)
    {
        std::vector<CubeMipLevelData> mipLevelData;
        TextureCube* texture = m_scene.createTextureCube(ETextureFormat::RGBA8, 10, mipLevelData, false, {}, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createCubeTextureWithoutName)
    {
        const std::byte data[4 * 10 * 10] = {};
        std::vector<CubeMipLevelData> mipLevelData = { CubeMipLevelData(sizeof(data), data, data, data, data, data, data) };
        TextureCube* texture = m_scene.createTextureCube(ETextureFormat::RGBA8, 10, mipLevelData, false, {}, {});
        ASSERT_TRUE(nullptr != texture);
        EXPECT_TRUE(texture->getName().empty());
    }

    TEST_F(AResourceTestClient, createCubeTextureRGBA_AndCheckTexels)
    {
        const auto dataArray = make_byte_array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        const auto* data = dataArray.data();
        std::vector<CubeMipLevelData> mipLevelData = { CubeMipLevelData(static_cast<uint32_t>(dataArray.size()), data, data, data, data, data, data) };
        TextureCube* texture = m_scene.createTextureCube(ETextureFormat::RGBA8, 2, mipLevelData, false);
        ASSERT_TRUE(nullptr != texture);
        ramses::internal::ManagedResource res = getCreatedResource(texture->impl().getLowlevelResourceHash());

        ASSERT_EQ(dataArray.size() * 6u, res->getDecompressedDataSize());
        for (uint32_t i = 0u; i < 6u; ++i)
            EXPECT_EQ(dataArray, res->getResourceData().span().subspan(i*dataArray.size(), dataArray.size()));
    }

    TEST_F(AResourceTestClient, createCubeTextureRGB_AndCheckTexels)
    {
        const auto dataArray = make_byte_array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
        const auto* data = dataArray.data();
        std::vector<CubeMipLevelData> mipLevelData = { CubeMipLevelData(static_cast<uint32_t>(dataArray.size()), data, data, data, data, data, data) };
        TextureCube* texture = m_scene.createTextureCube(ETextureFormat::RGB8, 2, mipLevelData, false);
        ASSERT_TRUE(nullptr != texture);
        ramses::internal::ManagedResource res = getCreatedResource(texture->impl().getLowlevelResourceHash());

        ASSERT_EQ(dataArray.size() * 6u, res->getDecompressedDataSize());
        for (uint32_t i = 0u; i < 6u; ++i)
            EXPECT_EQ(dataArray, res->getResourceData().span().subspan(i*dataArray.size(), dataArray.size()));
    }

    TEST_F(AResourceTestClient, createCubeTextureRGBWithPerFaceDataAndMips_AndCheckTexels)
    {
        const auto data0px = make_byte_array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
        const auto data1px = make_byte_array(13, 14, 15);
        const auto data0nx = make_byte_array(10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120);
        const auto data1nx = make_byte_array(130, 140, 150);
        const auto data0py = make_byte_array(11, 21, 31, 41, 51, 61, 71, 81, 91, 101, 111, 121);
        const auto data1py = make_byte_array(131, 141, 151);
        const auto data0ny = make_byte_array(12, 22, 32, 42, 52, 62, 72, 82, 92, 102, 112, 122);
        const auto data1ny = make_byte_array(132, 142, 152);
        const auto data0pz = make_byte_array(13, 23, 33, 43, 53, 63, 73, 83, 93, 103, 113, 123 );
        const auto data1pz = make_byte_array(133, 143, 153);
        const auto data0nz = make_byte_array(14, 24, 34, 44, 54, 64, 74, 84, 94, 104, 114, 124 );
        const auto data1nz = make_byte_array(134, 144, 154);

        std::vector<CubeMipLevelData> mipLevelData =
        {
            CubeMipLevelData{ static_cast<uint32_t>(data0px.size()), data0px.data(), data0nx.data(), data0py.data(), data0ny.data(), data0pz.data(), data0nz.data() },
            CubeMipLevelData{ static_cast<uint32_t>(data1px.size()), data1px.data(), data1nx.data(), data1py.data(), data1ny.data(), data1pz.data(), data1nz.data() }
        };
        const TextureCube* texture = m_scene.createTextureCube(ETextureFormat::RGB8, 2u, mipLevelData, false, {}, {});
        ASSERT_TRUE(nullptr != texture);

        const ramses::internal::ManagedResource res = getCreatedResource(texture->impl().getLowlevelResourceHash());

        ASSERT_EQ(6u * (data0px.size() + data1px.size()), res->getDecompressedDataSize());

        // mips are bundled together per face:
        // - facePX mip0, facePX mip1 .. face PX mipN, face NX mips, face PY .. face NZ
        size_t dataStart = 0;
        EXPECT_EQ(data0px, res->getResourceData().span().subspan(dataStart, data0px.size()));
        dataStart += data0px.size();
        EXPECT_EQ(data1px, res->getResourceData().span().subspan(dataStart, data1px.size()));
        dataStart += data1px.size();

        EXPECT_EQ(data0nx, res->getResourceData().span().subspan(dataStart, data0nx.size()));
        dataStart += data0nx.size();
        EXPECT_EQ(data1nx, res->getResourceData().span().subspan(dataStart, data1nx.size()));
        dataStart += data1nx.size();

        EXPECT_EQ(data0py, res->getResourceData().span().subspan(dataStart, data0py.size()));
        dataStart += data0py.size();
        EXPECT_EQ(data1py, res->getResourceData().span().subspan(dataStart, data1py.size()));
        dataStart += data1py.size();

        EXPECT_EQ(data0ny, res->getResourceData().span().subspan(dataStart, data0ny.size()));
        dataStart += data0ny.size();
        EXPECT_EQ(data1ny, res->getResourceData().span().subspan(dataStart, data1ny.size()));
        dataStart += data1ny.size();

        EXPECT_EQ(data0pz, res->getResourceData().span().subspan(dataStart, data0pz.size()));
        dataStart += data0pz.size();
        EXPECT_EQ(data1pz, res->getResourceData().span().subspan(dataStart, data1pz.size()));
        dataStart += data1pz.size();

        EXPECT_EQ(data0nz, res->getResourceData().span().subspan(dataStart, data0nz.size()));
        dataStart += data0nz.size();
        EXPECT_EQ(data1nz, res->getResourceData().span().subspan(dataStart, data1nz.size()));
    }

    TEST_F(AResourceTestClient, createTextureCubeWithProvidedMipsButNotFullChain)
    {
        const std::byte data[4 * 4 * 4] = {};

        std::vector<CubeMipLevelData> mipLevelData;
        mipLevelData.emplace_back(static_cast<uint32_t>(sizeof(data)), data, data, data, data, data, data);
        mipLevelData.emplace_back(2 * 2 * 4, data, data, data, data, data, data);

        TextureCube* texture = m_scene.createTextureCube(ETextureFormat::RGBA8, 4u, mipLevelData, false, {}, "name");
        EXPECT_NE(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createTextureCubeWithMoreMipsThanExpected)
    {
        const std::byte data[1] = {};

        std::vector<CubeMipLevelData> mipLevelData;
        mipLevelData.emplace_back(static_cast<uint32_t>(sizeof(data)), data, data, data, data, data, data);
        mipLevelData.emplace_back(static_cast<uint32_t>(sizeof(data)), data, data, data, data, data, data);

        TextureCube* texture = m_scene.createTextureCube(ETextureFormat::RGBA8, 1u, mipLevelData, false, {}, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createTextureCubeWithNullMipMapData)
    {
        const std::byte data[4 * 4 * 4] = {};

        std::vector<CubeMipLevelData> mipLevelData;
        mipLevelData.emplace_back(static_cast<uint32_t>(sizeof(data)), data, data, nullptr, data, data, data);

        TextureCube* texture = m_scene.createTextureCube(ETextureFormat::RGBA8, 4u, mipLevelData, false, {}, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createTextureCubeWithZeroSizeMipMapData)
    {
        const std::byte data[4 * 4 * 4] = {};

        std::vector<CubeMipLevelData> mipLevelData;
        mipLevelData.emplace_back(0u, data, data, data, data, data, data);

        TextureCube* texture = m_scene.createTextureCube(ETextureFormat::RGBA8, 4u, mipLevelData, false, {}, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createTextureCubeWithNullOrZeroSizeLowerMipMapData)
    {
        const std::byte data[2 * 2 * 4] = {};
        {
            std::vector<CubeMipLevelData> mipLevelData;
            mipLevelData.emplace_back(static_cast<uint32_t>(sizeof(data)), data, data, data, data, data, data);
            mipLevelData.emplace_back(0u, data, data, data, data, data, data);
            TextureCube* texture = m_scene.createTextureCube(ETextureFormat::RGBA8, 2u, mipLevelData, false, {}, "name");
            EXPECT_EQ(nullptr, texture);
        }
        {
            std::vector<CubeMipLevelData> mipLevelData;
            mipLevelData.emplace_back(static_cast<uint32_t>(sizeof(data)), data, data, data, data, data, data);
            mipLevelData.emplace_back(static_cast<uint32_t>(sizeof(data)), data, data, data, data, nullptr, data);
            TextureCube* texture = m_scene.createTextureCube(ETextureFormat::RGBA8, 2u, mipLevelData, false, {}, "name");
            EXPECT_EQ(nullptr, texture);
        }
    }

    TEST_F(AResourceTestClient, createTextureCubeWithWrongSizeOfLowerMipMapData)
    {
        const std::byte data[2 * 2 * 4] = {};
        std::vector<CubeMipLevelData> mipLevelData;
        mipLevelData.emplace_back(2 * 2 * 4, data, data, data, data, data, data);
        mipLevelData.emplace_back(1 * 1 * 2, data, data, data, data, data, data);
        TextureCube* texture = m_scene.createTextureCube(ETextureFormat::RGBA8, 2u, mipLevelData, false, {}, "name");
        EXPECT_EQ(nullptr, texture);
    }

    //##############################################################
    //##############    3D texture tests ###########################
    //##############################################################

    TEST_F(AResourceTestClient, create3DTextureAndDestroyManually)
    {
        const uint8_t data[4 * 10 * 12 * 14] = {};
        const std::vector<MipLevelData> mipLevelData{ MipLevelData(static_cast<uint32_t>(sizeof(data)), data) };
        Texture3D* texture = m_scene.createTexture3D(ETextureFormat::RGBA8, 10, 12, 14, mipLevelData, false, "name");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_TRUE(m_scene.destroy(*texture));
    }

    TEST_F(AResourceTestClient, create3DTextureWithMipMaps)
    {
        const uint8_t data[2 * 2 * 2 * 4] = {};
        std::vector<MipLevelData> mipLevelData;
        mipLevelData.emplace_back(2 * 2 * 2 * 4, data);
        mipLevelData.emplace_back(1 * 1 * 1 * 4, data);
        Texture3D* texture = m_scene.createTexture3D(ETextureFormat::RGBA8, 2u, 2u, 2u, mipLevelData, false, "name");
        ASSERT_TRUE(nullptr != texture);
    }

    TEST_F(AResourceTestClient, create3DTextureAndCheckWidthHeightDepth)
    {
        const uint8_t data[4 * 10 * 12 * 14] = {};
        const std::vector<MipLevelData> mipLevelData{ MipLevelData(static_cast<uint32_t>(sizeof(data)), data) };
        Texture3D* texture = m_scene.createTexture3D(ETextureFormat::RGBA8, 10, 12, 14, mipLevelData, false, "name");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(10u, texture->getWidth());
        EXPECT_EQ(12u, texture->getHeight());
        EXPECT_EQ(14u, texture->getDepth());
    }

    TEST_F(AResourceTestClient, create3DTextureAndCheckFormat)
    {
        const uint8_t data[3 * 10 * 12 * 14] = {};
        const std::vector<MipLevelData> mipLevelData{ MipLevelData(static_cast<uint32_t>(sizeof(data)), data) };
        Texture3D* texture = m_scene.createTexture3D(ETextureFormat::RGB8, 10, 12, 14, mipLevelData, false, "name");
        ASSERT_TRUE(nullptr != texture);
        EXPECT_EQ(ETextureFormat::RGB8, texture->getTextureFormat());
    }

    TEST_F(AResourceTestClient, create3DTextureOfZeroSize)
    {
        const uint8_t data[4 * 10 * 12 * 14] = {};
        const std::vector<MipLevelData> mipLevelData{ MipLevelData(static_cast<uint32_t>(sizeof(data)), data) };
        Texture3D* texture = m_scene.createTexture3D(ETextureFormat::RGBA8, 0, 0, 0, mipLevelData, false, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, create3DTextureWithNoMipData)
    {
        const std::vector<MipLevelData> mipLevelData;
        Texture3D* texture = m_scene.createTexture3D(ETextureFormat::RGBA8, 1, 1, 1, mipLevelData, false, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, create3DTextureWithoutName)
    {
        const uint8_t data[4 * 10 * 12 * 14] = {};
        const std::vector<MipLevelData> mipLevelData{ MipLevelData(static_cast<uint32_t>(sizeof(data)), data) };
        Texture3D* texture = m_scene.createTexture3D(ETextureFormat::RGBA8, 10, 12, 14, mipLevelData, false, {});
        ASSERT_TRUE(nullptr != texture);
        EXPECT_TRUE(texture->getName().empty());
    }

    TEST_F(AResourceTestClient, create3DTextureCheckHashIsValid)
    {
        const uint8_t data[4 * 10 * 12 * 14] = {};
        const std::vector<MipLevelData> mipLevelData{ MipLevelData(static_cast<uint32_t>(sizeof(data)), data) };
        Texture3D* texture = m_scene.createTexture3D(ETextureFormat::RGBA8, 10, 12, 14, mipLevelData, false, "name");
        ASSERT_TRUE(nullptr != texture);

        const ramses::internal::ResourceContentHash hash = texture->impl().getLowlevelResourceHash();
        EXPECT_TRUE(hash.isValid());
    }

    TEST_F(AResourceTestClient, create3DTextureCheckHashIsUnique)
    {
        uint8_t data[4 * 10 * 12 * 14] = {};
        data[20] = 48;
        const std::vector<MipLevelData> mipLevelData1{ MipLevelData(static_cast<uint32_t>(sizeof(data)), data) };
        Texture3D* texture = m_scene.createTexture3D(ETextureFormat::RGBA8, 10, 12, 14, mipLevelData1, false, "name");
        ASSERT_TRUE(nullptr != texture);

        const ramses::internal::ResourceContentHash hash = texture->impl().getLowlevelResourceHash();
        uint8_t data2[4 * 10 * 12 * 14] = {};
        data[20] = 42;
        const std::vector<MipLevelData> mipLevelData2{ MipLevelData(sizeof(data2), data2) };
        Texture3D* texture2 = m_scene.createTexture3D(ETextureFormat::RGBA8, 10, 12, 14, mipLevelData2, false, "name");
        ASSERT_TRUE(nullptr != texture2);

        const ramses::internal::ResourceContentHash hash2 = texture2->impl().getLowlevelResourceHash();
        EXPECT_TRUE(hash != hash2);
    }

    TEST_F(AResourceTestClient, create3DTextureRGBA_AndCheckTexels)
    {
        const auto data = make_byte_array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        const std::vector<MipLevelData> mipLevelData{ MipLevelData(static_cast<uint32_t>(data.size()), data.data()) };
        Texture3D* texture = m_scene.createTexture3D(ETextureFormat::RGBA8, 2, 1, 2, mipLevelData, false, {});

        ASSERT_TRUE(nullptr != texture);
        ramses::internal::ManagedResource res = getCreatedResource(texture->impl().getLowlevelResourceHash());

        ASSERT_EQ(data.size(), res->getDecompressedDataSize());
        EXPECT_EQ(data, res->getResourceData().span());
    }

    TEST_F(AResourceTestClient, create3DTextureRGB_AndCheckTexels)
    {
        const auto data = make_byte_array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
        const std::vector<MipLevelData> mipLevelData{ MipLevelData(static_cast<uint32_t>(data.size()), data.data()) };
        Texture3D* texture = m_scene.createTexture3D(ETextureFormat::RGB8, 2, 1, 2, mipLevelData, false, {});

        ASSERT_TRUE(nullptr != texture);
        ramses::internal::ManagedResource res = getCreatedResource(texture->impl().getLowlevelResourceHash());

        ASSERT_EQ(data.size(), res->getDecompressedDataSize());
        EXPECT_EQ(data, res->getResourceData().span());
    }

    TEST_F(AResourceTestClient, create3DTextureRGBWithMips_AndCheckTexels)
    {
        const auto data0 = make_byte_array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120);
        const auto data1 = make_byte_array(13, 14, 15);
        const std::vector<MipLevelData> mipLevelData = { MipLevelData{ static_cast<uint32_t>(data0.size()), data0.data() }, MipLevelData{ static_cast<uint32_t>(data1.size()), data1.data() } };
        const Texture3D* texture = m_scene.createTexture3D(ETextureFormat::RGB8, 2u, 2u, 2u, mipLevelData, false, {});
        ASSERT_TRUE(nullptr != texture);

        const ramses::internal::ManagedResource res = getCreatedResource(texture->impl().getLowlevelResourceHash());

        ASSERT_EQ(data0.size() + data1.size(), res->getDecompressedDataSize());
        EXPECT_EQ(data0, res->getResourceData().span().subspan(0, data0.size()));
        EXPECT_EQ(data1, res->getResourceData().span().subspan(data0.size()));
    }

    TEST_F(AResourceTestClient, createTexture3DWithProvidedMipsButNotFullChain)
    {
        const uint8_t data[4 * 4 * 4 * 4] = {};

        std::vector<MipLevelData> mipLevelData;
        mipLevelData.emplace_back(static_cast<uint32_t>(sizeof(data)), data);
        mipLevelData.emplace_back(2 * 2 * 2 * 4, data);

        Texture3D* texture = m_scene.createTexture3D(ETextureFormat::RGBA8, 4u, 4u, 4u, mipLevelData, false, "name");
        EXPECT_NE(nullptr, texture);
    }

    TEST_F(AResourceTestClient, createTexture3DWithMoreMipsThanExpected)
    {
        const uint8_t data[1] = {};

        std::vector<MipLevelData> mipLevelData;
        mipLevelData.emplace_back(static_cast<uint32_t>(sizeof(data)), data);
        mipLevelData.emplace_back(static_cast<uint32_t>(sizeof(data)), data);

        Texture3D* texture = m_scene.createTexture3D(ETextureFormat::RGBA8, 1u, 1u, 1u, mipLevelData, false, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, create3DTextureWithNullMipMapData)
    {
        const uint8_t data[2 * 2 * 2 * 4] = {};
        const uint8_t* dataNullPtr{nullptr};

        std::vector<MipLevelData> mipLevelData;
        mipLevelData.emplace_back(static_cast<uint32_t>(sizeof(data)), dataNullPtr);

        Texture3D* texture = m_scene.createTexture3D(ETextureFormat::RGBA8, 2u, 2u, 2u, mipLevelData, false, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, create3DTextureWithZeroSizeMipMapData)
    {
        const uint8_t data[2 * 2 * 2 * 4] = {};

        std::vector<MipLevelData> mipLevelData;
        mipLevelData.emplace_back(0, data);

        Texture3D* texture = m_scene.createTexture3D(ETextureFormat::RGBA8, 2u, 2u, 2u, mipLevelData, false, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, create3DTextureWithNullOrZeroSizeLowerMipMapData)
    {
        const uint8_t data[2 * 2 * 2 * 4] = {};
        const uint8_t* dataNullPtr{nullptr};
        {
            std::vector<MipLevelData> mipLevelData;
            mipLevelData.emplace_back(static_cast<uint32_t>(sizeof(data)), data);
            mipLevelData.emplace_back(0u, data);
            Texture3D* texture = m_scene.createTexture3D(ETextureFormat::RGBA8, 2u, 2u, 2u, mipLevelData, false, "name");
            EXPECT_EQ(nullptr, texture);
        }
        {
            std::vector<MipLevelData> mipLevelData;
            mipLevelData.emplace_back(static_cast<uint32_t>(sizeof(data)), data);
            mipLevelData.emplace_back(static_cast<uint32_t>(sizeof(data)), dataNullPtr);
            Texture3D* texture = m_scene.createTexture3D(ETextureFormat::RGBA8, 2u, 2u, 2u, mipLevelData, false, "name");
            EXPECT_EQ(nullptr, texture);
        }
    }

    TEST_F(AResourceTestClient, create3DTextureWithWrongSizeOfLowerMipMapData)
    {
        const uint8_t data[2 * 2 * 2 * 4] = {};
        std::vector<MipLevelData> mipLevelData;
        mipLevelData.emplace_back(2 * 2 * 2 * 4, data);
        mipLevelData.emplace_back(1 * 1 * 1 * 2, data);
        Texture3D* texture = m_scene.createTexture3D(ETextureFormat::RGBA8, 2u, 2u, 2u, mipLevelData, false, "name");
        EXPECT_EQ(nullptr, texture);
    }

    TEST_F(AResourceTestClient, create3DTextureWithCompressedFormatCanBeCreatedWithArbitraryNonZeroDataSize)
    {
        const uint8_t data[2 * 2 * 2 * 4] = {};
        std::vector<MipLevelData> mipLevelData;
        mipLevelData.emplace_back(1, data);
        mipLevelData.emplace_back(1, data);
        Texture3D* texture = m_scene.createTexture3D(ETextureFormat::ASTC_RGBA_4x4, 2u, 2u, 2u, mipLevelData, false, "name");
        EXPECT_TRUE(nullptr != texture);
    }

    //##############################################################
    //##############    Array resource tests #######################
    //##############################################################

    TEST_F(AResourceTestClient, creatingResourcesWithSameDataInternallyRefersToSameResourceHash)
    {
        const vec2f data[2] = { vec2f{1.f,2.f}, vec2f{3.f,4.f} };
        auto a = m_scene.createArrayResource(2, data);
        auto b = m_scene.createArrayResource(2, data);

        ASSERT_EQ(a->impl().getLowlevelResourceHash(), b->impl().getLowlevelResourceHash());
    }

    TEST_F(AResourceTestClient, creatingResourcesWithSameDataInternallySharesData)
    {
        const vec2f data[2] = { vec2f{1.f,2.f}, vec2f{3.f,4.f} };
        auto a = m_scene.createArrayResource(2, data);
        auto b = m_scene.createArrayResource(2, data);
        ramses::internal::ManagedResource resourceA = client.impl().getResource(a->impl().getLowlevelResourceHash());
        ramses::internal::ManagedResource resourceB = client.impl().getResource(b->impl().getLowlevelResourceHash());
        ASSERT_EQ(resourceA, resourceB);
    }

    TEST_F(AResourceTestClient, creatingResourcesWithSameDataContentInternallySharesData)
    {
        const vec2f data1[2] = { vec2f{1.f,2.f}, vec2f{3.f,4.f} };
        const vec2f data2[2] = { vec2f{1.f,2.f}, vec2f{3.f,4.f} };
        auto a = m_scene.createArrayResource(2, data1);
        auto b = m_scene.createArrayResource(2, data2);
        ramses::internal::ManagedResource resourceA = client.impl().getResource(a->impl().getLowlevelResourceHash());
        ramses::internal::ManagedResource resourceB = client.impl().getResource(b->impl().getLowlevelResourceHash());
        ASSERT_EQ(resourceA, resourceB);
    }

    TEST_F(AResourceTestClient, destroyingDuplicateResourceDoesNotDeleteData)
    {
        const vec2f data1[2] = { vec2f{1.f,2.f}, vec2f{3.f,4.f} };
        const vec2f data2[2] = { vec2f{1.f,2.f}, vec2f{3.f,4.f} };
        auto a = m_scene.createArrayResource(2, data1);
        auto b = m_scene.createArrayResource(2, data2);

        m_scene.destroy(*b);
        ramses::internal::ManagedResource aRes = client.impl().getResource(a->impl().getLowlevelResourceHash());
        EXPECT_TRUE(ramses::internal::UnsafeTestMemoryHelpers::CompareMemoryBlobToSpan(&data1, sizeof(data1), aRes->getResourceData().span()));
    }

    TEST_F(AResourceTestClient, createFloatArray)
    {
        const float data[2] = {};
        const auto a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);
        EXPECT_EQ(a->getNumberOfElements(), 2u);
        EXPECT_EQ(a->getDataType(), ramses::EDataType::Float);
    }

    TEST_F(AResourceTestClient, createFloatArrayHashIsValid)
    {
        const float data[2] = {};
        auto a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);

        const ramses::internal::ResourceContentHash hash = a->impl().getLowlevelResourceHash();
        EXPECT_TRUE(hash.isValid());
    }

    TEST_F(AResourceTestClient, createFloatArrayHashIsUnique)
    {
        float data[2] = {};
        data[0] = 4;
        auto a = m_scene.createArrayResource(2, data);

        const ramses::internal::ResourceContentHash hash = a->impl().getLowlevelResourceHash();

        float data2[2] = {};
        data2[0] = 42.0f;
        auto a2 = m_scene.createArrayResource(2, data2);

        const ramses::internal::ResourceContentHash hash2 = a2->impl().getLowlevelResourceHash();
        EXPECT_TRUE(hash != hash2);
    }

    TEST_F(AResourceTestClient, createEmptyFloatArrayFails)
    {
        auto a = m_scene.createArrayResource<float>(0, nullptr);
        EXPECT_TRUE(nullptr == a);
    }

    TEST_F(AResourceTestClient, createVector2fArray)
    {
        const vec2f data[2] = { vec2f{1.f,2.f}, vec2f{3.f,4.f} };
        const auto a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);
        EXPECT_EQ(a->getNumberOfElements(), 2u);
        EXPECT_EQ(a->getDataType(), ramses::EDataType::Vector2F);
    }

    TEST_F(AResourceTestClient, createVector2fArrayHashIsValid)
    {
        const vec2f data[2] = { vec2f{1.f,2.f}, vec2f{3.f,4.f} };
        const auto a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);

        const ramses::internal::ResourceContentHash hash = a->impl().getLowlevelResourceHash();
        EXPECT_TRUE(hash.isValid());
    }

    TEST_F(AResourceTestClient, createVector2fArrayHashIsUnique)
    {
        vec2f data[2] = { vec2f{1.f,2.f}, vec2f{3.f,4.f} };
        data[0][0] = 4;
        auto a = m_scene.createArrayResource(2, data);

        const ramses::internal::ResourceContentHash hash = a->impl().getLowlevelResourceHash();

        vec2f data2[2] = { vec2f{1.f,2.f}, vec2f{3.f,4.f} };
        data2[0][0] = 42.0f;
        auto a2 = m_scene.createArrayResource(2, data2);

        const ramses::internal::ResourceContentHash hash2 = a2->impl().getLowlevelResourceHash();
        EXPECT_TRUE(hash != hash2);
    }

    TEST_F(AResourceTestClient, createEmptyVector2fArrayFails)
    {
        auto a = m_scene.createArrayResource<vec2f>(0, nullptr);
        EXPECT_TRUE(nullptr == a);
    }

    TEST_F(AResourceTestClient, createVector3fArray)
    {
        const vec3f data[2] = { vec3f{1.f,2.f,3.f}, vec3f{3.f,4.f,5.f} };
        auto a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);
        EXPECT_EQ(a->getNumberOfElements(), 2u);
        EXPECT_EQ(a->getDataType(), ramses::EDataType::Vector3F);
    }

    TEST_F(AResourceTestClient, createVector3fArrayHashIsValid)
    {
        const vec3f data[2] = { vec3f{1.f,2.f,3.f}, vec3f{3.f,4.f,5.f} };
        auto a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);

        const ramses::internal::ResourceContentHash hash = a->impl().getLowlevelResourceHash();
        EXPECT_TRUE(hash.isValid());
    }

    TEST_F(AResourceTestClient, createVector3fArrayHashIsUnique)
    {
        vec3f data[2] = { vec3f{1.f,2.f,3.f}, vec3f{3.f,4.f,5.f} };
        data[0][0] = 4.0f;
        auto a = m_scene.createArrayResource(2, data);

        const ramses::internal::ResourceContentHash hash = a->impl().getLowlevelResourceHash();

        vec3f data2[2] = { vec3f{1.f,2.f,3.f}, vec3f{3.f,4.f,5.f} };
        data2[0][0] = 44.0f;
        auto a2 = m_scene.createArrayResource(2, data2);

        const ramses::internal::ResourceContentHash hash2 = a2->impl().getLowlevelResourceHash();
        EXPECT_TRUE(hash != hash2);
    }

    TEST_F(AResourceTestClient, createEmptyVector3fArrayFails)
    {
        auto a = m_scene.createArrayResource<vec3f>(0, nullptr);
        EXPECT_TRUE(nullptr == a);
    }

    TEST_F(AResourceTestClient, createVector4fArray)
    {
        const vec4f data[2] = { vec4f{1.f,2.f,3.f,4.f}, vec4f{3.f,4.f,5.f,6.f} };
        auto a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);
        EXPECT_EQ(a->getNumberOfElements(), 2u);
        EXPECT_EQ(a->getDataType(), ramses::EDataType::Vector4F);
    }

    TEST_F(AResourceTestClient, createVector4fArrayHashIsValid)
    {
        const vec4f data[2] = { vec4f{1.f,2.f,3.f,4.f}, vec4f{3.f,4.f,5.f,6.f} };
        auto a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);

        const ramses::internal::ResourceContentHash hash = a->impl().getLowlevelResourceHash();
        EXPECT_TRUE(hash.isValid());
    }

    TEST_F(AResourceTestClient, createVector4fArrayHashIsUnique)
    {
        vec4f data[2] = { vec4f{1.f,2.f,3.f,4.f}, vec4f{3.f,4.f,5.f,6.f} };
        data[0][0] = 4.0f;
        auto a = m_scene.createArrayResource(2, data);

        const ramses::internal::ResourceContentHash hash = a->impl().getLowlevelResourceHash();

        vec4f data2[2] = { vec4f{1.f,2.f,3.f,4.f}, vec4f{3.f,4.f,5.f,6.f} };
        data2[0][0] = 42.0f;
        auto a2 = m_scene.createArrayResource(2, data2);

        const ramses::internal::ResourceContentHash hash2 = a2->impl().getLowlevelResourceHash();
        EXPECT_TRUE(hash != hash2);
    }

    TEST_F(AResourceTestClient, createEmptyVector4fArrayFails)
    {
        auto a = m_scene.createArrayResource<vec4f>(0, nullptr);
        EXPECT_TRUE(nullptr == a);
    }

    TEST_F(AResourceTestClient, createUInt16ArrayResource)
    {
        const uint16_t data[2] = {};
        const ArrayResource *a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);
        EXPECT_EQ(a->getNumberOfElements(), 2u);
        EXPECT_EQ(a->getDataType(), ramses::EDataType::UInt16);
    }

    TEST_F(AResourceTestClient, createUInt16ArrayResourceHashIsValid)
    {
        const uint16_t data[2] = {};
        const ArrayResource* a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);

        const ramses::internal::ResourceContentHash hash = a->impl().getLowlevelResourceHash();
        EXPECT_TRUE(hash.isValid());
    }

    TEST_F(AResourceTestClient, createUInt16ArrayHashIsUnique)
    {
        uint16_t data[2] = {};
        data[0] = 4;
        auto a = m_scene.createArrayResource(2, data);

        const ramses::internal::ResourceContentHash hash = a->impl().getLowlevelResourceHash();

        uint16_t data2[2] = {};
        data2[0] = 42;
        auto a2 = m_scene.createArrayResource(2, data2);

        const ramses::internal::ResourceContentHash hash2 = a2->impl().getLowlevelResourceHash();
        EXPECT_TRUE(hash != hash2);
    }

    TEST_F(AResourceTestClient, createEmptyUInt16ArrayFails)
    {
        auto a = m_scene.createArrayResource<uint16_t>(2, nullptr);
        EXPECT_TRUE(nullptr == a);
    }

    TEST_F(AResourceTestClient, createUInt32Array)
    {
        const uint32_t data[2] = {};
        auto a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);
        EXPECT_EQ(a->getNumberOfElements(), 2u);
        EXPECT_EQ(a->getDataType(), ramses::EDataType::UInt32);
    }

    TEST_F(AResourceTestClient, createUInt32ArrayHashIsValid)
    {
        const uint32_t data[2] = {};
        auto a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);

        const ramses::internal::ResourceContentHash hash = a->impl().getLowlevelResourceHash();
        EXPECT_TRUE(hash.isValid());
    }

    TEST_F(AResourceTestClient, createUInt32ArrayHashIsUnique)
    {
        uint32_t data[2] = {};
        data[0] = 4;
        auto a = m_scene.createArrayResource(2, data);

        const ramses::internal::ResourceContentHash hash = a->impl().getLowlevelResourceHash();

        uint32_t data2[2] = {};
        data2[0] = 42;
        auto a2 = m_scene.createArrayResource(2, data2);

        const ramses::internal::ResourceContentHash hash2 = a2->impl().getLowlevelResourceHash();
        EXPECT_TRUE(hash != hash2);
    }

    TEST_F(AResourceTestClient, createEmptyUInt32ArrayFails)
    {
        auto a = m_scene.createArrayResource<uint32_t>(0, nullptr);
        EXPECT_TRUE(nullptr == a);
    }


    TEST_F(AResourceTestClient, createByteBlobArray)
    {
        const std::byte data[2] = {};
        const auto a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);
        EXPECT_EQ(a->getNumberOfElements(), 2u);
        EXPECT_EQ(a->getDataType(), ramses::EDataType::ByteBlob);
    }

    TEST_F(AResourceTestClient, createByteBlobArrayHashIsValid)
    {
        const std::byte data[2] = {};
        const auto a = m_scene.createArrayResource(2, data);
        ASSERT_TRUE(nullptr != a);

        const ramses::internal::ResourceContentHash hash = a->impl().getLowlevelResourceHash();
        EXPECT_TRUE(hash.isValid());
    }

    TEST_F(AResourceTestClient, createByteBlobArrayHashIsUnique)
    {
        std::byte data[2] = {};
        data[0] = std::byte{4};
        auto a = m_scene.createArrayResource(sizeof(data), data);

        const ramses::internal::ResourceContentHash hash = a->impl().getLowlevelResourceHash();

        std::byte data2[2] = {};
        data2[0] = std::byte{5};
        auto a2 = m_scene.createArrayResource(2, data2);

        const ramses::internal::ResourceContentHash hash2 = a2->impl().getLowlevelResourceHash();
        EXPECT_TRUE(hash != hash2);
    }

    TEST_F(AResourceTestClient, createEmptyByteBlobArrayFails)
    {
        auto a = m_scene.createArrayResource<std::byte>(0, nullptr);
        EXPECT_TRUE(nullptr == a);
    }

    TEST_F(AResourceTestClient, createAndDestroyEffect)
    {
        EffectDescription effectDesc;
        EXPECT_TRUE(effectDesc.setVertexShaderFromFile("res/ramses-client-test_shader.vert"));
        EXPECT_TRUE(effectDesc.setFragmentShaderFromFile("res/ramses-client-test_minimalShader.frag"));

        auto effect = m_scene.createEffect(effectDesc, "effect");
        ASSERT_TRUE(effect != nullptr);
    }

    TEST_F(AResourceTestClient, effectCreatedTwiceWithSameHashCanBeCreatedAndDestroyed)
    {
        EffectDescription effectDesc;
        EXPECT_TRUE(effectDesc.setVertexShaderFromFile("res/ramses-client-test_shader.vert"));
        EXPECT_TRUE(effectDesc.setFragmentShaderFromFile("res/ramses-client-test_minimalShader.frag"));

        auto effect1 = m_scene.createEffect(effectDesc, "effect1");
        ASSERT_TRUE(effect1 != nullptr);
        auto effect2 = m_scene.createEffect(effectDesc, "effect2");
        ASSERT_TRUE(effect2 != nullptr);

        EXPECT_TRUE(m_scene.destroy(*effect1));
        EXPECT_TRUE(m_scene.destroy(*effect2));
    }

    TEST_F(AResourceTestClient, effectCreatedTwiceWithDifferentNameProducesSameHash)
    {
        EffectDescription effectDesc;
        EXPECT_TRUE(effectDesc.setVertexShaderFromFile("res/ramses-client-test_shader.vert"));
        EXPECT_TRUE(effectDesc.setFragmentShaderFromFile("res/ramses-client-test_minimalShader.frag"));
        auto effect1 = m_scene.createEffect(effectDesc, "effect1");
        ASSERT_TRUE(effect1 != nullptr);
        auto effect2 = m_scene.createEffect(effectDesc, "effect2");
        ASSERT_TRUE(effect2 != nullptr);
        EXPECT_EQ(effect1->impl().getLowlevelResourceHash(), effect2->impl().getLowlevelResourceHash());
        m_scene.destroy(*effect1);
        m_scene.destroy(*effect2);
    }

    template <typename ResourceType>
    class ResourceTest : public LocalTestClientWithScene, public testing::Test
    {
    public:
        ResourceType& createResource(std::string_view name)
        {
            return (this->template createObject<ResourceType>(name));
        }
    };

    TYPED_TEST_SUITE(ResourceTest, ResourceTypes);

    TYPED_TEST(ResourceTest, canBeConvertedToResource)
    {
        RamsesObject& obj = this->createResource("resource");
        EXPECT_TRUE(obj.as<Resource>() != nullptr);
        const RamsesObject& constObj = obj;
        EXPECT_TRUE(constObj.as<Resource>() != nullptr);
    }

    TYPED_TEST(ResourceTest, sameResourcesWithDifferentNamesShareSameHash)
    {
        const std::string name_A("name_A");
        const std::string name_B("name_B");
        auto* resource_A = static_cast<Resource*>(&this->createResource(name_A));
        auto* resource_B = static_cast<Resource*>(&this->createResource(name_B));

        RamsesObject* foundObject_A = this->getScene().findObject(name_A);
        RamsesObject* foundObject_B = this->getScene().findObject(name_B);

        ASSERT_TRUE(resource_A);
        ASSERT_TRUE(resource_B);
        EXPECT_EQ(resource_A->impl().getLowlevelResourceHash(), resource_B->impl().getLowlevelResourceHash());

        ASSERT_TRUE(foundObject_A);
        ASSERT_TRUE(foundObject_B);
        EXPECT_EQ(foundObject_A, static_cast<RamsesObject*>(resource_A));
        EXPECT_EQ(foundObject_B, static_cast<RamsesObject*>(resource_B));
        EXPECT_NE(resource_A, resource_B);
    }

    TYPED_TEST(ResourceTest, statisticCounterIsUpdated)
    {
        EXPECT_EQ(0u, this->getFramework().impl().getStatisticCollection().statResourcesCreated.getCounterValue());
        EXPECT_EQ(0u, this->getFramework().impl().getStatisticCollection().statResourcesDestroyed.getCounterValue());
        EXPECT_EQ(0u, this->getFramework().impl().getStatisticCollection().statResourcesNumber.getCounterValue());

        RamsesObject& obj = this->createResource("resource");
        auto* res = obj.as<Resource>();
        EXPECT_EQ(1u, this->getFramework().impl().getStatisticCollection().statResourcesCreated.getCounterValue());

        this->getFramework().impl().getStatisticCollection().nextTimeInterval(); //statResourcesNumber is updated by nextTimeInterval()
        EXPECT_EQ(1u, this->getFramework().impl().getStatisticCollection().statResourcesNumber.getCounterValue());

        this->getScene().destroy(*res);
        EXPECT_EQ(1u, this->getFramework().impl().getStatisticCollection().statResourcesDestroyed.getCounterValue());

        this->getFramework().impl().getStatisticCollection().nextTimeInterval();
        EXPECT_EQ(0u, this->getFramework().impl().getStatisticCollection().statResourcesNumber.getCounterValue());
    }

    TYPED_TEST(ResourceTest, canCreateResourceSetNameAndAfterwardsDestroyResourceCleanly)
    {
        TypeParam& obj = this->createResource("resource");
        obj.setName("otherName");
        EXPECT_TRUE(this->m_scene.destroy(obj));
        SceneObjectIterator iterator(this->m_scene, ERamsesObjectType::Resource);
        EXPECT_EQ(iterator.getNext(), nullptr);
        EXPECT_TRUE(this->m_scene.saveToFile("someFileName.ramses", {}));
    }

    TYPED_TEST(ResourceTest, canCreateSameResourceTwiceAndDeleteBothAgain)
    {
        TypeParam& obj1 = this->createResource("resource");
        TypeParam& obj2 = this->createResource("resource");
        EXPECT_TRUE(this->m_scene.destroy(obj1));
        EXPECT_TRUE(this->m_scene.destroy(obj2));
    }

    TYPED_TEST(ResourceTest, canFindResourceByItsNewIdAfterRenaming)
    {
        TypeParam& obj = this->createResource("resource");
        const auto id = obj.getResourceId();
        EXPECT_EQ(this->m_scene.getResource(id), &obj);
        obj.setName("otherName");
        EXPECT_EQ(this->m_scene.getResource(id), nullptr);
        EXPECT_EQ(this->m_scene.findObject("otherName"), &obj);
        EXPECT_EQ(this->m_scene.getResource(obj.getResourceId()), &obj);
    }

    TYPED_TEST(ResourceTest, canGetResourceByIdAsLongAsAtLeastOneExists)
    {
        const auto id = this->createResource("resource").getResourceId();
        this->createResource("resource");
        const auto obj1 = this->m_scene.getResource(id);
        EXPECT_TRUE(this->m_scene.destroy(*obj1));
        const auto obj2 = this->m_scene.getResource(id);
        EXPECT_NE(obj1, obj2);
        EXPECT_TRUE(this->m_scene.destroy(*obj2));
        EXPECT_EQ(this->m_scene.getResource(id), nullptr);
    }
}
