//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/Resource.h"
#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ramses/framework/TextureEnums.h"
#include "MipLevelData.h"
#include "TextureSwizzle.h"

namespace ramses
{
    namespace internal
    {
        class Texture2DImpl;
    }

    /**
    * @ingroup CoreAPI
    * @brief Texture represents a 2-D texture resource.
    */
    class RAMSES_API Texture2D : public Resource
    {
    public:
        /**
        * @brief Gets texture width
        *
        * @return Texture width
        */
        [[nodiscard]] uint32_t getWidth() const;

        /**
        * @brief Gets texture height
        *
        * @return Texture height
        */
        [[nodiscard]] uint32_t getHeight() const;

        /**
        * @brief Gets texture format
        *
        * @return Texture format
        */
        [[nodiscard]] ETextureFormat getTextureFormat() const;

        /**
        * @brief Gets swizzle description
        *
        * @return Swizzle Description
        */
        [[nodiscard]] const TextureSwizzle& getTextureSwizzle() const;

        /**
         * Get the internal data for implementation specifics of Texture.
         */
        [[nodiscard]] internal::Texture2DImpl& impl();

        /**
         * Get the internal data for implementation specifics of Texture.
         */
        [[nodiscard]] const internal::Texture2DImpl& impl() const;

    protected:
        /**
        * @brief Scene is the factory for creating Texture instances.
        */
        friend class internal::SceneObjectRegistry;

        /**
        * @brief Constructor of Texture
        *
        * @param[in] impl Internal data for implementation specifics of Texture (sink - instance becomes owner)
        */
        explicit Texture2D(std::unique_ptr<internal::Texture2DImpl> impl);

        /**
        * Stores internal data for implementation specifics of Texture.
        */
        internal::Texture2DImpl& m_impl;
    };
}
