//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Wayland/IVI/SystemCompositorController/SystemCompositorController_Wayland_IVI.h"
#include "internal/Platform/Wayland/IVI/SystemCompositorController/IVIControllerSurface.h"
#include "internal/Platform/Wayland/IVI/SystemCompositorController/IVIControllerScreen.h"
#include "internal/Platform/Wayland/IVI/SystemCompositorController/WaylandOutput.h"
#include "internal/Platform/Wayland/WaylandEnvironmentUtils.h"
#include "internal/PlatformAbstraction/PlatformMath.h"
#include "internal/PlatformAbstraction/Collections/StringOutputStream.h"
#include "internal/Core/Utils/LogMacros.h"

#include <algorithm>
#include "poll.h"

#include <algorithm>
#include <cassert>

namespace ramses::internal
{
    SystemCompositorController_Wayland_IVI::SystemCompositorController_Wayland_IVI(std::string_view waylandDisplay)
        : m_waylandDisplay(waylandDisplay)
    {
        LOG_INFO(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::SystemCompositorController_Wayland_IVI (" << waylandDisplay << ")");
    }

    SystemCompositorController_Wayland_IVI::~SystemCompositorController_Wayland_IVI()
    {
        m_controllerSurfaces.clear();
        m_controllerScreens.clear();
        m_waylandOutputs.clear();

        if (nullptr != m_registry)
        {
            wl_registry_destroy(m_registry);
        }

        if (nullptr != m_controller)
        {
            ivi_controller_destroy(m_controller);
        }

        if (nullptr != m_display)
        {
            wl_display_roundtrip(m_display);
            wl_display_disconnect(m_display);
        }
    }

    bool SystemCompositorController_Wayland_IVI::init()
    {
        LOG_INFO(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::init");

        WaylandEnvironmentUtils::LogEnvironmentState(m_waylandDisplay);
        m_display = wl_display_connect(m_waylandDisplay.empty()? nullptr : m_waylandDisplay.c_str());
        if (!m_display)
        {
            LOG_ERROR(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::init wl_display_connect() failed!");
            return false;
        }

        m_registry = wl_display_get_registry(m_display);

        wl_registry_add_listener(m_registry, &m_registryListener, this);

        // First roundtrip to get registryHandleGlobal beeing called
        wl_display_roundtrip(m_display);

        if (nullptr == m_controller)
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "SystemCompositorController_Wayland_IVI::init ivi_controller interface not available!");
            return false;
        }

        // Second roundtrip to receive all events from ivi_controller_listener (currently existing screens, layers and surfaces)
        wl_display_roundtrip(m_display);

        return true;
    }

    void SystemCompositorController_Wayland_IVI::update()
    {
        pollfd pfd{};
        pfd.fd      = wl_display_get_fd(m_display);
        pfd.events  = POLLIN;
        pfd.revents = 0;

        wl_display_dispatch_pending(m_display);

        if (-1 == poll(&pfd, 1, 0))
        {
            return;
        }

        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        if ((pfd.revents & POLLIN) != 0)
        {
            wl_display_dispatch(m_display);
        }
    }

    void SystemCompositorController_Wayland_IVI::listIVISurfaces() const
    {
        std::vector<uint32_t> sortedList;
        sortedList.reserve(m_controllerSurfaces.size());
        for (const auto& controllerSurface : m_controllerSurfaces)
        {
            sortedList.push_back(controllerSurface->getIVIId().getValue());
        }
        std::sort(sortedList.begin(), sortedList.end());

        // This log message is checked by test_testclient_system_compositor_controller.py, so be aware of changing it.
        LOG_INFO_F(CONTEXT_RENDERER, ([&](StringOutputStream& sos) {
                    sos << "SystemCompositorController_Wayland_IVI::listIVISurfaces Known ivi-surface ids:";
                    for (auto id : sortedList)
                    {
                        sos << ' ' << id;
                    }
                }));

        // Log surface statistics
        for (const auto& controllerSurface : m_controllerSurfaces)
        {
            controllerSurface->sendStats();
        }
        // Needed to ensure, that statistic info of surfaces is printed out by the listener of the controllerSurface
        wl_display_roundtrip(m_display);
    }

    bool SystemCompositorController_Wayland_IVI::setSurfaceVisibility(WaylandIviSurfaceId surfaceId, bool visibility)
    {
        LOG_INFO(CONTEXT_RENDERER,
                 "SystemCompositorController_Wayland_IVI::setSurfaceVisibility "
                     << surfaceId << " visibility: " << visibility);

        IVIControllerSurface& controllerSurface = getOrCreateControllerSurface(surfaceId);
        controllerSurface.setVisibility(visibility);

        commitAndFlushControllerChanges();
        return true;
    }

    bool SystemCompositorController_Wayland_IVI::setSurfaceOpacity(WaylandIviSurfaceId surfaceId, float opacity)
    {
        LOG_INFO(CONTEXT_RENDERER,
                 "SystemCompositorController_Wayland_IVI::setOpacity " << surfaceId << " opacity: " << opacity);

        IVIControllerSurface& controllerSurface = getOrCreateControllerSurface(surfaceId);

        // clamp opacity to valid range, convert it to fixed point representation, and set it
        opacity = std::min(std::max(0.f, opacity), 1.f);
        controllerSurface.setOpacity(opacity);

        commitAndFlushControllerChanges();
        return true;
    }

    bool SystemCompositorController_Wayland_IVI::setSurfaceDestinationRectangle(
        WaylandIviSurfaceId surfaceId, int32_t x, int32_t y, int32_t width, int32_t height)
    {
        LOG_INFO(CONTEXT_RENDERER,
                 "SystemCompositorController_Wayland_IVI::setSurfaceDestinationRectangle "
                     << surfaceId << " position: (" << x << ", " << y << ", " << width << ", " << height
                     << ")");

        IVIControllerSurface& controllerSurface = getOrCreateControllerSurface(surfaceId);

        controllerSurface.setDestinationRectangle(x, y, width, height);

        commitAndFlushControllerChanges();
        return true;
    }

    bool SystemCompositorController_Wayland_IVI::doScreenshot(std::string_view fileName, int32_t screenIviId)
    {
        // find screen with id
        IVIControllerScreen* screen = nullptr;
        if (screenIviId == -1)
        {
            // expect single screen
            if (m_controllerScreens.size() != 1)
            {
                LOG_WARN(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::screenshot fileName " << fileName << " for screenId " << screenIviId <<
                         " failed because found " << m_controllerScreens.size() << " screens");
                return false;
            }
            screen = m_controllerScreens.begin()->get();
        }
        else
        {
            // expect id exists
            for (const auto& controllerScreen : m_controllerScreens)
            {
                if (controllerScreen->getScreenId() == static_cast<uint32_t>(screenIviId))
                    screen = controllerScreen.get();
            }
            if (!screen)
            {
                LOG_WARN(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::screenshot fileName " << fileName << " for screenId " << screenIviId <<
                         " failed because sceenId not found");
                return false;
            }
        }

        // trigger screenshot
        screen->takeScreenshot(std::string{fileName});

        // ensure that all compositor operations have finished
        wl_display_roundtrip(m_display);

        LOG_INFO(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::screenshot: Saved screenshot for screen "
                 << screen->getScreenId() << " as " << fileName);

        return true;
    }

    bool SystemCompositorController_Wayland_IVI::addSurfaceToLayer(WaylandIviSurfaceId surfaceId,
                                                                   WaylandIviLayerId   layerId)
    {
        LOG_INFO(CONTEXT_RENDERER,
                 "SystemCompositorController_Wayland_IVI::addSurfaceToLayer " << surfaceId << " " << layerId);

        // Workaround for bug in compositor, create a new ivi_controller_layer here, otherwise the surface list of the
        // layer can get wrong, when another application has also changed it in the meantime.
        ivi_controller_layer*  controllerLayer = ivi_controller_layer_create(m_controller, layerId.getValue(), 0, 0);
        if (nullptr == controllerLayer)
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "SystemCompositorController_Wayland_IVI::addSurfaceToLayer ivi_controller_layer_create failed, " << layerId);
            return false;
        }

