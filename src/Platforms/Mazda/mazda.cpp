//
// Created by silverchris on 2022-08-07.
//
#ifdef BUILD_MAZDA
#include "Platforms/Mazda/mazda.hpp"
#include <version.h>

#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>

#define MINI_CASE_SENSITIVE
#include "ini.h"

#include <easylogging++.h>

bool checkAapaVersion() {
  mINI::INIFile file("/jci/version.ini");
  mINI::INIStructure ini;
  file.read(ini);
  return ini["VersionInfo"].has("JCI_BLM_AAPA-IHU");
}

Mazda::Mazda(asio::io_service &ioService, const autoapp::configuration::Configuration::Pointer& configuration) {
  /* Do some Mazda Specific Setup */
  setenv("DBUS_SYSTEM_BUS_ADDRESS", "unix:path=/tmp/dbus_service_socket", 1);
  setenv("DBUS_SESSION_BUS_ADDRESS", "unix:path=/tmp/dbus_hmi_socket", 1);

  int fileDescriptor;
  fileDescriptor = open("/sys/bus/usb/devices/usb1/authorized_default", O_WRONLY);
  write(fileDescriptor, "1", 1);
  close(fileDescriptor);
  fileDescriptor = open("/sys/bus/usb/devices/usb2/authorized_default", O_WRONLY);
  write(fileDescriptor, "1", 1);
  close(fileDescriptor);

  //  DBus::set_logging_function(DBus::log_std_err);
//  DBus::set_log_level(SL_TRACE);
  dispatcher = DBus::StandaloneDispatcher::create();
  session_connection = dispatcher->create_connection(DBus::BusType::SESSION);
  system_connection = dispatcher->create_connection(DBus::BusType::SYSTEM);

  autoapp::configuration::AudioConfiguration audioConfig;
  // Setup things that differ between CMU versions that have built in Android Auto, and earlier versions that do not.
  if (checkAapaVersion()) {
    LOG(DEBUG) << "Using Mazda Android Auto Video";
    AAPA::Pointer aapa = std::make_shared<AAPA>(session_connection);
    videoManager = aapa;
    bluetoothPairingManager = aapa;
    LOG(DEBUG) << "Using Mazda AA Audio";
    autoapp::configuration::AudioChannel mediaChannel;
    mediaChannel.channels = 2;
    mediaChannel.rate = 48000;
    mediaChannel.outputs.emplace_back("androidautoMainAudio");
    autoapp::configuration::AudioChannel speechChannel;
    speechChannel.channels = 1;
    speechChannel.rate = 16000;
    speechChannel.outputs.emplace_back("androidautoAltAudio");
    speechChannel.outputs.emplace_back("androidautoMainAudio");
    audioConfig.channels.emplace("Media", std::move(mediaChannel));
    audioConfig.channels.emplace("Speech", std::move(speechChannel));
  } else {
    LOG(DEBUG) << "Using internal Video handling";
    videoManager = std::make_shared<VideoManager>(session_connection);
    bluetoothPairingManager = std::make_shared<BluetoothPairingManager>();
    autoapp::configuration::AudioChannel mediaChannel;
    mediaChannel.channels = 2;
    mediaChannel.rate = 48000;
    mediaChannel.outputs.emplace_back("entertainmentMl");
    autoapp::configuration::AudioChannel speechChannel;
    speechChannel.channels = 1;
    speechChannel.rate = 16000;
    speechChannel.outputs.emplace_back("informationNavi");
    speechChannel.outputs.emplace_back("entertainmentMl");
    audioConfig.channels.emplace("Media", std::move(mediaChannel));
    audioConfig.channels.emplace("Speech", std::move(speechChannel));
  }
  configuration->setAudioConfig(std::move(audioConfig));
  autoapp::configuration::ServiceConfiguration serviceConfig;
  serviceConfig.name = "OpenAuto";
  serviceConfig.carModel = "Mazda";
  serviceConfig.carYear = "2018";
  serviceConfig.carSerial = "1234";
  serviceConfig.leftHandDrive = false;
  serviceConfig.huManufacturer = "N/A";
  serviceConfig.huModel = "N/A";
  serviceConfig.huBuild = OPENAUTO_VERSION;
  serviceConfig.huVersion = "1";
  serviceConfig.nativeMediaDuringVR = false;
  configuration->setServiceConfig(std::move(serviceConfig));

  configuration->getWifiConfig()->ssid = hostapd_config("ssid");
  configuration->getWifiConfig()->password = hostapd_config("wpa_passphrase");


  gpsManager =  std::make_shared<GPSManager>(ioService, system_connection);
  nightManager = std::make_shared<NightManager>(ioService);
  audioManager = std::make_shared<AudioManager>(system_connection, ioService);

  httpManager = new HttpManager(videoManager);

  navigationManager = std::make_shared<NavigationManager>(system_connection);

  bluetoothManager = std::make_shared<BluetoothManager>(configuration, session_connection, ioService);
  signals = std::make_shared<Signals>(videoManager, audioManager, gpsManager, nightManager, bluetoothManager, navigationManager);

}

void Mazda::start() {
  videoManager->start();
  audioManager->start();
  navigationManager->start();
  httpManager->handle_aa_connect(true);
  bluetoothManager->aaConnect(true);
}

void Mazda::stop() {
  navigationManager->stop();
  videoManager->stop();
  audioManager->stop();
  httpManager->handle_aa_connect(false);
  bluetoothManager->aaConnect(false);
}

std::string Mazda::hostapd_config(const std::string &key) {
  std::ifstream hostapd_file;
  hostapd_file.open("/tmp/current-session-hostapd.conf");

  if (hostapd_file) {
    std::string line;
    size_t pos;
    while (hostapd_file.good()) {
      getline(hostapd_file, line); // get line from file
      if (line[0] != '#') {
        pos = line.find(key); // search
        if (pos != std::string::npos) // string::npos is returned if string is not found
        {
          size_t equalPosition = line.find('=');
          std::string value = line.substr(equalPosition + 1);
          LOG(DEBUG) << value;
          return value;
        }
      }
    }
    return "";
  }
  return "";
}
#endif

