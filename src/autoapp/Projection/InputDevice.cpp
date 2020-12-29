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

#define EVENT_DEVICE_TS  "/dev/input/filtered-touchscreen0"
#define EVENT_DEVICE_KBD "/dev/input/filtered-keyboard0"

#include <easylogging++.h>
#include <autoapp/Projection/IInputDeviceEventHandler.hpp>
#include <autoapp/Projection/InputDevice.hpp>

using ButtonCode = aasdk::proto::enums::ButtonCode;

namespace autoapp::projection {

InputDevice::InputDevice(asio::io_service &ioService,
                         AudioSignals::Pointer audiosignals,
                         VideoSignals::Pointer videosignals) :
    timer_(ioService),
    strand_(ioService),
    audiosignals_(std::move(audiosignals)),
    videosignals_(std::move(videosignals)),
    eventHandler_(nullptr) {
  keymap = {
      {KEY_G, ButtonCode::MICROPHONE_1},
      {KEY_BACKSPACE, ButtonCode::BACK},
      {KEY_ENTER, ButtonCode::ENTER},
      {KEY_LEFT, ButtonCode::LEFT},
      {KEY_RIGHT, ButtonCode::RIGHT},
      {KEY_UP, ButtonCode::UP},
      {KEY_DOWN, ButtonCode::DOWN},
      {KEY_HOME, ButtonCode::HOME},
      {KEY_R, ButtonCode::NAVIGATION},
      {KEY_Z, ButtonCode::PHONE},
      {KEY_X, ButtonCode::CALL_END},
      {KEY_T, ButtonCode::TOGGLE_PLAY},
      {KEY_LEFTBRACE, ButtonCode::NEXT},
      {KEY_RIGHTBRACE, ButtonCode::PREV},
      {KEY_N, ButtonCode::SCROLL_WHEEL},
      {KEY_M, ButtonCode::SCROLL_WHEEL},
      {KEY_E, ButtonCode::MEDIA}
  };
  audioFocusChanged = audiosignals_->focusChanged.connect(sigc::mem_fun(*this, &InputDevice::audio_focus));
  videoFocusChanged = videosignals_->focusChanged.connect(sigc::mem_fun(*this, &InputDevice::video_focus));
}

void InputDevice::audio_focus(aasdk::proto::enums::AudioFocusState_Enum state) {
  audiofocus = state;
}

void InputDevice::video_focus(bool state) {
  videoFocus_ = state;
  if (videoFocus_) {
    if (keyboard_dev != nullptr) {
      libevdev_grab(keyboard_dev, LIBEVDEV_GRAB);
    }
  } else {
    if (keyboard_dev != nullptr) {
      libevdev_grab(keyboard_dev, LIBEVDEV_UNGRAB);
    }
  }
}

void InputDevice::start(IInputDeviceEventHandler &eventHandler) {
  std::lock_guard<decltype(mutex_)> lock(mutex_);
  int rc;

  LOG(INFO) << "[InputDevice] start.";
  eventHandler_ = &eventHandler;

  /* Open Touchscreen Device */
  touch_fd = open(EVENT_DEVICE_TS, O_RDONLY | O_NONBLOCK);

  if (touch_fd < 0) {
    LOG(ERROR) << EVENT_DEVICE_TS << " is not a vaild device";
  }

  rc = libevdev_new_from_fd(touch_fd, &touch_dev);
  if (rc < 0) {
    LOG(ERROR) << "Failed to init libevdev " << strerror(-rc);
  }

  if (libevdev_grab(touch_dev, LIBEVDEV_GRAB) < 0) {
    LOG(ERROR) << "Failed to grab " << EVENT_DEVICE_TS;
  }

  kbd_fd = open(EVENT_DEVICE_KBD, O_RDONLY | O_NONBLOCK);

  if (kbd_fd < 0) {
    LOG(ERROR) << EVENT_DEVICE_KBD << " is not a vaild device";
  }

  rc = libevdev_new_from_fd(kbd_fd, &keyboard_dev);
  if (rc < 0) {
    LOG(ERROR) << "Failed to init libevdev " << strerror(-rc);
  }

  if (libevdev_grab(keyboard_dev, LIBEVDEV_GRAB) < 0) {
    LOG(ERROR) << "Failed to grab " << EVENT_DEVICE_KBD;
  }

  struct libevdev *dev;
  dev = libevdev_new();
  libevdev_set_name(dev, "mzd-uinput");
  libevdev_set_id_bustype(dev, BUS_USB);
  libevdev_set_id_vendor(dev, 0x1);
  libevdev_set_id_product(dev, 0x1);
  libevdev_set_id_version(dev, 1);
  libevdev_enable_event_type(dev, EV_KEY);
  for (const auto &value: keymap) {
    libevdev_enable_event_code(dev, EV_KEY, value.first, nullptr);
  }
  rc = libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &ui_dev);

