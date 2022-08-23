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

#include <aasdk/Channel/AV/IAudioServiceChannel.hpp>
#include <aasdk/Channel/AV/IAudioServiceChannelEventHandler.hpp>
#include <autoapp/Projection/IAudioOutput.hpp>
#include <autoapp/Service/IService.hpp>
#include <autoapp/Managers/IAudioManager.hpp>
#include "aasdk/Messenger/IMessenger.hpp"

namespace autoapp::service {

class AudioService
    : public aasdk::channel::av::IAudioServiceChannelEventHandler,
      public IService,
      public std::enable_shared_from_this<AudioService> {
 public:
  using Pointer = std::shared_ptr<AudioService>;

  AudioService(asio::io_service &ioService,
               aasdk::messenger::IMessenger::Pointer messenger,
               aasdk::messenger::ChannelId channelID,
               std::vector<projection::IAudioOutput::Pointer> audioOutput,
               IAudioManager::Pointer AudioManager);

  ~AudioService() noexcept override;

  void start() override;
  void stop() override;
  void pause() override;
  void resume() override;
  void fillFeatures(aasdk::proto::messages::ServiceDiscoveryResponse &response) override;
  void onChannelOpenRequest(const aasdk::proto::messages::ChannelOpenRequest &error) override;
  void onAVChannelSetupRequest(const aasdk::proto::messages::AVChannelSetupRequest &error) override;
  void onAVChannelStartIndication(const aasdk::proto::messages::AVChannelStartIndication &indication) override;
  void onAVChannelStopIndication(const aasdk::proto::messages::AVChannelStopIndication &indication) override;
  void onAVMediaWithTimestampIndication(aasdk::messenger::Timestamp::ValueType error,
                                        const aasdk::common::DataConstBuffer &buffer) override;
  void onAVMediaIndication(const aasdk::common::DataConstBuffer &buffer) override;
  void onChannelError(const aasdk::error::Error &error) override;

 protected:
  using std::enable_shared_from_this<AudioService>::shared_from_this;

 private:
  asio::io_service::strand strand_;
  asio::io_service::strand WriterStrand;
  aasdk::channel::av::IAudioServiceChannel::Pointer channel_;
  std::vector<projection::IAudioOutput::Pointer> audioOutput_;
  int32_t session_;
  IAudioManager::Pointer audioManager;
};

}
