//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"

#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/TimerNode.h"
#include "ramses/client/logic/Property.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/LuaInterface.h"
#include "impl/logic/TimerNodeImpl.h"
#include "impl/logic/PropertyImpl.h"
#include "impl/ErrorReporting.h"
#include "internal/logic/SerializationMap.h"
#include "internal/logic/DeserializationMap.h"
#include "internal/logic/TypeData.h"
#include "internal/logic/EPropertySemantics.h"
#include "internal/logic/flatbuffers/generated/TimerNodeGen.h"
#include "flatbuffers/flatbuffers.h"
#include "LogicEngineTest_Base.h"
#include <numeric>
#include <thread>

namespace ramses::internal
{
    class ATimerNode : public ALogicEngine
    {
    };

    TEST_F(ATimerNode, IsCreated)
    {
        const auto timerNode = m_logicEngine->createTimerNode("timerNode");
        expectNoError();
        ASSERT_NE(nullptr, timerNode);
        EXPECT_EQ(timerNode, m_logicEngine->findObject<TimerNode>("timerNode"));

        EXPECT_EQ("timerNode", timerNode->getName());
    }

    TEST_F(ATimerNode, IsDestroyed)
    {
        const auto timerNode = m_logicEngine->createTimerNode("timerNode");
        EXPECT_TRUE(m_logicEngine->destroy(*timerNode));
        expectNoError();
        EXPECT_EQ(nullptr, m_logicEngine->findObject<TimerNode>("timerNode"));
    }

    TEST_F(ATimerNode, FailsToBeDestroyedIfFromOtherLogicInstance)
    {
        auto timerNode = m_logicEngine->createTimerNode("timerNode");

        auto& otherEngine = *m_scene->createLogicEngine();
        EXPECT_FALSE(otherEngine.destroy(*timerNode));
        EXPECT_EQ("Failed to destroy object 'timerNode [LogicObject ScnObjId=9]', cannot find it in this LogicEngine instance.", getLastErrorMessage());
    }

    TEST_F(ATimerNode, ChangesName)
    {
        const auto timerNode = m_logicEngine->createTimerNode("timerNode");

        timerNode->setName("an");
        EXPECT_EQ("an", timerNode->getName());
        EXPECT_EQ(timerNode, m_logicEngine->findObject<TimerNode>("an"));
        expectNoError();
    }

    TEST_F(ATimerNode, HasPropertiesAfterCreation)
    {
        const auto timerNode = m_logicEngine->createTimerNode("timerNode");

        const auto rootIn = timerNode->getInputs();
        EXPECT_EQ("", rootIn->getName());
        ASSERT_EQ(1u, rootIn->getChildCount());
        EXPECT_EQ("ticker_us", rootIn->getChild(0u)->getName());
        EXPECT_EQ(EPropertyType::Int64, rootIn->getChild(0u)->getType());

        const auto rootOut = timerNode->getOutputs();
        EXPECT_EQ("", rootOut->getName());
        ASSERT_EQ(1u, rootOut->getChildCount());
        EXPECT_EQ("ticker_us", rootOut->getChild(0u)->getName());
        EXPECT_EQ(EPropertyType::Int64, rootOut->getChild(0u)->getType());
    }

    TEST_F(ATimerNode, OutputsTicker_userTicker)
    {
        const auto timerNode = m_logicEngine->createTimerNode("timerNode");

        EXPECT_TRUE(timerNode->getInputs()->getChild(0u)->set<int64_t>(1000000));
        EXPECT_TRUE(m_logicEngine->update());
        EXPECT_EQ(1000000, *timerNode->getOutputs()->getChild(0u)->get<int64_t>());

        // +0.5 second
        EXPECT_TRUE(timerNode->getInputs()->getChild(0u)->set<int64_t>(1500000));
        EXPECT_TRUE(m_logicEngine->update());
        EXPECT_EQ(1500000, *timerNode->getOutputs()->getChild(0u)->get<int64_t>());

        // no change
        EXPECT_TRUE(timerNode->getInputs()->getChild(0u)->set<int64_t>(1500000));
        EXPECT_TRUE(m_logicEngine->update());
        EXPECT_EQ(1500000, *timerNode->getOutputs()->getChild(0u)->get<int64_t>());

        // +10 second
        EXPECT_TRUE(timerNode->getInputs()->getChild(0u)->set<int64_t>(11500000));
        EXPECT_TRUE(m_logicEngine->update());
        EXPECT_EQ(11500000, *timerNode->getOutputs()->getChild(0u)->get<int64_t>());
    }