  if (rc < 0) {
    LOG(ERROR) << "Failed to init libevdev " << strerror(-rc);
  }

  timer_.expires_from_now(std::chrono::milliseconds(50));
  timer_.async_wait(strand_.wrap([this](asio::error_code ec) { this->poll(ec); }));

}

void InputDevice::poll(asio::error_code ec) {
  if (ec == asio::error::operation_aborted) {
    return;
  }
  int rc = 0;
  while (rc != -EAGAIN) {
    input_event ev{};
    rc = libevdev_next_event(touch_dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
    if (rc == LIBEVDEV_READ_STATUS_SYNC) {
      LOG(ERROR) << "Input dropped";
      while (rc == LIBEVDEV_READ_STATUS_SYNC) {
        rc = libevdev_next_event(touch_dev, LIBEVDEV_READ_FLAG_SYNC, &ev);
      }
      LOG(DEBUG) << "Input resynced";
    } else if (rc == LIBEVDEV_READ_STATUS_SUCCESS)
      handle_touch(&ev);
  }

  rc = 0;
  while (rc != -EAGAIN) {
    input_event ev{};
    rc = libevdev_next_event(keyboard_dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
    if (rc == LIBEVDEV_READ_STATUS_SYNC) {
      LOG(ERROR) << "Input dropped";
      while (rc == LIBEVDEV_READ_STATUS_SYNC) {
        rc = libevdev_next_event(keyboard_dev, LIBEVDEV_READ_FLAG_SYNC, &ev);
      }
      LOG(DEBUG) << "Input resynced";
    } else if (rc == LIBEVDEV_READ_STATUS_SUCCESS) {
      handle_key(&ev);
    }
  }

  timer_.expires_from_now(std::chrono::milliseconds(50));
  timer_.async_wait(strand_.wrap([this](asio::error_code ec) { this->poll(ec); }));
}

void InputDevice::stop() {
  std::lock_guard<decltype(mutex_)> lock(mutex_);

  audioFocusChanged.disconnect();
  videoFocusChanged.disconnect();

  timer_.cancel();

  LOG(INFO) << "[InputDevice] stop.";
  eventHandler_ = nullptr;

  libevdev_uinput_destroy(ui_dev);
  libevdev_grab(touch_dev, LIBEVDEV_UNGRAB);
  libevdev_free(touch_dev);
  libevdev_grab(keyboard_dev, LIBEVDEV_UNGRAB);
  libevdev_free(keyboard_dev);
  close(touch_fd);
  close(kbd_fd);

}

bool InputDevice::hasTouchscreen() const {
  return true;
}

IInputDevice::ButtonCodes InputDevice::getSupportedButtonCodes() const {
  std::vector<ButtonCode::Enum> buttons;
  buttons.push_back(ButtonCode::MENU);
  buttons.push_back(ButtonCode::MICROPHONE_2);
  buttons.push_back(ButtonCode::HOME);
  buttons.push_back(ButtonCode::BACK);
  buttons.push_back(ButtonCode::PHONE);
  buttons.push_back(ButtonCode::CALL_END);
  buttons.push_back(ButtonCode::UP);
  buttons.push_back(ButtonCode::DOWN);
  buttons.push_back(ButtonCode::LEFT);
  buttons.push_back(ButtonCode::RIGHT);
  buttons.push_back(ButtonCode::ENTER);
  buttons.push_back(ButtonCode::MICROPHONE_1);
  buttons.push_back(ButtonCode::TOGGLE_PLAY);
  buttons.push_back(ButtonCode::NEXT);
  buttons.push_back(ButtonCode::PREV);
  buttons.push_back(ButtonCode::MUSIC);
  buttons.push_back(ButtonCode::SCROLL_WHEEL);
  buttons.push_back(ButtonCode::TEL);
  buttons.push_back(ButtonCode::NAVIGATION);
  return buttons;
}

TouchscreenSize InputDevice::getTouchscreenGeometry() const {
  return TouchscreenSize{480, 800};
}

void InputDevice::handle_key(input_event *ev) {
  std::chrono::steady_clock::time_point tpNow = std::chrono::steady_clock::now();

  if (ev->type == EV_KEY && (ev->value == 1 || ev->value == 0)) {
    ButtonCode::Enum scanCode;
    WheelDirection direction = WheelDirection::NONE;
    ButtonEventType eventType = (ev->value == 1) ? ButtonEventType::PRESS
                                                 : ButtonEventType::RELEASE;
    bool isPressed = (ev->value == 1);
    bool hasMediaAudioFocus = audiofocus == aasdk::proto::enums::AudioFocusState_Enum_GAIN;

    scanCode = keymap[ev->code];
    if (scanCode == ButtonCode::NEXT || scanCode == ButtonCode::PREV || scanCode == ButtonCode::MEDIA) {
      if (!hasMediaAudioFocus && videoFocus_) {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(tpNow - mediaDebounce).count() > 200
            && ev->value == 1) {
          libevdev_grab(keyboard_dev, LIBEVDEV_UNGRAB);
          libevdev_uinput_write_event(ui_dev, ev->type, ev->code, 1);
          libevdev_uinput_write_event(ui_dev, EV_SYN, SYN_REPORT, 0);
          libevdev_uinput_write_event(ui_dev, ev->type, ev->code, 0);
          libevdev_uinput_write_event(ui_dev, EV_SYN, SYN_REPORT, 0);
          mediaDebounce = tpNow;
          libevdev_grab(keyboard_dev, LIBEVDEV_GRAB);
        }
        return;
      }
    }
    if (scanCode == ButtonCode::SCROLL_WHEEL) {
      if (ev->value == 0) {
        eventType = ButtonEventType::NONE;
        if (ev->code == KEY_M) {
          direction = WheelDirection::RIGHT;
        } else if (ev->code == KEY_N) {
          direction = WheelDirection::LEFT;
        }
      } else {
        scanCode = ButtonCode::NONE;
      }
    }

    if (isPressed) {
      pressScanCode = keymap[ev->code];
      time(&pressedSince);
    } else {
      time_t now = time(nullptr);
      if (now - pressedSince >= 2) {
        switch (pressScanCode) {
          case ButtonCode::TOGGLE_PLAY:audiosignals_->focusRelease.emit();
            break;
          case ButtonCode::BACK: // We use both these buttons for releasing focus, so fall through to the next case.
          case ButtonCode::CALL_END:videosignals_->focusRelease.emit(VIDEO_FOCUS_REQUESTOR::HEADUNIT);
            break;
          case ButtonCode::HOME:videosignals_->focusRequest.emit(VIDEO_FOCUS_REQUESTOR::HEADUNIT);
            break;
          default:break;
        }
      }
      pressScanCode = 0;
    }
    if (videoFocus_) {
      if (scanCode != 0) {
        eventHandler_->onButtonEvent({eventType, direction, scanCode});
      }
    }
  }
}

void InputDevice::handle_touch(input_event *ev) {
  switch (ev->type) {
    case EV_ABS:
      switch (ev->code) {
        case ABS_MT_POSITION_X:mTouch.x = static_cast<uint32_t>(ev->value * 800 / 4095);
          break;
        case ABS_MT_POSITION_Y:mTouch.y = static_cast<uint32_t>(ev->value * 480 / 4095);
          break;
        default:break;
      }
      break;
    case EV_KEY:
      if (ev->code == BTN_TOUCH) {
        mTouch.action_recvd = 1;
        if (ev->value == 1) {
          mTouch.action = aasdk::proto::enums::TouchAction::PRESS;
        } else {
          mTouch.action = aasdk::proto::enums::TouchAction::RELEASE;
        }
      }
      break;
    case EV_SYN:
      if (mTouch.action_recvd == 0) {
        mTouch.action = aasdk::proto::enums::TouchAction::DRAG;
      } else {
        mTouch.action_recvd = 0;
      }
      if (videoFocus_) {
        eventHandler_->onTouchEvent({mTouch.action, mTouch.x, mTouch.y, 0});
      }
      break;
    default:break;
  }
}

}
