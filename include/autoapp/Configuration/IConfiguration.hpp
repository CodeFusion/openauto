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

namespace autoapp::configuration {

class IConfiguration {
 public:
  typedef std::shared_ptr<IConfiguration> Pointer;

  virtual ~IConfiguration() = default;

  virtual void reset() = 0;

  virtual void save() = 0;

  virtual void leftHandDrive(bool value) = 0;

  [[nodiscard]] virtual bool leftHandDrive() const = 0;

  virtual void hideClock(bool value) = 0;

  [[nodiscard]] virtual bool hideClock() const = 0;

  [[nodiscard]] virtual bool getTouchscreenEnabled() const = 0;

  virtual void setTouchscreenEnabled(bool value) = 0;

};

}
