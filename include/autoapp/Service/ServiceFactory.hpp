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

#include <autoapp/Service/IServiceFactory.hpp>
#include <autoapp/Configuration/Configuration.hpp>
#include <autoapp/Platform/IPlatform.hpp>

namespace autoapp::service {

class ServiceFactory : public IServiceFactory {
 public:
  ServiceFactory(asio::io_service &ioService,
                 configuration::Configuration::Pointer configuration,
                  IPlatform::Pointer Platform);
  ServiceList create(aasdk::messenger::IMessenger::Pointer messenger) override;

 private:
  IService::Pointer createVideoService(const aasdk::messenger::IMessenger::Pointer& messenger);
  IService::Pointer createBluetoothService(const aasdk::messenger::IMessenger::Pointer& messenger);
  IService::Pointer createInputService(const aasdk::messenger::IMessenger::Pointer& messenger);
  void createAudioServices(ServiceList &serviceList, const aasdk::messenger::IMessenger::Pointer &messenger);

  asio::io_service &ioService_;
  configuration::Configuration::Pointer configuration_;
  IPlatform::Pointer platform;
};

}