    TEST_F(ATimerNode, OutputsTicker_autoTicker)
    {
        const auto timerNode = m_logicEngine->createTimerNode("timerNode");

        // auto ticker is enabled if ticker input == 0
        EXPECT_TRUE(timerNode->getInputs()->getChild(0u)->set<int64_t>(0));

        EXPECT_TRUE(m_logicEngine->update());
        const auto initialTicker = *timerNode->getOutputs()->getChild(0u)->get<int64_t>();
        EXPECT_TRUE(initialTicker > 0);

        auto ticker = initialTicker;
        for (int i = 0; i < 10; ++i)
        {
            EXPECT_TRUE(m_logicEngine->update());
            const auto nextTicker = *timerNode->getOutputs()->getChild(0u)->get<int64_t>();
            EXPECT_TRUE(nextTicker >= ticker);
            ticker = nextTicker;
            std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
        }

        // check that auto ticker is actually progressing
        EXPECT_GT(ticker, initialTicker);
    }

    TEST_F(ATimerNode, OutputsTickerInAutoModeWhenConnectedToInterface)
    {
        const std::string_view interfaceSrc = R"(
            function interface(inputs)
                inputs.someParam = Type:Int32()
                inputs.ticker = Type:Int64()
            end
        )";
        const auto luaInterface = m_logicEngine->createLuaInterface(interfaceSrc, "intf");
        const auto timerNode = m_logicEngine->createTimerNode("timerNode");

        EXPECT_TRUE(m_logicEngine->link(*luaInterface->getOutputs()->getChild("ticker"), *timerNode->getInputs()->getChild("ticker_us")));

        EXPECT_TRUE(m_logicEngine->update());
        const auto initialTicker = *timerNode->getOutputs()->getChild(0u)->get<int64_t>();
        EXPECT_TRUE(initialTicker > 0);

        // no value set, expect ticker running in auto
        int64_t ticker = initialTicker;
        for (int i = 0; i < 10; ++i)
        {
            EXPECT_TRUE(m_logicEngine->update());
            const auto nextTicker = *timerNode->getOutputs()->getChild(0u)->get<int64_t>();
            EXPECT_TRUE(nextTicker >= ticker);
            ticker = nextTicker;
            std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
        }
        // check that auto ticker is actually progressing
        EXPECT_GT(ticker, initialTicker);

        // explicitly set to 0 and expect no change
        EXPECT_TRUE(luaInterface->getInputs()->getChild("ticker")->set<int64_t>(0));

