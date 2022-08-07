#pragma once
#include "autoapp/Managers/IBluetoothManager.hpp"

class BluetoothManager : public IBluetoothManager {
 public:
  explicit BluetoothManager();
  ~BluetoothManager() override;

  void start() override;
  void stop() override;
};