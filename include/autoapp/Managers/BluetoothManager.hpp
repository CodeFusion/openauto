#pragma once

#include <dbus-cxx.h>
#include <atomic>
#include <set>

#include <string>
#include <future>

#include <tinyxml2.h>

#include <aasdk_proto/WifiInfoRequestMessage.pb.h>
#include <aasdk_proto/WifiInfoResponseMessage.pb.h>
#include <aasdk_proto/WifiSecurityResponseMessage.pb.h>

#include <autoapp/Configuration/IConfiguration.hpp>

#include <com_jci_bca_objectProxy.h>

#include <autoapp/Managers/IBluetoothManager.hpp>

class BluetoothManager : public IBluetoothManager {
 public:
  explicit BluetoothManager(autoapp::configuration::IConfiguration::Pointer configuration,
                            std::shared_ptr<DBus::Connection> session_connection);
  ~BluetoothManager() override;

  void start() override;
  void stop() override;

 private:
  autoapp::configuration::IConfiguration::Pointer configuration_;
  std::shared_ptr<DBus::Connection> dbusConnection;
  bool bdsconfigured = false;
  uint32_t serviceId = 0;
  std::shared_ptr<com_jci_bca_objectProxy> bcaClient;
  void ConnectionStatusResp(uint32_t, uint32_t, uint32_t, uint32_t, std::tuple<std::vector<uint8_t>>) const;
};

struct connectionInfo {
  std::string ipAddress;
  std::string macAddress;
};

int update_connection_info(connectionInfo &info);

class BluetoothConnection {
 public:
  explicit BluetoothConnection(autoapp::configuration::IConfiguration::Pointer configuration);
  void handle_connect(const std::string &pty);

 private:
  int fd = 0;
  connectionInfo info;
  autoapp::configuration::IConfiguration::Pointer configuration_;

  void handleWifiInfoRequest(uint8_t *buffer, uint16_t length);
  void handleWifiSecurityRequest(__attribute__((unused)) uint8_t *buffer, __attribute__((unused)) uint16_t length);
  static int handleWifiInfoRequestResponse(uint8_t *buffer, uint16_t length);
  void sendMessage(google::protobuf::MessageLite &message, uint16_t type) const;
};