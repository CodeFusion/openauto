/*
*  This file is part of openauto project.
*  Copyright (C) 2021 silverchris (Chris Schimp)
*  Copyright (C) 2018 f1x.studio (Michal Szwaj)
*
*  aasdk is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 3 of the License, or
*  (at your option) any later version.

*  aasdk is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with aasdk. If not, see <http://www.gnu.org/licenses/>.
*/

#include <autoapp/Service/NavigationService.hpp>
#include <easylogging++.h>

namespace autoapp::service {
autoapp::service::NavigationService::NavigationService(asio::io_service &ioService,
                                                       aasdk::messenger::IMessenger::Pointer messenger,
                                                       INavigationManager::Pointer NavigationManager) :
    strand_(ioService),
    channel_(std::make_shared<aasdk::channel::navigation::NavigationChannel>(strand_, std::move(messenger))),
    navigationManager(std::move(NavigationManager)) {

}
void autoapp::service::NavigationService::start() {
  strand_.dispatch([this, self = this->shared_from_this()]() {
    LOG(INFO) << "[NavigationService] Start.";
    channel_->receive(this->shared_from_this());
  });
}
void NavigationService::stop() {
  LOG(INFO) << "[NavigationService] stop.";
}
void NavigationService::pause() {
  strand_.dispatch([this, self = this->shared_from_this()]() {
    LOG(INFO) << "[NavigationService] pause.";
  });
}
void NavigationService::resume() {
  strand_.dispatch([this, self = this->shared_from_this()]() {
    LOG(INFO) << "[NavigationService] resume.";
  });
}
void NavigationService::fillFeatures(aasdk::proto::messages::ServiceDiscoveryResponse &response) {
  LOG(INFO) << "[NavigationService] fill features.";

  auto *channelDescriptor = response.add_channels();
  channelDescriptor->set_channel_id(static_cast<uint32_t>(channel_->getId()));

  auto *navigationChannel = channelDescriptor->mutable_navigation_channel();
  navigationChannel->set_minimum_interval_ms(1000);
  navigationChannel->set_type(aasdk::proto::enums::NavigationType::NAVIGATION_CODES);

}

void NavigationService::onChannelOpenRequest(const aasdk::proto::messages::ChannelOpenRequest &request) {
  LOG(INFO) << "[NavigationService] open request, priority: " << request.priority();
  const aasdk::proto::enums::Status::Enum status = aasdk::proto::enums::Status::OK;
  LOG(INFO) << "[NavigationService] open status: " << status;

  aasdk::proto::messages::ChannelOpenResponse response;
  response.set_status(status);

  auto promise = aasdk::channel::SendPromise::defer(strand_);
  promise->then(std::function<void(void)>([]() {}),
                [this, self =
                this->shared_from_this()](const aasdk::error::Error &error) -> void { this->onChannelError(error); });
  channel_->sendChannelOpenResponse(response, std::move(promise));

  channel_->receive(this->shared_from_this());
}

void NavigationService::onChannelError(const aasdk::error::Error &error) {
  LOG(ERROR) << "[NavigationChannel] channel error: " << error.what();
}

void NavigationService::onNavigationStatus(const aasdk::proto::messages::NavigationRequestMessage &request) {
  LOG(DEBUG) << "onNavigationStatus " << aasdk::proto::enums::NavigationRequestState::Enum_Name(request.status());
  if (request.status() == aasdk::proto::enums::NavigationRequestState::NAVIGATION_START) {
    navigationManager->NavigationStart();
  } else {
    navigationManager->NavigationStop();
  }
  channel_->receive(this->shared_from_this());
}
void NavigationService::onNavigationTurn(const aasdk::proto::messages::NavigationTurnMessage &request) {
  LOG(DEBUG) << "onNavigationTurn " << request.event_name()
             << " " << aasdk::proto::enums::NavigationTurnSide::Enum_Name(request.turn_side())
             << " " << aasdk::proto::enums::NavigationTurnEvent::Enum_Name(request.turn_event())
             << " " << request.turn_number() << " " << request.turn_angle() << "°";
  navigationManager->NavigationTurn(request.turn_number(),
                                     request.event_name(),
                                     request.turn_side(),
                                     request.turn_event(),
                                     request.turn_angle());
  channel_->receive(this->shared_from_this());
}
void NavigationService::onNavigationDistance(const aasdk::proto::messages::NavigationDistanceMessage &request) {
  LOG(DEBUG) << "onNavigationDistance " << request.distance() << "m " << request.time_until() << "s "
             << request.display_distance() << " "
             << aasdk::proto::enums::NavigationDistanceUnit::Enum_Name(request.display_distance_unit());
  navigationManager->NavigationDistance(request.distance(),
                                         request.time_until(),
                                         request.display_distance(),
                                         request.display_distance_unit());
  channel_->receive(this->shared_from_this());
}
}
