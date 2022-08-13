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

#include <aasdk/IO/Promise.hpp>
#include <autoapp/Projection/InputEvent.hpp>

namespace autoapp::projection {

struct TouchscreenSize {
  uint32_t width;
  uint32_t height;
};

class IInputDeviceEventHandler;

class IInputDevice {
 public:
  using Pointer = std::shared_ptr<IInputDevice>;
  using ButtonCodes = std::vector<aasdk::proto::enums::ButtonCode::Enum>;

  virtual ~IInputDevice() = default;
  virtual void start(IInputDeviceEventHandler &eventHandler) = 0;
  virtual void stop() = 0;
  [[nodiscard]] virtual ButtonCodes getSupportedButtonCodes() const = 0;
  [[nodiscard]] virtual bool hasTouchscreen() const = 0;
  [[nodiscard]] virtual TouchscreenSize getTouchscreenGeometry() const = 0;
};

}
