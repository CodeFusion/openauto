#pragma once

#include <autoapp/Managers/IManager.hpp>

class IBluetoothManager: public IManager{
 public:
  using Pointer = std::shared_ptr<IBluetoothManager>;

  IBluetoothManager() = default;

  enum wifiMessages{
    WifiInfoRequest = 1,
    WifiSecurityRequest = 2,
    WifiSecurityReponse = 3,
    WifiInfoRequestResponse = 7
  };

};