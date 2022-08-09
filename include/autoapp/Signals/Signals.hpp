#pragma once

#include <memory>
#include <utility>
#include <sigc++/sigc++.h>

#include "AASignals.hpp"
#include "autoapp/Managers/IVideoManager.hpp"
#include "autoapp/Managers/IGPSManager.hpp"
#include <autoapp/Managers/INightManager.hpp>
#include <autoapp/Managers/IAudioManager.hpp>
#include <autoapp/Managers/IBluetoothManager.hpp>
#include <autoapp/Managers/INavigationManager.hpp>


class Signals : public sigc::trackable {
 public:
  using Pointer = std::shared_ptr<Signals>;

  IVideoManager::Pointer videoManager;
  IAudioManager::Pointer audioManager;
  IGPSManager::Pointer gpsManager;
  AASignals::Pointer aaSignals;
  INightManager::Pointer nightManager;
  IBluetoothManager::Pointer bluetoothManager;
  INavigationManager::Pointer navigationManager;

  explicit Signals(IVideoManager::Pointer VideoManager,
                   IAudioManager::Pointer AudioManager,
                   IGPSManager::Pointer GPSManager,
                   AASignals::Pointer AaSignals,
                   INightManager::Pointer NightManager,
                   IBluetoothManager::Pointer BluetoothManager,
                   INavigationManager::Pointer NavigationManager) :
      videoManager(std::move(VideoManager)),
      audioManager(std::move(AudioManager)),
      gpsManager(std::move(GPSManager)),
      aaSignals(std::move(AaSignals)),
      nightManager(std::move(NightManager)),
      bluetoothManager(std::move(BluetoothManager)),
      navigationManager(std::move(NavigationManager)){
  };
};