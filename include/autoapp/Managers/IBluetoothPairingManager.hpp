#pragma once

#include <vector>
#include <functional>
#include <asio.hpp>
#include <aasdk/IO/Promise.hpp>
#include <autoapp/Managers/IManager.hpp>

class IBluetoothPairingManager: public IManager{
 public:
  using pairingCallback = std::function<void(std::string)>;

 private:
  std::vector<pairingCallback> pairingCallbacks;

 public:
  using Pointer = std::shared_ptr<IBluetoothPairingManager>;

  IBluetoothPairingManager() = default;

  virtual void pairingRequest(std::string mac, aasdk::io::Promise<void>::Pointer promise) = 0;
  virtual std::string getMac() = 0;

  void registerPairing(const pairingCallback &callback){
    pairingCallbacks.emplace_back(callback);
  }

};