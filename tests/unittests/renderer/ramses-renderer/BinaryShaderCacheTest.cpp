//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/renderer/BinaryShaderCache.h"
#include "impl/BinaryShaderCacheImpl.h"
#include "gtest/gtest.h"
#include "internal/RendererLib/Types.h"
#include "internal/Core/Utils/File.h"
#include <sys/stat.h>

#include <array>
#include <string>

template <typename... Ts>
std::array<std::byte, sizeof...(Ts)> make_byte_array(Ts&&... args) noexcept
{
    return {std::byte(std::forward<Ts>(args))...};
}

namespace ramses::internal
{
    class ABinaryShaderCache : public testing::Test
    {
    public:
        ABinaryShaderCache()
            : m_binaryShaderFilePath("test.binaryshader")
        {

        }

        ~ABinaryShaderCache() override
        {
            File binaryShaderFile(m_binaryShaderFilePath);
            if (binaryShaderFile.exists())
            {
                binaryShaderFile.remove();
            }
        }

        void createTestFile()
        {
            ramses::BinaryShaderCache cache;

            const auto shaderData1 = make_byte_array(12u, 34u, 56u, 78u);
            const ramses::binaryShaderFormatId_t format1{ 123u };
            const ramses::effectId_t effectHash1 = {11u, 0};
            cache.storeBinaryShader(effectHash1, ramses::sceneId_t(1u), shaderData1.data(), static_cast<uint32_t>(shaderData1.size()), format1);

            const auto shaderData2 = make_byte_array(13u, 14u, 66u, 7u, 89u, 10u);
            const ramses::binaryShaderFormatId_t format2{ 112u };
            const ramses::effectId_t effectHash2 = {12u, 0};
            cache.storeBinaryShader(effectHash2, ramses::sceneId_t(2u), shaderData2.data(), static_cast<uint32_t>(shaderData2.size()), format2);

            cache.saveToFile(m_binaryShaderFilePath.c_str());
        }

        void enlargeTestFile()
        {
            ramses::internal::File file(m_binaryShaderFilePath.c_str());
            size_t fileSize(0);
            EXPECT_TRUE(file.getSizeInBytes(fileSize));
            EXPECT_TRUE(file.open(File::Mode::WriteExistingBinary));
            EXPECT_TRUE(file.seek(fileSize, File::SeekOrigin::BeginningOfFile));
            char data(33);
            EXPECT_TRUE(file.write(&data, sizeof(data)));
        }

        void truncateTestFile(int32_t size)
        {
            ramses::internal::File file(m_binaryShaderFilePath.c_str());
            EXPECT_TRUE(file.open(File::Mode::ReadOnlyBinary));
            size_t fileSize(0);
            EXPECT_TRUE(file.getSizeInBytes(fileSize));
            std::vector<uint8_t> data(fileSize);

            size_t numReadBytes = 0u;
            EXPECT_EQ(EStatus::Ok, file.read(data.data(), fileSize, numReadBytes));

            file.close();
            EXPECT_TRUE(file.open(File::Mode::WriteNewBinary));

            int32_t newSize = (size < 0) ?  static_cast<int32_t>(fileSize) + size : size;

            EXPECT_TRUE(file.write(data.data(), newSize));
        }

        void corruptTestFile()
        {
            ramses::internal::File file(m_binaryShaderFilePath);
            size_t fileSize(0);
            EXPECT_TRUE(file.getSizeInBytes(fileSize));
            EXPECT_TRUE(file.open(File::Mode::WriteExistingBinary));
            const auto offset = sizeof(BinaryShaderCacheImpl::FileHeader);
            EXPECT_TRUE(file.seek(offset, File::SeekOrigin::BeginningOfFile));
            char data = 0;
            size_t numBytesRead = 0u;
            EXPECT_EQ(EStatus::Ok, file.read(&data, sizeof(data), numBytesRead));
            EXPECT_TRUE(file.seek(offset, File::SeekOrigin::BeginningOfFile));
            data++;
            EXPECT_TRUE(file.write(&data, sizeof(data)));
        }

        void corruptVersionInTestFile()
        {
            ramses::internal::File file(m_binaryShaderFilePath);
            size_t fileSize(0);
            EXPECT_TRUE(file.getSizeInBytes(fileSize));
            EXPECT_TRUE(file.open(File::Mode::WriteExistingBinary));
            const auto transportVersionOffset = offsetof(BinaryShaderCacheImpl::FileHeader, transportVersion);
            EXPECT_TRUE(file.seek(transportVersionOffset, File::SeekOrigin::BeginningOfFile));
            char data = 0;
            size_t numBytesRead = 0u;
            EXPECT_EQ(EStatus::Ok, file.read(&data, sizeof(data), numBytesRead));
            EXPECT_TRUE(file.seek(transportVersionOffset, File::SeekOrigin::BeginningOfFile));
            data++;
            EXPECT_TRUE(file.write(&data, sizeof(data)));
        }

