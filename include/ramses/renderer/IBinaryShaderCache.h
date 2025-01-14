//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/renderer/Types.h"
#include "ramses/framework/APIExport.h"

namespace ramses
{
    /**
    * @ingroup RendererAPI
    * @brief An interface used to exchange Binary Shader Cache with the RamsesRenderer. The RamsesRenderer will not
    *        delete this class, it is owned by the application.
    *
    *   Implementation of this interface can be be passed to RendererConfig, and it will be used by RamsesRenderer internally
    *   to obtain/provide binary shaders. The interface handles both directions: from the application to RamsesRenderer (i.e. providing
    *   pre-cached effects to RAMSES) as well as from RamsesRenderer to the application (using the on-line shader compiler, if
    *   supported by the driver, to inform the application about existing binary shader data and providing the data itself). The
    *   application can decide whether to save, and if yes, in which form to save the provided binary shader data.
    *
    *   RAMSES itself does _not_ cache shaders internally - it only uses this interface to communicate with the application,
    *   which has the complete power how to manage binary shader data (file, memory, web-server)...
    *
    *   The interface works with the effectId_t to uniquely reference effects. Two different effects are guaranteed to have
    *   different such ids.
    *
    *   Example:
    *   Let S be the id of an effect which is cached in Database, and X be the id of an effect which is not cached. Database can
    *   be an indexed file, or just a hashmap in memory holding binary shaders and their mapping to effectId_t.
    *
    *    Database        Application                Renderer                  OpenGL driver
    *      |                  |                            |                              |
    *      |                  |  hasBinaryShader(X)        |                              |
    *      | hasData(X)       |<---------------------------|                              |
    *      |<-----------------|                            |                              |
    *      |----false-------->|                            |                              |
    *      |                  |-----false----------------->| Data = glGetProgramBinary(X) |
    *      |                  |                            |----------------------------->|
    *      |                  | storeBinaryShader(X, Data) |                              |
    *      |                  |<---------------------------|                              |
    *      |                  |                            |                              |
    *      |                  |                            |                              |
    *      |                  |  hasBinaryShader(C)        |                              |
    *      |  hasData(C)      |<---------------------------|                              |
    *      |<-----------------|                            |                              |
    *      |----true--------->|                            |                              |
    *      |                  |-----true------------------>|                              |
    *      |                  |                            |                              |
    *      |                  | getBinaryShaderFormat(C)   |                              |
    *      |  getFormat(C)    |<---------------------------|                              |
    *      |<-----------------|                            |                              |
    *      |----F------------>|                            |                              |
    *      |                  |-------F------------------->|                              |
    *      |                  |                            |                              |
    *      |                  | getBinaryShaderData(C)     |                              |
    *      |  getData(C)      |<---------------------------|                              |
    *      |<-----------------|                            |                              |
    *      |----data--------->|                            |                              |
    *      |                  |-------data---------------->|                              |
    *      |                  |                            |                              |
    *      |                  |                            |  glProgramBinary(C, F, data) |
    *      |                  |                            |----------------------------->|
    *
    */
    class RAMSES_API IBinaryShaderCache
    {
    public:
        /**
        * @brief Destructor of IBinaryShaderCache
        */
        virtual ~IBinaryShaderCache() = default;

        /**
        * @brief Provides a list of binary shader formats supported by the device in use.
        *        The cache implementation should provide only shaders that were compiled and stored with using of those formats.
        *        The format ID is platform/driver specific, on OpenGL it is equivalent to those retrieved from GL_PROGRAM_BINARY_FORMATS parameter.
        *        This callback will be called only once, after at least one display is created and at latest before the first IBinaryShaderCache::hasBinaryShader query.
        *
        * @param[in] supportedFormats Pointer to first element of a list of supported formats, this array is valid only for in the scope of this callback
        * @param[in] numSupportedFormats Number of elements in \c supportedFormats array, if zero there is no support for uploading binary shaders
        */
        virtual void deviceSupportsBinaryShaderFormats(const binaryShaderFormatId_t* supportedFormats, uint32_t numSupportedFormats) = 0;

        /**
         * @brief Used by RamsesRenderer to ask the application if it provides binary data for the Effect with the given effectId.
         *
         * @param effectId Effect Id of the Effect
         * @return true if there is binary shader for the Effect Id
         *         false otherwise
         */
        [[nodiscard]] virtual bool hasBinaryShader(effectId_t effectId) const = 0;

