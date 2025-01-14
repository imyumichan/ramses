//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFrameworkConfig.h"
#include "ramses/renderer/RendererConfig.h"
#include "ramses/renderer/DisplayConfig.h"

#include <memory>
#include <vector>
#include <string>

namespace ramses
{
    class RamsesClient;
    class Scene;
    class RamsesRenderer;
    class ValidationReport;
}

namespace CLI
{
    class App;
    class Option;
}

namespace ramses::internal
{
    class SceneViewer
    {
    public:
        SceneViewer();

        void registerOptions(CLI::App& cli);

        int run();

    private:
        enum class GuiMode
        {
            Off,
            On,       ///< Loaded scene rendered to offscreen buffer (configurable size and position)
            Overlay,  ///< Debugging gui overlaps the rendered scene (no offscreen buffer)
            Only,     ///< Only shows the gui, not the scene itself
        };

        int loadAndRenderScene(const std::string& sceneFile);
        void validateContent(ramses::ValidationReport& report, const ramses::Scene& scene) const;

        std::string m_sceneName;
        GuiMode     m_guiMode = GuiMode::Overlay;
        bool        m_noValidation = false;
        bool        m_noSkub = false;
        std::string m_validationOutput;
        std::string m_screenshotFile;

        ramses::RamsesFrameworkConfig  m_frameworkConfig{ramses::EFeatureLevel_Latest};
        ramses::RendererConfig         m_rendererConfig;
        ramses::DisplayConfig          m_displayConfig;

        CLI::Option* m_width = nullptr;
        CLI::Option* m_height = nullptr;
    };
}
