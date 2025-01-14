//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/IThreadWatchdogNotification.h"
#include "ramses/framework/APIExport.h"
#include "ramses/framework/EFeatureLevel.h"
#include "ramses/framework/RamsesFrameworkTypes.h"
#include <memory>
#include <chrono>
#include <string_view>

namespace ramses
{
    namespace internal
    {
        class RamsesFrameworkConfigImpl;
    }

    /**
    * @brief The RamsesFrameworkConfig holds a set of parameters to be used
    * to initialize ramses.
    */
    class RAMSES_API RamsesFrameworkConfig
    {
    public:
        /**
        * @brief Constructor of RamsesFrameworkConfig
        *
        * @param[in] featureLevel feature level to use (See #ramses::EFeatureLevel for more details)
        */
        explicit RamsesFrameworkConfig(EFeatureLevel featureLevel);

        /**
        * @brief Destructor of RamsesFrameworkConfig
        */
        ~RamsesFrameworkConfig();

        /**
        * @brief Set feature level
        *
        * Sets feature level that will be used when creating #ramses::RamsesFramework.
        * #ramses::RamsesClient and #ramses::RamsesRenderer created from this framework will only be able to connect
        * to client/renderer using a compatible feature level.
        * Only files exported using the exact same feature level can be loaded into #ramses::RamsesClient created
        * from this framework.
        * See #ramses::EFeatureLevel for more details.
        *
        * @param[in] featureLevel feature level to use
        * @return true on success, false if an error occurred (error is logged)
        */
        bool setFeatureLevel(EFeatureLevel featureLevel);

        /**
        * @brief Get feature level
        *
        * Gets feature level that was set using #setFeatureLevel.
        *
        * @return currently set feature level.
        */
        [[nodiscard]] EFeatureLevel getFeatureLevel() const;

        /**
         * @brief Request a certain type of ramses shell
         * @param[in] requestedShellType type of ramses shell
         * @return true on success, false if an error occurred (error is logged)
         */
        bool setRequestedRamsesShellType(ERamsesShellType requestedShellType);

        /**
        * @brief Set watchdog notification interval of ramses threads
        *
        * @param[in] thread which thread identifier to set the interval for
        * @param[in] interval interval in ms which is used to call given callback
        * @return true on success, false if an error occurred (error is logged)
        */
        bool setWatchdogNotificationInterval(ERamsesThreadIdentifier thread, uint32_t interval);

        /**
        * @brief Set watchdog callback
        *
        * notifyThread Method will be called in the interval specified
        * registerThread and unregisterThread are called to signal which threads will be calling the callback,
        * and should be 'monitored' by user code in an appropriate way.
        *
        * @param[in] callback callback class to use for watchdog reporting
        * @return true on success, false if an error occurred (error is logged)
        */
        bool setWatchdogNotificationCallBack(IThreadWatchdogNotification* callback);

        /**
         * @brief Disable DLT application registration
         *
         * When set and DLT is enabled ramses expects DLT_REGISTER_APP being called before
         * RamsesFramework construction and DLT_UNREGISTER_APP after RamsesFramework destruction.
         * Ramses will add its context to the existing application.
         *
         * When not disabled, ramses will manage DLT application registration itself.
         *
         * @return true on success, false if an error occurred (error is logged)
         */
        bool disableDLTApplicationRegistration();

        /**
        * @brief Set the application ID name for DLT (4 chars)
        *
        * @param[in] id to use as DLT application id
        */
        void setDLTApplicationID(std::string_view id);

        /**
        * @brief Return the DLT application id value set in configuration object
        *
        * @return DLT application id value set in this configuration object
        */
        [[nodiscard]] std::string_view getDLTApplicationID() const;

        /**
        * @brief Set the application description for DLT
        *
        * @param[in] description to use as DLT application description
        */
        void setDLTApplicationDescription(std::string_view description);

        /**
        * @brief Return the DLT application description set in configuration object
        *
        * @return DLT application description set in this configuration object
        */
        [[nodiscard]] std::string_view getDLTApplicationDescription() const;

        /**
        * @brief Sets the log level for all contexts
        *
        * @param[in] logLevel the log level to be applied
        */
        void setLogLevel(ELogLevel logLevel);

