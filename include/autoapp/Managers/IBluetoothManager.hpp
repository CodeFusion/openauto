#pragma once

#include <autoapp/Managers/IManager.hpp>

class IBluetoothManager: public IManager{
 public:
  using Pointer = std::shared_ptr<IBluetoothManager>;

 public:
  IBluetoothManager() = default;
  virtual ~IBluetoothManager() = default;

  virtual void start() = 0;
  virtual void stop() = 0;

};