        IVIControllerSurface& controllerSurface = getOrCreateControllerSurface(surfaceId);

        ivi_controller_surface* nativeWaylandControllerSurface = controllerSurface.getNativeWaylandControllerSurface();
        if (nullptr != nativeWaylandControllerSurface)
        {
            ivi_controller_layer_add_surface(controllerLayer, nativeWaylandControllerSurface);
            commitAndFlushControllerChanges();
        }
        else
        {
            LOG_ERROR(
                CONTEXT_RENDERER,
                "SystemCompositorController_Wayland_IVI::addSurfaceToLayer nativeWaylandControllerSurface is nullptr!");
            assert(false);
        }

        ivi_controller_layer_destroy(controllerLayer, 0);
        return true;
    }

    bool SystemCompositorController_Wayland_IVI::removeSurfaceFromLayer(WaylandIviSurfaceId surfaceId,
                                                                        WaylandIviLayerId   layerId)
    {
        LOG_INFO(CONTEXT_RENDERER,
                 "SystemCompositorController_Wayland_IVI::removeSurfaceFromLayer " << surfaceId << " " << layerId);

        // Workaround for bug in compositor, create a new ivi_controller_layer here, otherwise the surface list of the
        // layer can get wrong, when another application has also changed it in the meantime.
        ivi_controller_layer* controllerLayer = ivi_controller_layer_create(m_controller, layerId.getValue(), 0, 0);
        if (nullptr == controllerLayer)
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "SystemCompositorController_Wayland_IVI::removeSurfaceFromLayer ivi_controller_layer_create "
                      "failed " << layerId);
            return false;
        }

        IVIControllerSurface* controllerSurface = getControllerSurface(surfaceId);
        if (nullptr == controllerSurface)
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "SystemCompositorController_Wayland_IVI::removeSurfaceFromLayer " << surfaceId << " does not exist!");
            return false;
        }

        ivi_controller_surface* nativeWaylandControllerSurface = controllerSurface->getNativeWaylandControllerSurface();
        if (nullptr != nativeWaylandControllerSurface)
        {
            ivi_controller_layer_remove_surface(controllerLayer, nativeWaylandControllerSurface);
            commitAndFlushControllerChanges();
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "SystemCompositorController_Wayland_IVI::removeSurfaceFromLayer nativeWaylandControllerSurface "
                      "is nullptr!");
            assert(false);
        }

        ivi_controller_layer_destroy(controllerLayer, 0);
        return true;
    }

    bool SystemCompositorController_Wayland_IVI::destroySurface(WaylandIviSurfaceId surfaceId)
    {
        LOG_INFO(CONTEXT_RENDERER,
                 "SystemCompositorController_Wayland_IVI::destroySurface " << surfaceId);

        IVIControllerSurface* controllerSurface = getControllerSurface(surfaceId);

        if (nullptr == controllerSurface)
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "SystemCompositorController_Wayland_IVI::destroySurface " << surfaceId << " does not exist!");
            return false;
        }
        controllerSurface->destroy();
        commitAndFlushControllerChanges();

        deleteControllerSurface(*controllerSurface);
        return true;
    }

    bool SystemCompositorController_Wayland_IVI::setLayerVisibility(WaylandIviLayerId layerId, bool visibility)
    {
        LOG_INFO(CONTEXT_RENDERER,
                 "SystemCompositorController_Wayland_IVI::setLayerVisibility "
                     << layerId << " visibility: " << visibility);

        // Workaround for bug in compositor, create a new ivi_controller_layer here, otherwise the surface list of the
        // layer can get wrong, when another application has also changed it in the meantime.
        ivi_controller_layer* controllerLayer = ivi_controller_layer_create(m_controller, layerId.getValue(), 0, 0);
        if (nullptr == controllerLayer)
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "SystemCompositorController_Wayland_IVI::setLayerVisibility ivi_controller_layer_create "
                      "failed, " << layerId);
            return false;
        }

        ivi_controller_layer_set_visibility(controllerLayer, (visibility) ? 1 : 0);

        commitAndFlushControllerChanges();

        ivi_controller_layer_destroy(controllerLayer, 0);
        return true;
    }

    void SystemCompositorController_Wayland_IVI::deleteControllerSurface(IVIControllerSurface& controllerSurface)
    {
        auto it = std::find_if(std::begin(m_controllerSurfaces), std::end(m_controllerSurfaces), [&](const auto& s){ return s.get() == & controllerSurface; });

        if (it == m_controllerSurfaces.end())
        {
            LOG_ERROR(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::removeControllerSurface failed !");
            assert(false);
            return;
        }

        m_controllerSurfaces.erase(it);
    }

    IVIControllerSurface* SystemCompositorController_Wayland_IVI::getControllerSurface(WaylandIviSurfaceId iviId) const
    {
        auto it = std::find_if(std::begin(m_controllerSurfaces), std::end(m_controllerSurfaces), [&](const auto& s){ return s->getIVIId() == iviId; });

        if (it == m_controllerSurfaces.end())
            return nullptr;

        return it->get();
    }

    IVIControllerScreen*  SystemCompositorController_Wayland_IVI::getControllerScreen(uint32_t screenId) const
    {
        auto it = std::find_if(std::begin(m_controllerScreens), std::end(m_controllerScreens), [&](const auto& s){ return s->getScreenId() == screenId; });

        if (it == m_controllerScreens.end())
            return nullptr;

        return it->get();
    }

    IVIControllerSurface& SystemCompositorController_Wayland_IVI::getOrCreateControllerSurface(WaylandIviSurfaceId iviId)
    {
        IVIControllerSurface* controllerSurface = getControllerSurface(iviId);
        if (nullptr == controllerSurface)
        {
            ivi_controller_surface* nativeControllerSurface = ivi_controller_surface_create(m_controller, iviId.getValue());

            if (nullptr == nativeControllerSurface)
            {
                LOG_ERROR(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::getOrCreateControllerSurface " << iviId);
            }

            controllerSurface = new IVIControllerSurface(nativeControllerSurface, iviId, *this);

            m_controllerSurfaces.emplace_back(controllerSurface);
        }
        return *controllerSurface;
    }

    void SystemCompositorController_Wayland_IVI::commitAndFlushControllerChanges()
    {
        ivi_controller_commit_changes(m_controller);
        wl_display_flush(m_display);
    }

    void SystemCompositorController_Wayland_IVI::registryHandleGlobal(wl_registry* registry, uint32_t name, const char* interface, [[maybe_unused]] uint32_t version)
    {
        // Binding the wl_output is needed, otherwise the controller screens don't come in.
        if (std::string_view("wl_output") == interface)
        {
            m_waylandOutputs.emplace_back(std::make_unique<WaylandOutput>(registry, name));
        }

        if (std::string_view("ivi_controller") == interface)
        {
            assert(nullptr == m_controller);
            m_controller = static_cast<ivi_controller*>(wl_registry_bind(registry, name, &ivi_controller_interface, 1));
            ivi_controller_add_listener(m_controller, &m_iviControllerListener, this);
        }
    }

    void SystemCompositorController_Wayland_IVI::iviControllerHandleScreen([[maybe_unused]] ivi_controller* controller,
                                                                           uint32_t                         id_screen,
                                                                           ivi_controller_screen*           nativeControllerScreen)
    {
        LOG_INFO(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::iviControllerHandleScreen Detected ivi-screen: " << id_screen);

        if (nullptr != getControllerScreen(id_screen))
        {
            LOG_ERROR(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::iviControllerHandleScreen Screen with id " << id_screen << " already registered!");
            assert(false);
            return;
        }

        if (nullptr == nativeControllerScreen)
        {
            LOG_ERROR(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::iviControllerHandleScreen nativeControllerScreen is nullptr!");
            assert(false);
            return;
        }

        m_controllerScreens.emplace_back(std::make_unique<IVIControllerScreen>(*nativeControllerScreen, id_screen));
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void SystemCompositorController_Wayland_IVI::iviControllerHandleLayer([[maybe_unused]] ivi_controller* controller, [[maybe_unused]] uint32_t id_layer)
    {
        LOG_INFO(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::iviControllerHandleLayer Detected " << WaylandIviLayerId(id_layer));
    }

    void SystemCompositorController_Wayland_IVI::iviControllerHandleSurface([[maybe_unused]] ivi_controller* controller, uint32_t iviID)
    {
        const WaylandIviSurfaceId surfaceId{iviID};
        LOG_INFO(CONTEXT_RENDERER, "SystemCompositorController_Wayland_IVI::iviControllerHandleSurface Detected " << surfaceId);

        getOrCreateControllerSurface(surfaceId);
    }

    void SystemCompositorController_Wayland_IVI::RegistryHandleGlobalCallback(
        void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
    {
        auto* systemCompositorController =
            static_cast<SystemCompositorController_Wayland_IVI*>(data);
        systemCompositorController->registryHandleGlobal(registry, name, interface, version);
    }

    void SystemCompositorController_Wayland_IVI::RegistryHandleGlobalRemoveCallback([[maybe_unused]] void*        data,
                                                                                    [[maybe_unused]] wl_registry* wl_registry,
                                                                                    [[maybe_unused]] uint32_t     name)
    {
    }

    void SystemCompositorController_Wayland_IVI::IVIControllerHandleScreenCallback(void*                  data,
                                                                                   ivi_controller*        controller,
                                                                                   uint32_t               id_screen,
                                                                                   ivi_controller_screen* screen)
    {
        auto* systemCompositorController =
            static_cast<SystemCompositorController_Wayland_IVI*>(data);
        systemCompositorController->iviControllerHandleScreen(controller, id_screen, screen);
    }

    void SystemCompositorController_Wayland_IVI::IVIControllerHandleLayerCallback(void*                  data,
                                                                                  struct ivi_controller* controller,
                                                                                  uint32_t               id_layer)
    {
        auto* systemCompositorController =
            static_cast<SystemCompositorController_Wayland_IVI*>(data);
        systemCompositorController->iviControllerHandleLayer(controller, id_layer);
    }

    void SystemCompositorController_Wayland_IVI::IVIControllerHandleSurfaceCallback(void*           data,
                                                                                    ivi_controller* controller,
                                                                                    uint32_t        id_surface)
    {
        auto* systemCompositorController =
            static_cast<SystemCompositorController_Wayland_IVI*>(data);
        systemCompositorController->iviControllerHandleSurface(controller, id_surface);
    }

    void SystemCompositorController_Wayland_IVI::IVIControllerHandleErrorCallback([[maybe_unused]] void*           data,
                                                                                  [[maybe_unused]] ivi_controller* controller,
                                                                                  [[maybe_unused]] int32_t         objectId,
                                                                                  [[maybe_unused]] int32_t         objectType,
                                                                                  [[maybe_unused]] int32_t         errorCode,
                                                                                  [[maybe_unused]] const char*     errorText)
    {
    }
}
