#pragma once
#ifdef BUILD_MAZDA
#include "autoapp/Platform/IPlatform.hpp"
#include <dbus-cxx.h>

#include "Platforms/Mazda/Managers/VideoManager.hpp"
#include "Platforms/Mazda/Managers/AudioManager.hpp"
#include "Platforms/Mazda/Managers/AAPA.hpp"
#include "Platforms/Mazda/Managers/GPSManager.hpp"
#include "Platforms/Mazda/Managers/HttpManager.hpp"
#include "Platforms/Mazda/Managers/BluetoothManager.hpp"
#include "autoapp/Managers/IVideoManager.hpp"
#include "autoapp/Configuration/Configuration.hpp"
#include "Platforms/Mazda/Managers/NightManager.hpp"
#include "Platforms/Mazda/Managers/NavigationManager.hpp"


#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>


class Mazda: public IPlatform{
 public:
  Mazda(asio::io_service &ioService, autoapp::configuration::IConfiguration::Pointer configuration);

  void start() override;
  void stop() override;

  std::shared_ptr<DBus::Dispatcher> dispatcher;
  std::shared_ptr<DBus::Connection> session_connection;
  std::shared_ptr<DBus::Connection> system_connection;

  IVideoManager::Pointer videoManager;
  AASignals::Pointer aaSignals;

  IGPSManager::Pointer gpsManager;
  INightManager::Pointer nightManager;
  IAudioManager::Pointer audioManager;
  IBluetoothManager::Pointer bluetoothManager;
  INavigationManager::Pointer navigationManager;
  HttpManager *httpManager;

};

#endif