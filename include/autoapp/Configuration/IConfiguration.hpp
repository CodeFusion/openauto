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

#include <string>
#include <aasdk_proto/VideoFPSEnum.pb.h>
#include <aasdk_proto/VideoResolutionEnum.pb.h>
#include <aasdk_proto/ButtonCodeEnum.pb.h>

#include "easylogging++.h"

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

class IConfiguration {
 protected:
  AudioConfiguration audioConfiguration;

 public:
  using Pointer = std::shared_ptr<IConfiguration>;

  virtual ~IConfiguration() = default;

  virtual void reset() = 0;

  virtual void save() = 0;

  virtual void leftHandDrive(bool value) = 0;

  [[nodiscard]] virtual bool leftHandDrive() const = 0;

  virtual void hideClock(bool value) = 0;

  [[nodiscard]] virtual bool hideClock() const = 0;

  [[nodiscard]] virtual bool getTouchscreenEnabled() const = 0;

  virtual void setTouchscreenEnabled(bool value) = 0;

  [[nodiscard]] virtual uint32_t wifiPort() = 0;

  virtual void wifiPort(uint32_t port) = 0;

  [[nodiscard]] virtual el::Level logLevel() = 0;

  [[nodiscard]] virtual std::string logFile() = 0;

  [[nodiscard]] virtual std::string wifiSSID() = 0;

  [[nodiscard]] virtual std::string wifiPassword() = 0;

  virtual AudioConfiguration getAudioConfig() = 0;
  virtual void setAudioConfig(AudioConfiguration audioConfig) = 0;

};

}
