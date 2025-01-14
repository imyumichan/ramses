//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/logic/LogicNode.h"
#include <memory>

namespace ramses::internal
{
    class TimerNodeImpl;
}

namespace ramses
{
    /**
    * Timer node can be used to provide timing information to animation nodes (#ramses::AnimationNode) or any other logic nodes.
    * - Property inputs:
    *     - ticker_us (int64)  - (optional) user provided ticker in microseconds (by default, see below to learn how to use arbitrary time units).
    *                          - if the input is 0 (default) then this TimerNode uses system clock to generate ticker by itself,
    *                            this is recommended for simple use cases when application does not need more advanced timing control.
    * - Property outputs:
    *     - ticker_us (int64)  - in auto-generate mode ('ticker_us' input stays 0) this will output system clock time since epoch in microseconds
    *                          - in case of user provided ticker (non-zero 'ticker_us' input) this output will contain the same value
    *                            (user ticker is just passed through, this way time units are user defined).
    *
    * Timer node works in one of two modes - it generates ticker by itself or uses user provided ticker. This allows quick and easy switch
    * between stages of the development, e.g. prototyping, testing or production, where for some use cases auto-generated time is desired
    * and other require well specified timing provided by application.
    * Due to the auto-generate mode both the input and output has defined time units, however timer node can also be used in a fully
    * time unit agnostic mode, see inputs/outputs description above for details.
    * Note that unlike other logic nodes a TimerNode is always updated on every #ramses::LogicEngine::update call regardless of if any of its
    * inputs were modified or not.
    */
    class RAMSES_API TimerNode : public LogicNode
    {
    public:
        /**
         * Get the internal data for implementation specifics of TimerNode.
         */
        [[nodiscard]] internal::TimerNodeImpl& impl();

        /**
         * Get the internal data for implementation specifics of TimerNode.
         */
        [[nodiscard]] const internal::TimerNodeImpl& impl() const;

    protected:
        /**
        * Constructor of TimerNode. User is not supposed to call this - TimerNodes are created by other factory classes
        *
        * @param impl implementation details of the TimerNode
        */
        explicit TimerNode(std::unique_ptr<internal::TimerNodeImpl> impl) noexcept;

        /**
        * Implementation of TimerNode
        */
        internal::TimerNodeImpl& m_timerNodeImpl;

        friend class internal::ApiObjects;
    };
}
