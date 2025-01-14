//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/text/IFontAccessor.h"
#include "ramses/client/text/IFontInstance.h"

#include <string_view>

namespace ramses
{
    namespace internal
    {
        class FontRegistryImpl;
    }

    /**
    * @ingroup TextAPI
    * @brief Font registry can be used to load Freetype2 fonts and create font instances (optionally with Harfbuzz).
    *        These are owned and managed by FontRegistry.
    */
    class RAMSES_API FontRegistry : public IFontAccessor
    {
    public:
        /**
        * @brief Constructor for FontRegistry.
        */
        FontRegistry();

        /**
        * @brief Destructor for FontRegistry. Destroys all created fonts
        */
        ~FontRegistry() override;

        /**
        * @brief Get font instance object corresponding to given id
        *
        * @param[in] fontInstanceId The id of font instance
        * @return The font instance object, nullptr if not found
        */
        [[nodiscard]] IFontInstance*  getFontInstance(FontInstanceId fontInstanceId) const override;

        /**
        * @brief Load Freetype2 font from file
        *
        * @param[in] fontPath The file path to the font
        * @return The font id, FontId::Invalid() on error
        */
        FontId createFreetype2Font(std::string_view fontPath);

        /**
        * @brief Load Freetype2 font from an open file descriptor.
        *
        * The font file must be in the already opened filedescriptor at absolute position offset within
        * the file. The filedescriptor must be opened for read access and may not be modified anymore after
        * this call. The filedescriptor must support seeking.
        * Ramses takes ownership of the filedescriptor and will close it when not needed anymore.
        *
        * The behavior is undefined if the filedescriptor does not contain a complete and valid font
        * at offset.
        *
        * @param[in] fd Open and readable filedescriptor.
        * @param[in] offset Absolute starting position of the font data within fd.
        * @param[in] length Size of the font data within fd.
        * @return The font id, FontId::Invalid() on error
        */
        FontId createFreetype2FontFromFileDescriptor(int fd, size_t offset, size_t length);

        /**
        * @brief Create Freetype2 font instance
        *
        * @param[in] fontId The id of the font from which to create a font instance
        * @param[in] size Size (height in rasterized texels of glyphs) of the font
        * @param[in] forceAutohinting Force autohinting (a flag for FT2 library)
        * @return The font instance id, FontInstanceId::Invalid() on error
        */
        FontInstanceId          createFreetype2FontInstance(FontId fontId, uint32_t size, bool forceAutohinting = false);

        /**
        * @brief Create Freetype2 font instance with Harfbuzz shaping
        *
        * @param[in] fontId The id of the font from which to create a font instance
        * @param[in] size Size (in rasterized texels of glyphs) of the font
        * @param[in] forceAutohinting Force autohinting (a flag for FT2 library)
        * @return The font instance id, FontInstanceId::Invalid() on error
        */
        FontInstanceId          createFreetype2FontInstanceWithHarfBuzz(FontId fontId, uint32_t size, bool forceAutohinting = false);

        /**
        * @brief Delete an existing font
        *
        * @param[in] fontId The id of the font to be deleted
        * @return True on success, false otherwise
        */
        bool                    deleteFont(FontId fontId);

        /**
        * @brief Delete an existing font instance
        *
        * @param[in] fontInstance The id of the font instance to be deleted
        * @return True on success, false otherwise
        */
        bool                    deleteFontInstance(FontInstanceId fontInstance);

        /**
        * Stores internal data for implementation specifics of FontRegistry.
        */
        class internal::FontRegistryImpl& impl;

        /**
         * @brief Deleted copy constructor
         * @param other unused
         */
        FontRegistry(const FontRegistry& other) = delete;

        /**
         * @brief Deleted copy assignment
         * @param other unused
         * @return unused
         */
        FontRegistry& operator=(const FontRegistry& other) = delete;
    };
}
