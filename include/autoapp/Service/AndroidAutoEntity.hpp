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

#include <asio.hpp>
#include <aasdk/Transport/ITransport.hpp>
#include <aasdk/Channel/Control/IControlServiceChannel.hpp>
#include <aasdk/Channel/Control/IControlServiceChannelEventHandler.hpp>
#include <aasdk/Channel/AV/VideoServiceChannel.hpp>
#include <aasdk/IO/Promise.hpp>
#include <autoapp/Configuration/Configuration.hpp>
#include <autoapp/Service/IAndroidAutoEntity.hpp>
#include <autoapp/Service/IService.hpp>
#include <autoapp/Service/IPinger.hpp>
#include <autoapp/Platform/IPlatform.hpp>

namespace autoapp::service {

class AndroidAutoEntity
    : public IAndroidAutoEntity,
      public aasdk::channel::control::IControlServiceChannelEventHandler,
      public std::enable_shared_from_this<AndroidAutoEntity> {
 public:
  AndroidAutoEntity(asio::io_service &ioService,
                    aasdk::messenger::ICryptor::Pointer cryptor,
                    aasdk::transport::ITransport::Pointer transport,
                    aasdk::messenger::IMessenger::Pointer messenger,
                    configuration::Configuration::Pointer configuration,
                    ServiceList serviceList,
                    IPinger::Pointer pinger,
                    IPlatform::Pointer Device);
  ~AndroidAutoEntity() override;

  void start(IAndroidAutoEntityEventHandler &eventHandler) override;
  void stop() override;
  void pause() override;
  void resume() override;
  void onVersionResponse(uint16_t majorCode,
                         uint16_t minorCode,
                         aasdk::proto::enums::VersionResponseStatus::Enum status) override;
  void onHandshake(const aasdk::common::DataConstBuffer &payload) override;
  void onServiceDiscoveryRequest(const aasdk::proto::messages::ServiceDiscoveryRequest &request) override;
  void onAudioFocusRequest(const aasdk::proto::messages::AudioFocusRequest &request) override;
  void onAudioFocusResponse(aasdk::messenger::ChannelId channel_id, aasdk::proto::enums::AudioFocusState_Enum state);
  void onShutdownRequest(const aasdk::proto::messages::ShutdownRequest &request) override;
  void onShutdownResponse(const aasdk::proto::messages::ShutdownResponse &response) override;
  void sendShutdownRequest();
  void onNavigationFocusRequest(const aasdk::proto::messages::NavigationFocusRequest &request) override;
  void onPingRequest(const aasdk::proto::messages::PingRequest &request) override;
  void onPingResponse(const aasdk::proto::messages::PingResponse &response) override;
  void onChannelError(const aasdk::error::Error &error) override;
  void onVoiceSessionRequest(const aasdk::proto::messages::VoiceSessionRequest &request) override;

 private:
  using std::enable_shared_from_this<AndroidAutoEntity>::shared_from_this;
  void triggerQuit();
  void schedulePing();
  void sendPing();

  asio::io_service::strand strand_;
  aasdk::messenger::ICryptor::Pointer cryptor_;
  aasdk::transport::ITransport::Pointer transport_;
  aasdk::messenger::IMessenger::Pointer messenger_;
  aasdk::channel::control::IControlServiceChannel::Pointer controlServiceChannel_;
  configuration::Configuration::Pointer configuration_;
  ServiceList serviceList_;
  IPinger::Pointer pinger_;
  IAndroidAutoEntityEventHandler *eventHandler_;
  IPlatform::Pointer device;
};

}
