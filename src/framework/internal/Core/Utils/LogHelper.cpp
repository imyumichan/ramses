//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Core/Utils/LogHelper.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/Core/Utils/StringUtils.h"

#include <string_view>
#include <algorithm>
#include <cctype>


namespace ramses::internal
{
    namespace LogHelper
    {
        bool StringToLogLevel(std::string_view str, ELogLevel& logLevel)
        {
            // Trim string because envvar on windows may have space(s) appended
            std::string lowered{StringUtils::TrimView(str)};
            std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) { return static_cast<std::string::value_type>(std::tolower(c)); });

            if (lowered == "trace" || lowered == "6")
            {
                logLevel = ELogLevel::Trace;
                return true;
            }
            if (lowered == "debug" || lowered == "5")
            {
                logLevel = ELogLevel::Debug;
                return true;
            }
            if (lowered == "info" || lowered == "4")
            {
                logLevel = ELogLevel::Info;
                return true;
            }
            if (lowered == "warn" || lowered == "3")
            {
                logLevel = ELogLevel::Warn;
                return true;
            }
            if (lowered == "error" || lowered == "2")
            {
                logLevel = ELogLevel::Error;
                return true;
            }
            if (lowered == "fatal" || lowered == "1")
            {
                logLevel = ELogLevel::Fatal;
                return true;
            }
            if (lowered == "off" || lowered == "0")
            {
                logLevel = ELogLevel::Off;
                return true;
            }
            return false;
        }

        std::vector<ContextFilter> ParseContextFilters(const std::string& filterCommand)
        {
            std::vector<ContextFilter> returnValue;
            // loop over commands separated by ','
            size_t currentCommandStart = 0;
            do
            {
                size_t currentCommandEnd = filterCommand.find(',', currentCommandStart);
                if (currentCommandEnd == 0 || currentCommandEnd == std::string::npos)
                {
                    // no more ',', so command goes until end of string
                    currentCommandEnd = filterCommand.size();
                }
                const size_t positionOfColon = filterCommand.find(':', currentCommandStart);
                if (positionOfColon != std::string::npos && currentCommandStart < positionOfColon)
                {
                    const auto lengthOfLogLevelString = positionOfColon - currentCommandStart;
                    const std::string logLevelStr = filterCommand.substr(currentCommandStart, lengthOfLogLevelString);
                    ELogLevel logLevel;
                    if (StringToLogLevel(logLevelStr, logLevel))
                    {
                        const auto offsetOfLogContextPattern = lengthOfLogLevelString + 1;
                        const std::string contextPattern = filterCommand.substr(currentCommandStart + offsetOfLogContextPattern, currentCommandEnd - currentCommandStart - offsetOfLogContextPattern);
                        if (!contextPattern.empty())
                        {
                            returnValue.emplace_back(logLevel, contextPattern);
                        }
                    }
                    else
                    {
                        LOG_WARN(CONTEXT_FRAMEWORK, "LogHelper::ParseContextFilters: Skip unknown log level '" << logLevelStr << "'");
                    }
                }
                currentCommandStart = currentCommandEnd + 1;
            } while (currentCommandStart <= filterCommand.size());
            return returnValue;
        }

        ELogLevel GetLoglevelFromInt(int32_t logLevelInt)
        {
            switch (logLevelInt)
            {
            case static_cast<int32_t>(ELogLevel::Off) : return ELogLevel::Off;
            case static_cast<int32_t>(ELogLevel::Fatal) : return ELogLevel::Fatal;
            case static_cast<int32_t>(ELogLevel::Error) : return ELogLevel::Error;
            case static_cast<int32_t>(ELogLevel::Warn) : return ELogLevel::Warn;
            case static_cast<int32_t>(ELogLevel::Info) : return ELogLevel::Info;
            case static_cast<int32_t>(ELogLevel::Debug) : return ELogLevel::Debug;
            case static_cast<int32_t>(ELogLevel::Trace) : return ELogLevel::Trace;
            default:                                   return ELogLevel::Trace;
            }
        }
    }
}
