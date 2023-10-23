//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/Property.h"
#include "ramses/client/ramses-client.h"

#include <iostream>

/**
 * This example demonstrates how to use primitive property types (int, bool, string etc.)
 */
int main()
{
    ramses::RamsesFramework framework{ ramses::RamsesFrameworkConfig{ ramses::EFeatureLevel_Latest } };
    ramses::RamsesClient* client = framework.createClient("client");
    ramses::Scene* scene = client->createScene(ramses::sceneId_t{ 123u });
    ramses::LogicEngine& logicEngine{ *scene->createLogicEngine() };

    // Create a simple script which multiplies two numbers and stores the result in a string
    ramses::LuaScript* multiplyScript = logicEngine.createLuaScript(R"(
        function interface(IN,OUT)
            IN.param1 = Type:Int32()
            IN.param2 = Type:Float()

            OUT.result = Type:String()
        end

        function run(IN,OUT)
            OUT.result = "Calculated result is: " .. IN.param1 * IN.param2
        end
    )");

    /**
     * Query the inputs of the script. The inputs are
     * stored in a "Property" instance and can be used to
     * get the information about available inputs and outputs
     */
    ramses::Property* inputs = multiplyScript->getInputs();

    for (size_t i = 0u; i < inputs->getChildCount(); ++i)
    {
        ramses::Property* input = inputs->getChild(i);
        std::cout << "Input: " << input->getName() << " is of type: " << ramses::GetLuaPrimitiveTypeName(input->getType()) << std::endl;
    }

    // We can do the same with the outputs
    const ramses::Property* outputs = multiplyScript->getOutputs();

    for (size_t i = 0u; i < outputs->getChildCount(); ++i)
    {
        const ramses::Property* output = outputs->getChild(i);
        std::cout << "Output: " << output->getName() << " is of type: " << ramses::GetLuaPrimitiveTypeName(output->getType()) << std::endl;
    }

    // Set some test values to the inputs before executing the script
    inputs->getChild("param1")->set<int32_t>(21);
    inputs->getChild("param2")->set<float>(2.f);

    // Update the logic engine (executes the script we created above)
    logicEngine.update();

    /**
     * After execution, we can get the calculated outputs
     * The getters of the values are returned as std::optional
     * to ensure the combination of name and type matches an existing input.
     */
    std::optional<std::string> result = outputs->getChild("result")->get<std::string>();
    if (result)
    {
        std::cout << *result << std::endl;
    }

    // To delete the script call the destroy method on the LogicEngine instance
    logicEngine.destroy(*multiplyScript);

    return 0;
}
