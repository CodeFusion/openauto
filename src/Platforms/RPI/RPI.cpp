
#include "Platforms/RPI/RPI.hpp"

#ifdef BUILD_RPI
RPI::RPI( autoapp::configuration::IConfiguration::Pointer configuration) {
  videoManager = std::make_shared<VideoManager>();
  aaSignals = std::make_shared<AASignals>();


  gpsManager =  std::make_shared<GPSManager>();
  nightManager = std::make_shared<NightManager>();
  audioManager = std::make_shared<AudioManager>();
  bluetoothManager = std::make_shared<BluetoothManager>();

  autoapp::configuration::AudioConfiguration audioConfig;
  autoapp::configuration::AudioChannel mediaChannel;
  mediaChannel.channels = 2;
  mediaChannel.rate = 48000;
  mediaChannel.outputs.emplace_back("default");
  autoapp::configuration::AudioChannel speechChannel;
  speechChannel.channels = 1;
  speechChannel.rate = 16000;
  speechChannel.outputs.emplace_back("default");
  audioConfig.channels.emplace("Media", std::move(mediaChannel));
  audioConfig.channels.emplace("Speech", std::move(speechChannel));
  configuration->setAudioConfig(std::move(audioConfig));


  signals = std::make_shared<Signals>(videoManager, audioManager, gpsManager, aaSignals, nightManager, bluetoothManager);
}

RPI::start(){

}

RPI::stop(){

}

#endif

