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

#include <autoapp/Configuration/Configuration.hpp>
#include <easylogging++.h>

namespace autoapp::configuration {

Configuration::Configuration() {
  wifiConfiguration = std::make_shared<WifiConfiguration>();
}

AudioConfiguration Configuration::getAudioConfig() {
  return audioConfiguration;
}

void Configuration::setAudioConfig(AudioConfiguration audioConfig) {
  audioConfiguration = audioConfig;
}

ServiceConfiguration Configuration::getServiceConfig() {
  return serviceConfiguration;
}

void Configuration::setServiceConfig(ServiceConfiguration serviceConfig) {
  serviceConfiguration = serviceConfig;
}

WifiConfiguration::pointer Configuration::getWifiConfig() {
  return wifiConfiguration;
}

void Configuration::setWifiConfig(WifiConfiguration::pointer wifiConfig) {
  wifiConfiguration = wifiConfig;
}

}
