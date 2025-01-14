//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandOutputGlobal.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandOutputConnection.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandClient.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandDisplay.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandGlobal.h"
#include "internal/Core/Utils/ThreadLocalLogForced.h"

#include <cassert>

namespace ramses::internal
{
    WaylandOutputGlobal::WaylandOutputGlobal(const WaylandOutputParams& waylandOutputParams)
        : m_waylandOutputParams(waylandOutputParams)
    {
    }

    bool WaylandOutputGlobal::init(IWaylandDisplay& serverDisplay)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandOutputGlobal::init");

        const int maximumSupportedOutputInterfaceVersion = WL_OUTPUT_RELEASE_SINCE_VERSION;

        const int supportedOutputInterfaceVersion = std::min(maximumSupportedOutputInterfaceVersion,  wl_output_interface.version);
        m_waylandGlobal = serverDisplay.createGlobal(&wl_output_interface, supportedOutputInterfaceVersion, this, GlobalBindCallback);

        if (!m_waylandGlobal)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandOutputGlobal::init(): Failed to create global!");
            return false;
        }

        return true;
    }

    WaylandOutputGlobal::~WaylandOutputGlobal()
    {
        assert(!m_waylandGlobal);
    }

    void WaylandOutputGlobal::destroy()
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandOutputGlobal::~WaylandOutputGlobal");

        delete m_waylandGlobal;
        m_waylandGlobal = nullptr;
    }

    void WaylandOutputGlobal::globalBind(IWaylandClient& client, uint32_t version, uint32_t id)
    {
        new WaylandOutputConnection(m_waylandOutputParams, client, version, id);
    }

    void WaylandOutputGlobal::GlobalBindCallback(wl_client* client, void* data, uint32_t version, uint32_t id)
    {
        auto* waylandOutputGlobal = static_cast<WaylandOutputGlobal*>(data);
        WaylandClient waylandClient(client);
        waylandOutputGlobal->globalBind(waylandClient, version, id);
    }
}
