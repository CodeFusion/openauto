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

#include "autoapp/Configuration/IConfiguration.hpp"

#include <com_jci_bca_objectProxy.h>

#include "autoapp/Managers/IBluetoothManager.hpp"
#include "autoapp/Managers/BluetoothConnection.hpp"

struct connectionInfo {
  std::string ipAddress;
  std::string macAddress;
};


class BluetoothManager : public IBluetoothManager {
 public:
  explicit BluetoothManager(autoapp::configuration::IConfiguration::Pointer configuration,
                            std::shared_ptr<DBus::Connection> session_connection);
  ~BluetoothManager() override = default;

  void start() override;
  void stop() override;

 private:
  autoapp::configuration::IConfiguration::Pointer configuration_;
  std::shared_ptr<DBus::Connection> dbusConnection;
  bool bdsconfigured = false;
  uint32_t serviceId = 0;
  std::shared_ptr<com_jci_bca_objectProxy> bcaClient;
  void ConnectionStatusResp(uint32_t, uint32_t, uint32_t, uint32_t, std::tuple<std::vector<uint8_t>>);
  int update_connection_info();
  connectionInfo info;


};

class MazdaBluetoothConnection: public BluetoothConnection{
 public:
  MazdaBluetoothConnection(std::string SSID, std::string Password, std::string IpAddress, std::string MacAddress, uint32_t Port);
  void handle_connect(const std::string &pty);

 protected:
  ssize_t read(char *buf, ssize_t size) override;
  ssize_t write(char *buf, ssize_t size) override;


 private:
  int fd = 0;

};