    protected:
        ramses::BinaryShaderCache m_cache;
        const std::string m_binaryShaderFilePath;
    };

    TEST_F(ABinaryShaderCache, canStoreAndGetBinaryShader)
    {
        const auto shaderData = make_byte_array(12u, 34u, 56u, 78u);
        const auto shaderDataSize = static_cast<uint32_t>(shaderData.size());
        const ramses::binaryShaderFormatId_t format{ 123u };
        const ramses::effectId_t effectHash1 = { 11u, 0 };
        m_cache.storeBinaryShader(effectHash1, ramses::sceneId_t(1u), shaderData.data(), shaderDataSize, format);

        m_cache.deviceSupportsBinaryShaderFormats(&format, 1u);
        EXPECT_TRUE(m_cache.hasBinaryShader(effectHash1));
        EXPECT_EQ(shaderDataSize, m_cache.getBinaryShaderSize(effectHash1));
        EXPECT_EQ(format, m_cache.getBinaryShaderFormat(effectHash1));

        std::vector<std::byte> dataRead(shaderDataSize);
        m_cache.getBinaryShaderData(effectHash1, &dataRead.front(), shaderDataSize);

        // make sure the data value is the same.
        for (uint32_t index = 0; index < shaderDataSize; index++)
        {
            EXPECT_TRUE(shaderData[index] == dataRead[index]);
        }
    }

    TEST_F(ABinaryShaderCache, reportsNoShaderAvailableIfShaderCachedButWithWrongFormat)
    {
        const auto shaderData = make_byte_array(12u, 34u, 56u, 78u);
        const ramses::binaryShaderFormatId_t format{ 123u };
        const ramses::effectId_t effectHash1 = { 11u, 0 };
        m_cache.storeBinaryShader(effectHash1, ramses::sceneId_t(1u), shaderData.data(), uint32_t(shaderData.size()), format);

        const ramses::binaryShaderFormatId_t supportedFormat{ 124u };
        m_cache.deviceSupportsBinaryShaderFormats(&supportedFormat, 1u);
        EXPECT_FALSE(m_cache.hasBinaryShader(effectHash1));
    }

    TEST_F(ABinaryShaderCache, reportsNoShaderAvailableIfShaderNotCachedAndDeviceReportsFormatZeroSupported) // zero format is used as invalid format in cache impl
    {
        const ramses::binaryShaderFormatId_t supportedFormat{ 0u };
        m_cache.deviceSupportsBinaryShaderFormats(&supportedFormat, 1u);
        const ramses::effectId_t effectHash1 = { 11u, 0 };
        EXPECT_FALSE(m_cache.hasBinaryShader(effectHash1));
    }

    TEST_F(ABinaryShaderCache, canReturnFalseWhenBinaryShaderNotInCache)
    {
        const auto shaderData = make_byte_array(12u, 34u, 56u, 78u);
        const auto shaderDataSize = static_cast<uint32_t>(shaderData.size());
        const ramses::binaryShaderFormatId_t format{ 123u };
        const ramses::effectId_t effectHash1 = { 11u, 0 };
        m_cache.storeBinaryShader(effectHash1, ramses::sceneId_t(1u), shaderData.data(), shaderDataSize, format);

        ramses::effectId_t nonExistEffectHash = { 13u, 0 };
        EXPECT_FALSE(m_cache.hasBinaryShader(nonExistEffectHash));
        EXPECT_EQ(ramses::binaryShaderFormatId_t{ 0u }, m_cache.getBinaryShaderFormat(nonExistEffectHash));
        EXPECT_EQ(0u, m_cache.getBinaryShaderSize(nonExistEffectHash));
    }

