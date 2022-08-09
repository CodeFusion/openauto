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


#include <aasdk/USB/USBHub.hpp>
#include <aasdk/USB/ConnectedAccessoriesEnumerator.hpp>
#include <aasdk/USB/AccessoryModeQueryChainFactory.hpp>
#include <aasdk/USB/AccessoryModeQueryFactory.hpp>
#include <aasdk/TCP/TCPWrapper.hpp>
#include <autoapp/App.hpp>
#include <autoapp/Service/AndroidAutoEntityFactory.hpp>
#include <autoapp/Service/ServiceFactory.hpp>
#include <easylogging++.h>
#include <autoapp/Configuration/Configuration.hpp>
#include "Platforms/Mazda/mazda.hpp"
#include <Platforms/RPI/RPI.hpp>
#include "autoapp/Platform/IPlatform.hpp"

using ThreadPool = std::vector<std::thread>;

class usbThreadPool {
 public:
  explicit usbThreadPool(libusb_context *usbContext): usbContext_(usbContext) {
    for (unsigned i = 0; i < 4; ++i) {
      threads.emplace_back(&usbThreadPool::USBWorkers, this);
    }
  }
  ~usbThreadPool() {
    LOG(DEBUG) << "Stopping USB Threads";
    done = true;
    for (auto &thread : threads) {
      if (thread.joinable()) {
        thread.join();
      }
    }
    libusb_exit(usbContext_);
  }

 private:
  libusb_context *usbContext_{};
  std::vector<std::thread> threads;
  bool done{};

  void USBWorkers() {
    timeval libusbEventTimeout{10, 0};

    while (!done) {
      libusb_handle_events_timeout_completed(usbContext_, &libusbEventTimeout, nullptr);
    }
  }

};

void startIOServiceWorkers(asio::io_service &ioService, ThreadPool &threadPool) {
  auto ioServiceWorker = [&ioService]() {
    ioService.run();
  };

  threadPool.emplace_back(ioServiceWorker);
  threadPool.emplace_back(ioServiceWorker);
  threadPool.emplace_back(ioServiceWorker);
  threadPool.emplace_back(ioServiceWorker);
}

INITIALIZE_EASYLOGGINGPP

bool running = true;
bool connected = false;

void signalHandler(int signum) {
  if (signum == SIGINT) {
    running = false;
    LOG(INFO) << "Received SIGINT";
  } else if (signum == SIGTERM) {
    running = false;
    LOG(INFO) << "Received SIGTERM";
  }
}

int main(int argc, char *argv[]) {
  auto start_time = std::chrono::high_resolution_clock::now();


  /*                              */

  auto configuration = std::make_shared<autoapp::configuration::Configuration>();

  el::Configurations defaultConf;
  defaultConf.setToDefault();
  el::Loggers::addFlag(el::LoggingFlag::HierarchicalLogging);
  if (!configuration->logFile().empty()) {
    defaultConf.set(el::Level::Global, el::ConfigurationType::Filename, configuration->logFile());
    defaultConf.set(el::Level::Global, el::ConfigurationType::ToFile, "true");
    defaultConf.set(el::Level::Global, el::ConfigurationType::ToStandardOutput, "false");
  }
  el::Helpers::installCustomFormatSpecifier(el::CustomFormatSpecifier("%ts",
                                                                      [start_time](const el::LogMessage *) {
                                                                        auto current_time =
                                                                            std::chrono::high_resolution_clock::now();
                                                                        char buf[50];
                                                                        float_t ts =
                                                                            std::chrono::duration_cast<std::chrono::milliseconds>(
                                                                                current_time - start_time).count()
                                                                                / 1000.00;
                                                                        snprintf(buf, sizeof(buf), "%.3f", ts);
                                                                        std::string out = buf;
                                                                        return out;
                                                                      }));
  // Values are always std::string
  defaultConf.set(el::Level::Info,
                  el::ConfigurationType::Format, "%ts %levshort [%fbase] %msg");
  defaultConf.set(el::Level::Debug,
                  el::ConfigurationType::Format, "%ts %levshort [%fbase] [%func] %msg");
  defaultConf.set(el::Level::Error,
                  el::ConfigurationType::Format, "%ts %levshort [%fbase] [%func] %msg");
  // default logger uses default configurations
  el::Loggers::reconfigureLogger("default", defaultConf);
  el::Loggers::setLoggingLevel(configuration->logLevel());

  LOG(INFO) << "[OpenAuto] starting";
  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);
  signal(SIGPIPE, SIG_IGN);
  libusb_context *usbContext;
  if (libusb_init(&usbContext) != 0) {
    LOG(ERROR) << "[OpenAuto] libusb init failed.";
    return 1;
  }


  asio::io_service ioService;
  asio::io_service::work work(ioService);
  std::vector<std::thread> threadPool;
  usbThreadPool usb_thread_pool(usbContext);
  startIOServiceWorkers(ioService, threadPool);

  IPlatform::Pointer device;

#ifdef BUILD_MAZDA
  device = std::make_shared<Mazda>(ioService, configuration);
#endif
#ifdef BUILD_RPI
  device = std::make_shared<RPI>();
#endif

  aasdk::tcp::TCPWrapper tcpWrapper;

  aasdk::usb::USBWrapper usbWrapper(usbContext);
  aasdk::usb::AccessoryModeQueryFactory queryFactory(usbWrapper, ioService);
  aasdk::usb::AccessoryModeQueryChainFactory queryChainFactory(usbWrapper, ioService, queryFactory);
  autoapp::service::ServiceFactory serviceFactory(ioService, configuration, device->signals);
  autoapp::service::AndroidAutoEntityFactory
      androidAutoEntityFactory(ioService, configuration, serviceFactory, device->signals);
  auto usbHub(std::make_shared<aasdk::usb::USBHub>(usbWrapper, ioService, queryChainFactory));
  auto connectedAccessoriesEnumerator
      (std::make_shared<aasdk::usb::ConnectedAccessoriesEnumerator>(usbWrapper, ioService, queryChainFactory));
  auto app = std::make_shared<autoapp::App>(ioService,
                                            usbWrapper,
                                            tcpWrapper,
                                            androidAutoEntityFactory,
                                            std::move(usbHub),
                                            std::move(connectedAccessoriesEnumerator),
                                            configuration->wifiPort());

  app->waitForUSBDevice();

  device->signals->bluetoothManager->start();

  while (running) {
    sleep(1);
  }

  device->signals->bluetoothManager->stop();

  sleep(2);

  LOG(DEBUG) << "Calling app->stop()";
  app->stop();
  LOG(DEBUG) << "Stopping ioService";
  ioService.stop();
  LOG(DEBUG) << "Joining threads";
  std::for_each(threadPool.begin(), threadPool.end(), [](std::thread &thread) { thread.join(); });
  return 0;
}