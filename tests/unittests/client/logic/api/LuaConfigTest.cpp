//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "LogTestUtils.h"

#include "ramses/client/logic/LuaConfig.h"
#include "ramses/client/logic/LogicEngine.h"
#include "impl/logic/LuaConfigImpl.h"
#include "LogicEngineTest_Base.h"

#include "fmt/format.h"

namespace ramses::internal
{
    class ALuaConfig : public ALogicEngine
    {
    protected:
        LuaModule* m_module{ m_logicEngine->createLuaModule(R"(
            local mymath = {}
            function mymath.add(a,b)
                print(a+b)
            end
            return mymath
        )") };

        std::string m_errorMessage;
        ScopedLogContextLevel m_scopedLogs{CONTEXT_CLIENT, ELogLevel::Error, [&](ELogLevel msgType, std::string_view message) {
            m_errorMessage = message;
            if (msgType != ELogLevel::Error)
            {
                ASSERT_TRUE(false) << "Should be error!";
            }
        }};
    };

    TEST_F(ALuaConfig, IsCreated)
    {
        LuaConfig config;
        EXPECT_TRUE(config.impl().getModuleMapping().empty());
        EXPECT_FALSE(config.impl().hasDebugLogFunctionsEnabled());
    }

    TEST_F(ALuaConfig, IsCopied)
    {
        LuaConfig config;
        config.addDependency("mod1", *m_module);
        config.addDependency("mod2", *m_module);
        config.addStandardModuleDependency(EStandardModule::Debug);
        config.enableDebugLogFunctions();

        LuaConfig configCopy(config);
        EXPECT_EQ(config.impl().getModuleMapping(), configCopy.impl().getModuleMapping());
        EXPECT_EQ(config.impl().getStandardModules(), configCopy.impl().getStandardModules());
        EXPECT_EQ(config.impl().hasDebugLogFunctionsEnabled(), configCopy.impl().hasDebugLogFunctionsEnabled());
    }

    TEST_F(ALuaConfig, IsCopyAssigned)
    {
        LuaConfig config;
        config.addDependency("mod1", *m_module);
        config.addDependency("mod2", *m_module);
        config.addStandardModuleDependency(EStandardModule::Debug);

        // Copy assignment
        LuaConfig configCopy;
        configCopy = config;
        EXPECT_EQ(config.impl().getModuleMapping(), configCopy.impl().getModuleMapping());
        EXPECT_EQ(config.impl().getStandardModules(), configCopy.impl().getStandardModules());
        EXPECT_EQ(config.impl().hasDebugLogFunctionsEnabled(), configCopy.impl().hasDebugLogFunctionsEnabled());
    }

    TEST_F(ALuaConfig, IsMoved)
    {
        LuaConfig config;
        config.addDependency("mod1", *m_module);
        config.addStandardModuleDependency(EStandardModule::Debug);
        config.enableDebugLogFunctions();

        LuaConfig movedConfig(std::move(config));

        EXPECT_EQ(movedConfig.impl().getModuleMapping().at("mod1"), m_module);
        EXPECT_THAT(movedConfig.impl().getStandardModules(), ::testing::ElementsAre(EStandardModule::Debug));
        EXPECT_TRUE(movedConfig.impl().hasDebugLogFunctionsEnabled());
    }

    TEST_F(ALuaConfig, IsMoveAssigned)
    {
        LuaConfig config;
        config.addDependency("mod1", *m_module);
        config.addStandardModuleDependency(EStandardModule::Debug);

        LuaConfig movedAssigned;
        movedAssigned = std::move(config);

        EXPECT_EQ(movedAssigned.impl().getModuleMapping().at("mod1"), m_module);
        EXPECT_THAT(movedAssigned.impl().getStandardModules(), ::testing::ElementsAre(EStandardModule::Debug));
        EXPECT_FALSE(movedAssigned.impl().hasDebugLogFunctionsEnabled());
    }

    TEST_F(ALuaConfig, ProducesErrorWhenCreatingLuaScriptUsingModuleUnderInvalidName)
    {
        std::vector<std::string> invalidLabels = {
            "",
            "!invalid",
            "invalid%",
            "3invalid",
            "42",
            "\n",
        };

        LuaConfig config;
        for (const auto& invalidLabel : invalidLabels)
        {
            EXPECT_FALSE(config.addDependency(invalidLabel, *m_module));

            EXPECT_EQ(fmt::format("Failed to add dependency '{}'! The alias name should be a valid Lua label.", invalidLabel), m_errorMessage);
        }
    }

    TEST_F(ALuaConfig, ProducesErrorWhenUsingTheSameLabelTwice)
    {
        LuaConfig config;

        ASSERT_TRUE(config.addDependency("module", *m_module));
        EXPECT_FALSE(config.addDependency("module", *m_module));

        EXPECT_EQ("Module dependencies must be uniquely aliased! Alias 'module' is already used!", m_errorMessage);
    }

    TEST_F(ALuaConfig, ProducesErrorWhenUsingStandardModuleAsAliasName)
    {
        LuaConfig config;

        EXPECT_FALSE(config.addDependency("math", *m_module));
        EXPECT_EQ("Failed to add dependency 'math'! The alias collides with a standard library name!", m_errorMessage);
        EXPECT_FALSE(config.addDependency("string", *m_module));
        EXPECT_EQ("Failed to add dependency 'string'! The alias collides with a standard library name!", m_errorMessage);
        EXPECT_FALSE(config.addDependency("debug", *m_module));
        EXPECT_EQ("Failed to add dependency 'debug'! The alias collides with a standard library name!", m_errorMessage);
        EXPECT_FALSE(config.addDependency("table", *m_module));
        EXPECT_EQ("Failed to add dependency 'table'! The alias collides with a standard library name!", m_errorMessage);
    }

    TEST_F(ALuaConfig, CanEnableDebugLogFunctions)
    {
        LuaConfig config;
        EXPECT_FALSE(config.impl().hasDebugLogFunctionsEnabled());
        config.enableDebugLogFunctions();
        EXPECT_TRUE(config.impl().hasDebugLogFunctionsEnabled());
    }

    class ALuaConfig_StdModules : public ::testing::Test
    {
    };

    TEST_F(ALuaConfig_StdModules, AddsStandardModuleDependencies)
    {
        LuaConfig config;

        EXPECT_TRUE(config.impl().getStandardModules().empty());

        ASSERT_TRUE(config.addStandardModuleDependency(EStandardModule::Base));
        EXPECT_THAT(config.impl().getStandardModules(), ::testing::ElementsAre(EStandardModule::Base));
        ASSERT_TRUE(config.addStandardModuleDependency(EStandardModule::String));
        EXPECT_THAT(config.impl().getStandardModules(), ::testing::ElementsAre(EStandardModule::Base, EStandardModule::String));
    }

    TEST_F(ALuaConfig_StdModules, CantAddTheSameStandardModuleTwice)
    {
        LuaConfig config;

        EXPECT_TRUE(config.impl().getStandardModules().empty());

        ASSERT_TRUE(config.addStandardModuleDependency(EStandardModule::Base));
        EXPECT_FALSE(config.addStandardModuleDependency(EStandardModule::Base));
    }

    TEST_F(ALuaConfig_StdModules, AddsAllModulesAtOnce)
    {
        LuaConfig config;

        EXPECT_TRUE(config.addStandardModuleDependency(EStandardModule::All));

        EXPECT_THAT(config.impl().getStandardModules(), ::testing::ElementsAre(
            EStandardModule::Base,
            EStandardModule::String,
            EStandardModule::Table,
            EStandardModule::Math,
            EStandardModule::Debug));
    }
}