        /**
        * @brief Sets the log level for the provided context
        *
        * @param[in] context the log context to modify
        * @param[in] logLevel the log level to be applied
        * @return true on success, false if an error occurred (error is logged)
        */
        bool setLogLevel(std::string_view context, ELogLevel logLevel);

        /**
        * @brief Sets the maximum log level for all contexts on console output
        *
        * Other outputs are not affected.
        *
        * @param[in] logLevel the log level to be applied
        */
        void setLogLevelConsole(ELogLevel logLevel);

        /**
        * @brief Sets the logging interval for the periodic log messages
        *
        * The Ramses framework periodically logs information about the Ramses version, connected participants, scene states etc.
        * Please leave unchanged in a production environment as the provided information is important for error analysis.
        *
        * Default value is 2 seconds. A value of 0 disables logging.
        *
        * @param[in] interval logging interval in seconds.
        */
        void setPeriodicLogInterval(std::chrono::seconds interval);

        /**
        * @brief Sets the participant identifier
        *
        * The guid identifies the ramses participant in a distributed rendering setup.
        * It is auto-generated by default.
        * Guid values < 256 are reserved and may not be used (an error will be returned)
        *
        * @param[in] guid participant identifier
        * @return true on success, false if an error occurred (error is logged)
        */
        bool setParticipantGuid(uint64_t guid);

        /**
        * @brief Sets the participant name in a distributed rendering setup
        *
        * Default value is an empty string.
        *
        * @param[in] name human readable participant name
        * @return true on success, false if an error occurred (error is logged)
        */
        bool setParticipantName(std::string_view name);

        /**
        * @brief Sets the connection system for a distributed setup
        *
        * @param[in] connectionSystem the connection system to use (default: TCP)
        * @return true on success, false if an error occurred (error is logged)
        */
        bool setConnectionSystem(EConnectionSystem connectionSystem);

        /**
        * @brief Sets the IP address that is used to select the local network interface
        *
        * @param[in] ip IP to use
        */
        void setInterfaceSelectionIPForTCPCommunication(std::string_view ip);

        /**
        * @brief Sets the port that is used to select the local network interface
        *
        * @param[in] port Port to use
        */
        void setInterfaceSelectionPortForTCPCommunication(uint16_t port);

        /**
        * @brief Sets the IP address of the communication daemon
        *
        * @param[in] ip IP to use
        */
        void setDaemonIPForTCPCommunication(std::string_view ip);

        /**
        * @brief Sets the port of the communication daemon
        *
        * @param[in] port Port to use
        */
        void setDaemonPortForTCPCommunication(uint16_t port);

        /**
        * @brief Configures the network connection monitoring
        *
        * In a distributed setup ramses participants repeatedly send keepalive messages to other participants within the given time interval.
        * If no keepalive is received for the given timeout, the remote will be considered stalled and disconnected.
        * This prevents ramses renderers from showing old content from stalled clients.
        *
        * @param interval time interval for sending keepalive messages
        * @param timeout maximum time to tolerate a missing keepalive message
        *
        * @return true on success, false if an error occurred (error is logged)
        */
        bool setConnectionKeepaliveSettings(std::chrono::milliseconds interval, std::chrono::milliseconds timeout);

        /**
         * @brief Copy constructor
         * @param other source to copy from
         */
        RamsesFrameworkConfig(const RamsesFrameworkConfig& other);

        /**
         * @brief Move constructor
         * @param other source to move from
         */
        RamsesFrameworkConfig(RamsesFrameworkConfig&& other) noexcept;

        /**
         * @brief Copy assignment
         * @param other source to copy from
         * @return this instance
         */
        RamsesFrameworkConfig& operator=(const RamsesFrameworkConfig& other);

        /**
         * @brief Move assignment
         * @param other source to move from
         * @return this instance
         */
        RamsesFrameworkConfig& operator=(RamsesFrameworkConfig&& other) noexcept;

        /**
         * Get the internal data for implementation specifics of RamsesFrameworkConfig.
         */
        [[nodiscard]] internal::RamsesFrameworkConfigImpl& impl();

        /**
         * Get the internal data for implementation specifics of RamsesFrameworkConfig.
         */
        [[nodiscard]] const internal::RamsesFrameworkConfigImpl& impl() const;

    protected:
        /**
        * Stores internal data for implementation specifics of RamsesFrameworkConfig
        */
        std::unique_ptr<internal::RamsesFrameworkConfigImpl> m_impl;
    };
}
