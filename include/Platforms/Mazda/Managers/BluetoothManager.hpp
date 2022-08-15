#pragma once

#include <dbus-cxx.h>
#include <atomic>
#include <set>

#include <string>
#include <future>

#include <tinyxml2.h>

#include "external/aasdk/aasdk_proto/WifiInfoRequestMessage.pb.h"
#include "external/aasdk/aasdk_proto/WifiInfoResponseMessage.pb.h"
#include "external/aasdk/aasdk_proto/WifiSecurityResponseMessage.pb.h"

#include "autoapp/Configuration/Configuration.hpp"

#include <com_jci_bca_objectProxy.h>
#include <asio/basic_waitable_timer.hpp>
#include <asio/io_service.hpp>

#include "autoapp/Managers/IBluetoothManager.hpp"
#include "autoapp/Managers/BluetoothConnection.hpp"

struct connectionInfo {
  std::string ipAddress;
  std::string macAddress;
};


class BluetoothManager : public IBluetoothManager {
 public:
  using Pointer = std::shared_ptr<BluetoothManager>;

  explicit BluetoothManager(autoapp::configuration::Configuration::Pointer configuration,
                            std::shared_ptr<DBus::Connection> session_connection, asio::io_service &ioService);
  ~BluetoothManager() override = default;

  void start() override;
  void stop() override;

  void aaConnect(bool connected);

 private:
  autoapp::configuration::Configuration::Pointer configuration_;
  std::shared_ptr<DBus::Connection> dbusConnection;
  bool bdsconfigured = false;
  uint32_t serviceId = 0;
  std::shared_ptr<com_jci_bca_objectProxy> bcaClient;
  void ConnectionStatusResp(uint32_t, uint32_t, uint32_t, uint32_t, std::tuple<std::vector<uint8_t>>);
  int update_connection_info();
  connectionInfo info;
  asio::basic_waitable_timer<std::chrono::steady_clock> timer;
  void retryTimer(const asio::error_code &error);


};

class MazdaBluetoothConnection: public BluetoothConnection{
 public:
  MazdaBluetoothConnection(autoapp::configuration::WifiConfiguration::pointer WifiConfig);
  void handle_connect(const std::string &pty);

 protected:
  ssize_t read(char *buf, ssize_t size) override;
  ssize_t write(char *buf, ssize_t size) override;


 private:
  int fd = 0;

};