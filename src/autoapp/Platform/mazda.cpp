//
// Created by silverchris on 2022-08-07.
//
#ifdef BUILD_MAZDA
#include "autoapp/Platform/mazda.hpp"
#include "autoapp/Managers/VideoManager.hpp"
#include "autoapp/Managers/AAPA.hpp"
#include "autoapp/Managers/GPSManager.hpp"
#include "autoapp/Managers/HttpManager.hpp"
#include "autoapp/Managers/BluetoothManager.hpp"
#include "autoapp/Managers/IVideoManager.hpp"
#include "autoapp/Configuration/Configuration.hpp"
#include "autoapp/Managers/NightManager.hpp"

#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>

#define MINI_CASE_SENSITIVE
#include "ini.h"

bool checkAapaVersion() {
  mINI::INIFile file("/jci/version.ini");
  mINI::INIStructure ini;
  file.read(ini);
  return ini["VersionInfo"].has("JCI_BLM_AAPA-IHU");
}

Mazda::Mazda(asio::io_service &ioService, autoapp::configuration::IConfiguration::Pointer configuration) {
  /* Do some Mazda Specific Setup */
  setenv("DBUS_SYSTEM_BUS_ADDRESS", "unix:path=/tmp/dbus_service_socket", 1);
  setenv("DBUS_SESSION_BUS_ADDRESS", "unix:path=/tmp/dbus_hmi_socket", 1);

  int fd;
  fd = open("/sys/bus/usb/devices/usb1/authorized_default", O_WRONLY);
  write(fd, "1", 1);
  close(fd);
  fd = open("/sys/bus/usb/devices/usb2/authorized_default", O_WRONLY);
  write(fd, "1", 1);
  close(fd);

  //  DBus::set_logging_function(DBus::log_std_err);
//  DBus::set_log_level(SL_TRACE);
  dispatcher = DBus::StandaloneDispatcher::create();
  session_connection = dispatcher->create_connection(DBus::BusType::SESSION);
  system_connection = dispatcher->create_connection(DBus::BusType::SYSTEM);


  aaSignals = std::make_shared<AASignals>();

  if (checkAapaVersion()) {
    LOG(DEBUG) << "Using Mazda Android Auto Video";
    videoManager = std::make_shared<AAPA>(session_connection);
  } else {
    LOG(DEBUG) << "Using internal Video handling";
    videoManager = std::make_shared<VideoManager>(session_connection);
  }
  gpsManager =  std::make_shared<GPSManager>(ioService, system_connection);
  nightManager = std::make_shared<NightManager>(ioService);
  audioManager = std::make_shared<AudioManager>(system_connection);

  httpManager = new HttpManager(videoManager, aaSignals);

//  NavigationManager navigationManager(signals.navSignals, system_connection);

  bluetoothManager = std::make_shared<BluetoothManager>(configuration, session_connection);
  signals = std::make_shared<Signals>(videoManager, audioManager, gpsManager, aaSignals, nightManager, bluetoothManager);
}
#endif