        for (int i = 0; i < 10; ++i)
        {
            EXPECT_TRUE(m_logicEngine->update());
            const auto nextTicker = *timerNode->getOutputs()->getChild(0u)->get<int64_t>();
            EXPECT_TRUE(nextTicker >= ticker);
            ticker = nextTicker;
            std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
        }
        // check that auto ticker is actually progressing
        EXPECT_GT(ticker, initialTicker);
    }

    TEST_F(ATimerNode, CanBeSerializedAndDeserialized)
    {
        withTempDirectory();
        {
            auto& otherEngine = *m_logicEngine;
            otherEngine.createTimerNode("timerNode");
            ASSERT_TRUE(saveToFileWithoutValidation("logic_timerNode.bin"));
        }

        ASSERT_TRUE(recreateFromFile("logic_timerNode.bin"));
        expectNoError();

        EXPECT_EQ(1u, m_logicEngine->getCollection<TimerNode>().size());
        const auto timerNode = m_logicEngine->findObject<TimerNode>("timerNode");
        ASSERT_TRUE(timerNode);
        EXPECT_EQ("timerNode", timerNode->getName());

        const auto rootIn = timerNode->getInputs();
        EXPECT_EQ("", rootIn->getName());
        ASSERT_EQ(1u, rootIn->getChildCount());
        EXPECT_EQ("ticker_us", rootIn->getChild(0u)->getName());
        EXPECT_EQ(EPropertyType::Int64, rootIn->getChild(0u)->getType());

        const auto rootOut = timerNode->getOutputs();
        EXPECT_EQ("", rootOut->getName());
        ASSERT_EQ(1u, rootOut->getChildCount());
        EXPECT_EQ("ticker_us", rootOut->getChild(0u)->getName());
        EXPECT_EQ(EPropertyType::Int64, rootOut->getChild(0u)->getType());
    }

    class ATimerNode_SerializationLifecycle : public ATimerNode
    {
    protected:
        enum class ESerializationIssue
        {
            AllValid,
            NameMissing,
            IdMissing,
            RootInMissing,
            RootOutMissing,
            PropertyInMissing,
            PropertyOutMissing,
            PropertyInWrongName,
            PropertyOutWrongName
        };

        std::unique_ptr<TimerNodeImpl> deserializeSerializedDataWithIssue(ESerializationIssue issue)
        {
            flatbuffers::FlatBufferBuilder flatBufferBuilder;
            SerializationMap serializationMap;
            DeserializationMap deserializationMap{ m_scene->impl() };

            {
                HierarchicalTypeData inputs = MakeStruct("", {});
                if (issue == ESerializationIssue::PropertyInWrongName)
                {
                    inputs.children.push_back(MakeType("wrongInput", EPropertyType::Int64));
                }
                else if (issue != ESerializationIssue::PropertyInMissing)
                {
                    inputs.children.push_back(MakeType("ticker_us", EPropertyType::Int64));
                }
                auto inputsImpl = std::make_unique<PropertyImpl>(std::move(inputs), EPropertySemantics::ScriptInput);

                HierarchicalTypeData outputs = MakeStruct("", {});
                if (issue == ESerializationIssue::PropertyOutWrongName)
                {
                    outputs.children.push_back(MakeType("wrongOutput", EPropertyType::Int64));
                }
                else if (issue != ESerializationIssue::PropertyOutMissing)
                {
                    outputs.children.push_back(MakeType("ticker_us", EPropertyType::Int64));
                }
                auto outputsImpl = std::make_unique<PropertyImpl>(std::move(outputs), EPropertySemantics::ScriptOutput);

                const auto timerNodeFB = rlogic_serialization::CreateTimerNode(
                    flatBufferBuilder,
                    rlogic_serialization::CreateLogicObject(flatBufferBuilder,
                        issue == ESerializationIssue::NameMissing ? 0 : flatBufferBuilder.CreateString("timerNode"),
                        issue == ESerializationIssue::IdMissing ? 0 : 1u),
                    issue == ESerializationIssue::RootInMissing ? 0 : PropertyImpl::Serialize(*inputsImpl, flatBufferBuilder, serializationMap),
                    issue == ESerializationIssue::RootOutMissing ? 0 : PropertyImpl::Serialize(*outputsImpl, flatBufferBuilder, serializationMap)
                );

                flatBufferBuilder.Finish(timerNodeFB);
            }

            const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::TimerNode>(flatBufferBuilder.GetBufferPointer());
            return TimerNodeImpl::Deserialize(serialized, m_errorReporting, deserializationMap);
        }

        ErrorReporting m_errorReporting;
    };

    TEST_F(ATimerNode_SerializationLifecycle, FailsDeserializationIfEssentialDataMissing)
    {
        EXPECT_TRUE(deserializeSerializedDataWithIssue(ATimerNode_SerializationLifecycle::ESerializationIssue::AllValid));
        EXPECT_FALSE(m_errorReporting.getError().has_value());

        for (const auto issue : { ESerializationIssue::NameMissing, ESerializationIssue::IdMissing, ESerializationIssue::RootInMissing, ESerializationIssue::RootOutMissing })
        {
            EXPECT_FALSE(deserializeSerializedDataWithIssue(issue));
            ASSERT_TRUE(m_errorReporting.getError().has_value());
            EXPECT_EQ("Fatal error during loading of TimerNode from serialized data: missing name, id or in/out property data!", m_errorReporting.getError()->message);
            m_errorReporting.reset();
        }
    }

    TEST_F(ATimerNode_SerializationLifecycle, FailsDeserializationIfInvalidOrMissingProperties)
    {
        for (const auto issue : { ESerializationIssue::PropertyInMissing, ESerializationIssue::PropertyOutMissing, ESerializationIssue::PropertyInWrongName, ESerializationIssue::PropertyOutWrongName })
        {
            EXPECT_FALSE(deserializeSerializedDataWithIssue(issue));
            ASSERT_TRUE(m_errorReporting.getError().has_value());
            EXPECT_EQ("Fatal error during loading of TimerNode 'timerNode': missing or invalid properties!", m_errorReporting.getError()->message);
            m_errorReporting.reset();
        }
    }
}
