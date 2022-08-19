#pragma once

#include <autoapp/Managers/IVideoManager.hpp>
#include <autoapp/Managers/IBluetoothPairingManager.hpp>
#include <autoapp/Managers/IGPSManager.hpp>
#include <autoapp/Managers/INightManager.hpp>
#include <autoapp/Managers/IAudioManager.hpp>
#include <autoapp/Managers/IBluetoothManager.hpp>
#include <autoapp/Managers/INavigationManager.hpp>

class IPlatform {
 public:
  using Pointer = std::shared_ptr<IPlatform>;

  IVideoManager::Pointer videoManager;
  IBluetoothPairingManager::Pointer bluetoothPairingManager;

  IGPSManager::Pointer gpsManager;
  INightManager::Pointer nightManager;
  IAudioManager::Pointer audioManager;
  IBluetoothManager::Pointer bluetoothManager;
  INavigationManager::Pointer navigationManager;

  IPlatform() = default;
  virtual ~IPlatform() = default;

  /// Called when Android Auto connection starts
  virtual void start() = 0;

  /// Called when Android Auto connection stops
  virtual void stop() = 0;


};