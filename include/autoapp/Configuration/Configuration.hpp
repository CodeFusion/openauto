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

#include <autoapp/Configuration/IConfiguration.hpp>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#include <toml.hpp>
#pragma GCC diagnostic pop

#include <fstream>

namespace autoapp::configuration {

class Configuration : public IConfiguration {
 public:
  Configuration();

  void reset() override;

  void save() override;

  void leftHandDrive(bool value) override;

  [[nodiscard]] bool leftHandDrive() const override;

  void hideClock(bool value) override;

  [[nodiscard]] bool hideClock() const override;

  [[nodiscard]] bool getTouchscreenEnabled() const override;

  void setTouchscreenEnabled(bool value) override;

  [[nodiscard]] uint32_t wifiPort() override;

  void wifiPort(uint32_t port) override;

  [[nodiscard]] el::Level logLevel() override;

  [[nodiscard]] std::string logFile() override;

  [[nodiscard]] std::string wifiSSID() override;

  [[nodiscard]] std::string wifiPassword() override;

  AudioConfiguration getAudioConfig();
  void setAudioConfig(AudioConfiguration audioConfig);

 private:

  bool lefthandDrive_ = true;
  bool hideClock_ = false;
  bool enableTouchscreen_ = true;
  int wifiPort_ = 30515;
  el::Level logLevel_ = el::Level::Debug;
  std::string logFile_;
  std::string wifiSSID_;
  std::string wifiPassword_;
};

}
