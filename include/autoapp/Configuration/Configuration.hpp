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

#include <fstream>
#include <vector>
#include <map>
#include <memory>

namespace autoapp::configuration {


struct AudioChannel{
  int rate;
  int channels;
  std::vector<std::string> outputs;
};

struct AudioConfiguration{
 public:
  std::map<std::string, AudioChannel> channels;
};

struct ServiceConfiguration {
  std::string name;
  std::string carModel;
  std::string carYear;
  std::string carSerial;
  bool leftHandDrive;
  std::string huManufacturer;
  std::string huModel;
  std::string huBuild;
  std::string huVersion;
  bool nativeMediaDuringVR;
};

struct WifiConfiguration {
  using pointer = std::shared_ptr<WifiConfiguration>;
  bool enabled;
  std::string device;
  std::string ssid;
  std::string bssid;
  std::string password;
  std::string ipAddress;
  uint32_t port = 30515;
};

class Configuration {
 private:
  AudioConfiguration audioConfiguration;
  ServiceConfiguration serviceConfiguration;
  WifiConfiguration::pointer wifiConfiguration;

 public:
  using Pointer = std::shared_ptr<Configuration>;

  Configuration();

  AudioConfiguration getAudioConfig();
  void setAudioConfig(AudioConfiguration audioConfig);

  ServiceConfiguration getServiceConfig();
  void setServiceConfig(ServiceConfiguration serviceConfig);

  WifiConfiguration::pointer getWifiConfig();
  void setWifiConfig(WifiConfiguration::pointer wifiConfig);
};

}
