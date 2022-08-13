/*
*  This file is part of openauto project.
*  Copyright (C) 2018 f1x.studio (Michal Szwaj)
*
*  openauto is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 3 of the License, or
*  (at your option) any later version.

*  openauto is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with openauto. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

//#include <gps.h>
#include <aasdk/Channel/Sensor/SensorServiceChannel.hpp>
#include <autoapp/Service/IService.hpp>
#include <autoapp/Managers/IGPSManager.hpp>
#include <autoapp/Managers/INightManager.hpp>

namespace autoapp::service {

class SensorService
    : public aasdk::channel::sensor::ISensorServiceChannelEventHandler,
      public IService,
      public std::enable_shared_from_this<SensorService> {
 public:
  SensorService(asio::io_service &ioService,
                aasdk::messenger::IMessenger::Pointer messenger,
                IGPSManager::Pointer gpsmanager, INightManager::Pointer NightManager);

  void start() override;
  void stop() override;
  void pause() override;
  void resume() override;
  void fillFeatures(aasdk::proto::messages::ServiceDiscoveryResponse &response) override;
  void onChannelOpenRequest(const aasdk::proto::messages::ChannelOpenRequest &error) override;
  void onSensorStartRequest(const aasdk::proto::messages::SensorStartRequestMessage &error) override;
  void onChannelError(const aasdk::error::Error &error) override;

 private:
  using std::enable_shared_from_this<SensorService>::shared_from_this;
  void sendDrivingStatusUnrestricted();
  void sendNightData(bool isNight);
  void sendGPSLocationData(const aasdk::proto::data::GPSLocation& error);

  asio::basic_waitable_timer<std::chrono::steady_clock> timer_;
  asio::io_service::strand strand_;
  aasdk::channel::sensor::SensorServiceChannel::Pointer channel_;
  IGPSManager::Pointer gpsManager;
  INightManager::Pointer nightManger;
};

}

bool checkNight();
