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

#include <thread>
#include <aasdk/USB/AOAPDevice.hpp>
#include <aasdk/TCP/TCPEndpoint.hpp>
#include <autoapp/App.hpp>
#include <easylogging++.h>

namespace autoapp {

App::App(asio::io_service &ioService,
         aasdk::usb::USBWrapper &usbWrapper,
         aasdk::tcp::ITCPWrapper &tcpWrapper,
         service::IAndroidAutoEntityFactory &androidAutoEntityFactory,
         aasdk::usb::IUSBHub::Pointer usbHub,
         aasdk::usb::IConnectedAccessoriesEnumerator::Pointer connectedAccessoriesEnumerator,
         uint32_t port)
    : ioService_(ioService),
      usbWrapper_(usbWrapper),
      tcpWrapper_(tcpWrapper),
      acceptor_(ioService, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
      strand_(ioService_),
      androidAutoEntityFactory_(androidAutoEntityFactory),
      usbHub_(std::move(usbHub)),
      connectedAccessoriesEnumerator_(std::move(connectedAccessoriesEnumerator)),
      port_(port),
      isStopped_(false) {

}

void App::waitForUSBDevice() {
  strand_.dispatch([this, self = this->shared_from_this()]() {
    try {
      this->waitForDevice();
    }
    catch (...) {
      LOG(ERROR) << "[App] waitForUSBDevice() -exception caused by this->waitForDevice();";
    }
    try {
      this->enumerateDevices();
    }
    catch (...) {
      LOG(ERROR) << "[App] waitForUSBDevice() exception caused by this->enumerateDevices()";
    }

  });
}

void App::start(aasdk::tcp::ITCPEndpoint::SocketPointer socket) {
  strand_.dispatch([this, self = this->shared_from_this(), socket = std::move(socket)]() mutable {
    LOG(INFO) << "Start from socket";
    if (androidAutoEntity_ != nullptr) {
//            tcpWrapper_.close(*socket);
//            LOG(WARNING) << "[App] android auto entity is still running.";
//            return;
      try {
        androidAutoEntity_->stop();
      } catch (...) {
        LOG(ERROR) << "[App] onAndroidAutoQuit: exception caused by androidAutoEntity_->stop();";
      }
      try {
        androidAutoEntity_.reset();
      } catch (...) {
        LOG(ERROR) << "[App] onAndroidAutoQuit: exception caused by androidAutoEntity_.reset();";
      }
    }

    try {
//            usbHub_->cancel();
//            connectedAccessoriesEnumerator_->cancel();

      auto tcpEndpoint(std::make_shared<aasdk::tcp::TCPEndpoint>(tcpWrapper_, std::move(socket)));
      androidAutoEntity_ = androidAutoEntityFactory_.create(std::move(tcpEndpoint));
      androidAutoEntity_->start(*this);
    }
    catch (const aasdk::error::Error &error) {
      LOG(ERROR) << "[App] TCP AndroidAutoEntity create error: " << error.what();

      //androidAutoEntity_.reset();
      this->waitForDevice();
    }
  });
}

void App::stop() {
  strand_.dispatch([this, self = this->shared_from_this()]() {
    isStopped_ = true;
    try {
      connectedAccessoriesEnumerator_->cancel();
    } catch (...) {
      LOG(ERROR) << "[App] stop: exception caused by connectedAccessoriesEnumerator_->cancel()";
    }
    try {
      usbHub_->cancel();
    } catch (...) {
      LOG(ERROR) << "[App] stop: exception caused by usbHub_->cancel();";
    }

    if (androidAutoEntity_ != nullptr) {
      try {
        androidAutoEntity_->stop();
      } catch (...) {
        LOG(ERROR) << "[App] stop: exception caused by androidAutoEntity_->stop();";
      }
      try {
        androidAutoEntity_.reset();
      } catch (...) {
        LOG(ERROR) << "[App] stop: exception caused by androidAutoEntity_.reset();";
      }
    }
  });

}

void App::aoapDeviceHandler(aasdk::usb::DeviceHandle deviceHandle) {
  LOG(INFO) << "[App] Device connected.";

  if (androidAutoEntity_ != nullptr) {
    LOG(WARNING) << "[App] android auto entity is still running.";
    return;
  }

  try {
    // ignore autostart if exit to csng was used
    if (!disableAutostartEntity) {
      LOG(INFO) << "[App] Start Android Auto allowed - let's go.";
      connectedAccessoriesEnumerator_->cancel();

      auto aoapDevice(aasdk::usb::AOAPDevice::create(usbWrapper_, ioService_, std::move(deviceHandle)));
      androidAutoEntity_ = androidAutoEntityFactory_.create(std::move(aoapDevice));
      androidAutoEntity_->start(*this);
    } else {
      LOG(INFO) << "[App] Start Android Auto not allowed - skip.";
    }
  }
  catch (const aasdk::error::Error &error) {
    LOG(ERROR) << "[App] USB AndroidAutoEntity create error: " << error.what();

    androidAutoEntity_.reset();
    this->waitForDevice();
  }
}

void App::enumerateDevices() {
  auto promise = aasdk::usb::IConnectedAccessoriesEnumerator::Promise::defer(strand_);
  promise->then([](auto result) {
                  LOG(INFO) << "[App] Devices enumeration result: " << result;
                },
                [](auto error) {
                  LOG(ERROR) << "[App] Devices enumeration failed: " << error.what();
                });

  connectedAccessoriesEnumerator_->enumerate(std::move(promise));
}

void App::waitForDevice() {
  LOG(INFO) << "[App] Waiting for device...";

  auto promise = aasdk::usb::IUSBHub::Promise::defer(strand_);
  promise->then([&](aasdk::usb::DeviceHandle deviceHandle) { aoapDeviceHandler(std::move(deviceHandle)); },
                [&](const aasdk::error::Error &error) { onUSBHubError(error); });
  usbHub_->start(std::move(promise));
  startServerSocket();
}

void App::startServerSocket() {
  strand_.dispatch([this, self = this->shared_from_this()]() {
    LOG(INFO) << "Listening for WIFI clients on port " << port_;
    auto socket = std::make_shared<asio::ip::tcp::socket>(ioService_);
    acceptor_.async_accept(
        *socket,
        std::bind(&App::handleNewClient, this, socket, std::placeholders::_1)
    );
  });
}

void App::handleNewClient(std::shared_ptr<asio::ip::tcp::socket> socket, const asio::error_code &err) {
  LOG(INFO) << "WIFI Client connected";
  if (!err) {
    start(std::move(socket));
  }
}

void App::pause() {
  strand_.dispatch([this, self = this->shared_from_this()]() {
    LOG(INFO) << "[App] pause...";
    androidAutoEntity_->pause();
  });
}

void App::resume() {
  strand_.dispatch([this, self = this->shared_from_this()]() {
    if (androidAutoEntity_ != nullptr) {
      LOG(INFO) << "[App] resume...";
      androidAutoEntity_->resume();
    } else {
      LOG(INFO) << "[App] Ignore resume -> no androidAutoEntity_ ...";
    }
  });
}

void App::onAndroidAutoQuit() {
  strand_.dispatch([this, self = this->shared_from_this()]() {
    LOG(INFO) << "[App] onAndroidAutoQuit.";

    //acceptor_.close();

    if (androidAutoEntity_ != nullptr) {
      try {
        androidAutoEntity_->stop();
      } catch (...) {
        LOG(ERROR) << "[App] onAndroidAutoQuit: exception caused by androidAutoEntity_->stop();";
      }
      try {
        androidAutoEntity_.reset();
      } catch (...) {
        LOG(ERROR) << "[App] onAndroidAutoQuit: exception caused by androidAutoEntity_.reset();";
      }
    }

    if (!isStopped_) {
      try {
        this->waitForDevice();
      } catch (...) {
        LOG(ERROR) << "[App] onAndroidAutoQuit: exception caused by this->waitForDevice();";
      }
    }
  });
}

void App::onUSBHubError(const aasdk::error::Error &error) {
  LOG(ERROR) << "[App] usb hub error: " << error.what();

//    if(error != aasdk::error::ErrorCode::OPERATION_ABORTED &&
//       error != aasdk::error::ErrorCode::OPERATION_IN_PROGRESS)
//    {
//        try {
//            this->waitForDevice();
//        } catch (...) {
//            LOG(ERROR) << "[App] onUSBHubError: exception caused by this->waitForDevice();";
//        }
//    }
}

}
