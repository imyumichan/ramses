//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesObject.h"

namespace ramses
{
    namespace internal
    {
        class SceneIteratorImpl;
    }

    class RamsesClient;
    class Scene;

    /**
    * @ingroup CoreAPI
    * @brief The SceneIterator traverses scenes in a RamsesClient.
    *
    * It provides a way to traverse all scenes created with a given client.
    */
    class RAMSES_API SceneIterator
    {
    public:
        /**
        * @brief A SceneIterator can iterate through scenes of the given client.
        *
        * @param[in] client RamsesClient whose scenes to iterate through
        **/
        explicit SceneIterator(const RamsesClient& client);

        /**
        * Destructor
        **/
        ~SceneIterator();

        /**
        * @brief Returns the next scene while iterating.
        *
        * @return The next scene, null when no more scenes are available.
        * The iterator is invalid and may not be used after any scenes are added or removed.
        **/
        Scene* getNext();

    private:
        std::unique_ptr<internal::SceneIteratorImpl> m_impl;
    };
}
