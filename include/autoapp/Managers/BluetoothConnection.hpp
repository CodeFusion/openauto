#pragma once
#include <google/protobuf/message_lite.h>
#include "IBluetoothManager.hpp"
#include "autoapp/Configuration/Configuration.hpp"

class BluetoothConnection {
 public:
  explicit BluetoothConnection(autoapp::configuration::WifiConfiguration::pointer WifiConfig);
  void messageHandler();
  void sendMessage(google::protobuf::MessageLite &message, IBluetoothManager::wifiMessages type);
  void handleWifiInfoRequest();
  void handleWifiSecurityRequest(__attribute__((unused)) uint8_t *buffer, __attribute__((unused)) uint16_t length);
  static int handleWifiInfoRequestResponse(uint8_t *buffer, uint16_t length);

 protected:
  autoapp::configuration::WifiConfiguration::pointer wifiConfig;

  virtual ssize_t read(char *buf, ssize_t size) = 0;
  virtual ssize_t write(char *buf, ssize_t size) = 0;

};