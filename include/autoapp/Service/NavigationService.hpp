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

#pragma once

#include <aasdk/Channel/Navigation/NavigationChannel.hpp>
#include <autoapp/Service/IService.hpp>
#include <autoapp/Managers/INavigationManager.hpp>

namespace autoapp::service {

class NavigationService
    : public aasdk::channel::navigation::INavigationChannelEventHandler,
      public IService,
      public std::enable_shared_from_this<NavigationService> {
 public:
  NavigationService(asio::io_service &ioService,
                    aasdk::messenger::IMessenger::Pointer messenger, INavigationManager::Pointer NavigationManager);

  void start() override;
  void stop() override;
  void pause() override;
  void resume() override;
  void fillFeatures(aasdk::proto::messages::ServiceDiscoveryResponse &response) override;
  void onChannelOpenRequest(const aasdk::proto::messages::ChannelOpenRequest &request) override;
  void onChannelError(const aasdk::error::Error &e) override;

 private:
  using std::enable_shared_from_this<NavigationService>::shared_from_this;

  void onNavigationStatus(const aasdk::proto::messages::NavigationRequestMessage &request) override;
  void onNavigationTurn(const aasdk::proto::messages::NavigationTurnMessage &request) override;
  void onNavigationDistance(const aasdk::proto::messages::NavigationDistanceMessage &request) override;

  asio::io_service::strand strand_;
  aasdk::channel::navigation::NavigationChannel::Pointer channel_;
  INavigationManager::Pointer navigationManager;
};

}