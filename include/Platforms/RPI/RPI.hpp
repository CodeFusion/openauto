#pragma once
#ifdef BUILD_RPI
#include "autoapp/Platform/IPlatform.hpp"

#include <Platforms/RPI/Managers/VideoManager.hpp>
#include <Platforms/RPI/Managers/AudioManager.hpp>
#include <Platforms/RPI/Managers/GPSManager.hpp>
#include <Platforms/RPI/Managers/NightManager.hpp>
#include <Platforms/RPI/Managers/BluetoothManager.hpp>


#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>


class RPI: public IPlatform{
 public:
  RPI();

  IVideoManager::Pointer videoManager;
  AASignals::Pointer aaSignals;

  IGPSManager::Pointer gpsManager;
  INightManager::Pointer nightManager;
  IAudioManager::Pointer audioManager;
  IBluetoothManager::Pointer bluetoothManager;

};

#endif