        /**
         * @brief Used by RamsesRenderer to ask the application for the size in bytes of the binary shader for the given effectId
         *
         *  This method will only be used by RamsesRenderer if the call to hasBinaryShader() returned true
         *
         * @param effectId Effect Id of the Effect
         * @return the application must return the exact size of the binary shader data for effectId in bytes. This is the amount of
         *          memory RAMSES will allocate for the buffer parameter in the getBinaryShaderData() method below
         */
        [[nodiscard]] virtual uint32_t getBinaryShaderSize(effectId_t effectId) const = 0;

        /**
         * @brief Used by RamsesRenderer to ask the application for the shader format of the binary shader for the given effectId
         *
         *  This method will only be used by RamsesRenderer if the call to hasBinaryShader() returned true
         *
         * @param effectId Effect Id of the Effect
         * @return The application must provide the exact shader format specified provided by either the offline shader compiler
         *          or the storeBinaryShader() call below, depending on whether the shader was generated by an offline compiler
         *          or loaded dynamically at runtime from the driver
         *
         * WARNING! OpenGL can behave very fragile if the shader format is wrong or forged! Never try to mix or interchange
         * binary shaders from an offline shader compiler with storage format from an online shader compiler or vice-versa,
         * as they may differ and using them wrongly can cause surprising driver crashes at unsuspected places!
         */
        [[nodiscard]] virtual binaryShaderFormatId_t getBinaryShaderFormat(effectId_t effectId) const = 0;

        /**
         * @brief      Used by RamsesRenderer to ask the application whether a specific effect should be cached.
         *
         *             This method is called if the shader is not cached yet but could be provided by the renderer.
         *             The cache can decide whether it the shader should be cached.
         *             ID of scene that requires the given effect is provided to allow caching for certain scenes only.
         *             Note that the scene ID is simply the first scene that needs this effect for rendering, there can
         *             however be more scenes using the same effect.
         *
         * @param[in]  effectId  Effect Id of the Effect
         * @param[in]  sceneId  ID of scene that uses the effect.
         * @return     true if the effect should be cached, false otherwise.
         */
        [[nodiscard]] virtual bool shouldBinaryShaderBeCached(effectId_t effectId, sceneId_t sceneId) const = 0;

        /**
         * @brief Used by RamsesRenderer to ask the application to provide the binary shader data for the given effectId
         *
         *  This method will only be used by RamsesRenderer if the call to hasBinaryShader() returned true
         *
         *  RamsesRenderer expects the application to write into 'buffer' which is guaranteed to have at least
         *  'bufferSize' allocated bytes
         *
         * @param effectId Effect Id of the Effect
         * @param buffer pointer to the buffer to be filled the binary shader data
         * @param bufferSize the size of 'buffer'
         */
        virtual void getBinaryShaderData(effectId_t effectId, std::byte* buffer, uint32_t bufferSize) const = 0;

        /**
         * @brief Used by RamsesRenderer to provide the application with a binary shader data an effect with the given effectId
         *
         *  If the driver supports reading binary shader cache for compiled shaders, then RamsesRenderer will
         *  call storeBinaryShader() whenever a new shader was compiled, to provide the application with the corresponding data
         *  Ramses will _not_ cache this data itself - so if the effect is deleted and used again later, RamsesRenderer
         *  will compile it again, but before it does it will ask the application if a cache is available by calling
         *  hasBinaryShader(). So use storeBinaryShader() to cache shaders if they are not supposed to be recompiled.
         *
         *  ID of scene that requires the given effect is provided to allow caching for certain scenes only.
         *  Note that the scene ID is simply the first scene that needs this effect for rendering, there can
         *  however be more scenes using the same effect.
         *
         * @param[in] effectId Effect Id of the Effect
         * @param[in] sceneId ID of scene that uses the effect.
         * @param[in] binaryShaderData pointer to the binary shader data
         * @param[in] binaryShaderDataSize size of the binary shader data
         * @param[in] binaryShaderFormat format of the binary shader - platform/driver specific binary shader format ID
         */
        virtual void storeBinaryShader(effectId_t effectId, sceneId_t sceneId, const std::byte* binaryShaderData, uint32_t binaryShaderDataSize, binaryShaderFormatId_t binaryShaderFormat) = 0;

        /**
        * @brief Used by RamsesRenderer to provide a callback with information on the result of a binary shader upload operation.
        *
        *  This method is called by the renderer after each attempted upload operation from a binary shader cache.
        *  It is intended for informational/debugging purposes in detecting an invalid binary shader cache.
        *
        * @param effectId Effect Id of the Effect
        * @param success The result
        */
        virtual void binaryShaderUploaded(effectId_t effectId, bool success) const = 0;
    };
}