    TEST_F(ABinaryShaderCache, canSaveToAndLoadFromBinaryShaderFile)
    {
        const auto shaderData1 = make_byte_array(12u, 34u, 56u, 78u);
        const auto shaderDataSize1 = static_cast<uint32_t>(shaderData1.size());
        const ramses::binaryShaderFormatId_t format1{ 123u };
        const ramses::effectId_t effectHash1 = { 11u, 0 };
        m_cache.storeBinaryShader(effectHash1, ramses::sceneId_t(1u), shaderData1.data(), shaderDataSize1, format1);

        const auto shaderData2 = make_byte_array(13u, 14u, 66u, 7u, 89u, 10u);
        const auto shaderDataSize2 = static_cast<uint32_t>(shaderData2.size());
        const ramses::binaryShaderFormatId_t format2{ 112u };
        const ramses::effectId_t effectHash2 = { 12u, 0 };
        m_cache.storeBinaryShader(effectHash2, ramses::sceneId_t(2u), shaderData2.data(), shaderDataSize2, format2);

        m_cache.saveToFile(m_binaryShaderFilePath.c_str());

        ramses::BinaryShaderCache newCache;
        EXPECT_TRUE(newCache.loadFromFile(m_binaryShaderFilePath.c_str()));

        // need to init with supported formats in order to report cached shaders
        std::array<ramses::binaryShaderFormatId_t, 2> supportedFormats = { format1, format2 };
        newCache.deviceSupportsBinaryShaderFormats(supportedFormats.data(), uint32_t(supportedFormats.size()));

        // check shader data 1
        EXPECT_TRUE(newCache.hasBinaryShader(effectHash1));
        EXPECT_EQ(format1, newCache.getBinaryShaderFormat(effectHash1));

        const uint32_t shaderDataSize1Read = newCache.getBinaryShaderSize(effectHash1);
        EXPECT_EQ(shaderDataSize1, shaderDataSize1Read);

        std::vector<std::byte> shaderData1Read(shaderDataSize1Read);
        newCache.getBinaryShaderData(effectHash1, &shaderData1Read.front(), shaderDataSize1Read);
        for (uint32_t index = 0; index < shaderDataSize1Read; index++)
        {
            EXPECT_EQ(shaderData1[index], shaderData1Read[index]);
        }

        // check shader data 2
        EXPECT_TRUE(newCache.hasBinaryShader(effectHash2));
        EXPECT_EQ(format2, newCache.getBinaryShaderFormat(effectHash2));

        const uint32_t shaderDataSize2Read = newCache.getBinaryShaderSize(effectHash2);
        EXPECT_EQ(shaderDataSize2, shaderDataSize2Read);

        std::vector<std::byte> shaderData2Read(shaderDataSize2Read);
        newCache.getBinaryShaderData(effectHash2, &shaderData2Read.front(), shaderDataSize2Read);
        for (uint32_t index = 0; index < shaderDataSize2Read; index++)
        {
            EXPECT_EQ(shaderData2[index], shaderData2Read[index]);
        }
    }

    TEST_F(ABinaryShaderCache, canReturnFalseWhenTryToLoadFromNonExistFile)
    {
        EXPECT_FALSE(m_cache.loadFromFile("NonExistFile.binaryshader"));
    }

    TEST_F(ABinaryShaderCache, reportsSuccessOnCorrectFile)
    {
        createTestFile();
        EXPECT_TRUE(m_cache.loadFromFile(m_binaryShaderFilePath.c_str()));
    }

    TEST_F(ABinaryShaderCache, reportsFailOnFileToLarge)
    {
        createTestFile();
        enlargeTestFile();
        EXPECT_FALSE(m_cache.loadFromFile(m_binaryShaderFilePath.c_str()));
    }

    TEST_F(ABinaryShaderCache, reportsFailOnFileToShort)
    {
        createTestFile();
        truncateTestFile(-1);
        EXPECT_FALSE(m_cache.loadFromFile(m_binaryShaderFilePath.c_str()));
    }

    TEST_F(ABinaryShaderCache, reportsFailOnFileShorterThanHeader)
    {
        createTestFile();
        truncateTestFile(sizeof(BinaryShaderCacheImpl::FileHeader) - 1);
        EXPECT_FALSE(m_cache.loadFromFile(m_binaryShaderFilePath.c_str()));
    }

    TEST_F(ABinaryShaderCache, reportsFailOnNotMatchingChecksum)
    {
        createTestFile();
        corruptTestFile();
        EXPECT_FALSE(m_cache.loadFromFile(m_binaryShaderFilePath.c_str()));
    }

    TEST_F(ABinaryShaderCache, reportsFailOnInvalidFileVersion)
    {
        createTestFile();
        corruptVersionInTestFile();
        EXPECT_FALSE(m_cache.loadFromFile(m_binaryShaderFilePath.c_str()));
    }

    TEST_F(ABinaryShaderCache, handlesDoubleStoreProperly)
    {
        const auto shaderData = make_byte_array(12u, 34u, 56u, 78u);
        const auto shaderDataSize = static_cast<uint32_t>(shaderData.size());
        const ramses::binaryShaderFormatId_t format{ 123u };
        const ramses::effectId_t effectHash1 = { 11u, 0 };
        m_cache.storeBinaryShader(effectHash1, ramses::sceneId_t(1u), shaderData.data(), shaderDataSize, format);
        m_cache.deviceSupportsBinaryShaderFormats(&format, 1u);
        EXPECT_TRUE(m_cache.hasBinaryShader(effectHash1));
        m_cache.storeBinaryShader(effectHash1, ramses::sceneId_t(1u), shaderData.data(), shaderDataSize, format);
        EXPECT_TRUE(m_cache.hasBinaryShader(effectHash1));
    }
}
