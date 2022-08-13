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

#include <chrono>

#include <linux/input.h>
#include <linux/uinput.h>
#include "libevdev/libevdev.h"
#include "libevdev/libevdev-uinput.h"

#include <aasdk_proto/ButtonCodeEnum.pb.h>
#include <aasdk_proto/TouchActionEnum.pb.h>
#include <aasdk/IO/Promise.hpp>
#include <autoapp/Projection/IInputDevice.hpp>
#include <autoapp/Configuration/IConfiguration.hpp>
#include <autoapp/Signals/Signals.hpp>

struct TouchScreenState {
  uint32_t x;
  uint32_t y;
  aasdk::proto::enums::TouchAction::Enum action;
  int action_recvd;
};

namespace autoapp::projection {
class InputDevice : public IInputDevice {

 public:
  InputDevice(asio::io_service &ioService, IAudioManager::Pointer AudioManager, IVideoManager::Pointer videosignals);

  void start(IInputDeviceEventHandler &eventHandler) override;

  void stop() override;

  [[nodiscard]] ButtonCodes getSupportedButtonCodes() const override;

  [[nodiscard]] bool hasTouchscreen() const override;

  [[nodiscard]] TouchscreenSize getTouchscreenGeometry() const override;

 private:
  asio::basic_waitable_timer<std::chrono::steady_clock> timer_;
  asio::io_service::strand strand_;
  IAudioManager::Pointer audioManager;
  IVideoManager::Pointer videoManger;
  IInputDeviceEventHandler *eventHandler_;
  std::mutex mutex_;
  int touch_fd = -1, kbd_fd = -1;
  aasdk::proto::enums::ButtonCode::Enum pressScanCode = aasdk::proto::enums::ButtonCode::NONE;
  time_t pressedSince = 0;
  aasdk::proto::enums::AudioFocusState_Enum audiofocus = aasdk::proto::enums::AudioFocusState_Enum_NONE;
  bool videoFocus_ = false;
  TouchScreenState mTouch{0, 0, (aasdk::proto::enums::TouchAction::Enum) 0, 0};
  libevdev *touch_dev = nullptr;
  libevdev *keyboard_dev = nullptr;
  libevdev_uinput *ui_dev = nullptr;
  std::chrono::steady_clock::time_point mediaDebounce;
  std::map<uint16_t, aasdk::proto::enums::ButtonCode_Enum> keymap;
  sigc::connection audioFocusChanged;
  sigc::connection videoFocusChanged;

  void handle_key(input_event *inputEvent);

  void handle_touch(input_event *inputEvent);

  void audio_focus(aasdk::messenger::ChannelId channel_id, aasdk::proto::enums::AudioFocusState_Enum state);
  void video_focus(bool state);
  void poll(asio::error_code errorCode);
  bool canceled_;
};
}