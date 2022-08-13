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

#include <memory>
#include <aasdk_proto/VideoFPSEnum.pb.h>
#include <aasdk_proto/VideoResolutionEnum.pb.h>
#include <aasdk/Common/Data.hpp>

namespace autoapp::projection {
class VideoMargins {
 private:
  uint32_t _height = 0;
  uint32_t _width = 0;
 public:
  VideoMargins(uint32_t height, uint32_t width) {
    _height = height;
    _width = width;
  }

  [[nodiscard]] uint32_t height() const {
    return _height;
  }

  [[nodiscard]] uint32_t width() const {
    return _width;
  }

  void setHeight(uint32_t height) {
    _height = height;
  }

  void setWidth(uint32_t width) {
    _width = width;
  }
};

class IVideoOutput {
 public:
  using Pointer = std::shared_ptr<IVideoOutput>;

  IVideoOutput() = default;

  virtual ~IVideoOutput() = default;

  virtual bool open() = 0;

  virtual bool init() = 0;

  virtual void write(uint64_t timestamp, const aasdk::common::DataConstBuffer &buffer) = 0;

  virtual void stop() = 0;

  [[nodiscard]] virtual aasdk::proto::enums::VideoFPS::Enum getVideoFPS() const = 0;

  [[nodiscard]] virtual aasdk::proto::enums::VideoResolution::Enum getVideoResolution() const = 0;

  [[nodiscard]] virtual size_t getScreenDPI() const = 0;

  [[nodiscard]] virtual VideoMargins getVideoMargins() const = 0;
};

}
