#pragma once
#ifdef BUILD_MAZDA
#include "autoapp/Platform/IPlatform.hpp"
#include <dbus-cxx.h>
#include "autoapp/Managers/VideoManager.hpp"
#include "autoapp/Managers/AAPA.hpp"
#include "autoapp/Managers/AudioManager.hpp"
#include "autoapp/Managers/GPSManager.hpp"
#include "autoapp/Managers/HttpManager.hpp"
#include "autoapp/Managers/BluetoothManager.hpp"
#include "autoapp/Managers/NavigationManager.hpp"
#include "autoapp/Managers/IVideoManager.hpp"
#include "autoapp/Configuration/Configuration.hpp"
#include "autoapp/Managers/NightManager.hpp"

#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>


class Mazda: public IPlatform{
 public:
  Mazda(asio::io_service &ioService, autoapp::configuration::IConfiguration::Pointer configuration);

  std::shared_ptr<DBus::Dispatcher> dispatcher;
  std::shared_ptr<DBus::Connection> session_connection;
  std::shared_ptr<DBus::Connection> system_connection;

  IVideoManager::Pointer videoManager;
  AASignals::Pointer aaSignals;

  IGPSManager::Pointer gpsManager;
  INightManager::Pointer nightManager;
  IAudioManager::Pointer audioManager;
  IBluetoothManager::Pointer bluetoothManager;
  HttpManager *httpManager;

};

#endif