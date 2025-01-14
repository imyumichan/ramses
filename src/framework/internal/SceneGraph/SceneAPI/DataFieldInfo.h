//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/EDataType.h"
#include "internal/SceneGraph/SceneAPI/EFixedSemantics.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"

namespace ramses::internal
{
    struct DataFieldInfo
    {
        explicit DataFieldInfo(EDataType dataType_ = EDataType::Invalid, uint32_t elementCount_ = 1u, EFixedSemantics semantics_ = EFixedSemantics::Invalid)
            : dataType(dataType_)
            , elementCount(elementCount_)
            , semantics(semantics_)
        {
        }

        bool operator==(const DataFieldInfo& other) const
        {
            return dataType == other.dataType
                && elementCount == other.elementCount
                && semantics == other.semantics;
        }

        bool operator!=(const DataFieldInfo& other) const
        {
            return !operator==(other);
        }

        EDataType       dataType;
        uint32_t          elementCount;
        EFixedSemantics semantics;
    };

    using DataFieldInfoVector = std::vector<DataFieldInfo>;
}
