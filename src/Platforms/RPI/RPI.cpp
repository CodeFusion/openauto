
#include "Platforms/RPI/RPI.hpp"

#ifdef BUILD_RPI
RPI::RPI() {
  videoManager = std::make_shared<VideoManager>();
  aaSignals = std::make_shared<AASignals>();


  gpsManager =  std::make_shared<GPSManager>();
  nightManager = std::make_shared<NightManager>();
  audioManager = std::make_shared<AudioManager>();
  bluetoothManager = std::make_shared<BluetoothManager>();

  signals = std::make_shared<Signals>(videoManager, audioManager, gpsManager, aaSignals, nightManager, bluetoothManager);
}


#endif

