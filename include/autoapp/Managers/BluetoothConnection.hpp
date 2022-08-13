#pragma once
#include <google/protobuf/message_lite.h>
#include "IBluetoothManager.hpp"
#include "autoapp/Configuration/IConfiguration.hpp"

class BluetoothConnection {
 public:
  BluetoothConnection(std::string SSID, std::string Password, std::string IpAddress, std::string MacAddress, uint32_t Port);
  void messageHandler();
  void sendMessage(google::protobuf::MessageLite &message, IBluetoothManager::wifiMessages type);
  void handleWifiInfoRequest();
  void handleWifiSecurityRequest(__attribute__((unused)) uint8_t *buffer, __attribute__((unused)) uint16_t length);
  static int handleWifiInfoRequestResponse(uint8_t *buffer, uint16_t length);

 protected:
  std::string ipAddress;
  std::string macAddress;
  std::string ssid;
  std::string password;
  uint32_t port;

  virtual ssize_t read(char *buf, ssize_t size) = 0;
  virtual ssize_t write(char *buf, ssize_t size) = 0;

};