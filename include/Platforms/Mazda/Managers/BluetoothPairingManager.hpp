#pragma once

#include "autoapp/Managers/IBluetoothPairingManager.hpp"

class BluetoothPairingManager: public IBluetoothPairingManager{
 public:
  using pairingCallback = std::function<void(std::string)>;

 private:
  std::vector<pairingCallback> pairingCallbacks;

 public:
  using Pointer = std::shared_ptr<BluetoothPairingManager>;

  BluetoothPairingManager();
  void start() override {};
  void stop() override {};

  void pairingRequest(std::string mac, aasdk::io::Promise<void>::Pointer promise) override;
  std::string getMac() override;


};