
#include "Platforms/RPI/RPI.hpp"

#ifdef BUILD_RPI
RPI::RPI( autoapp::configuration::Configuration::Pointer configuration) {
  videoManager = std::make_shared<VideoManager>();


  gpsManager =  std::make_shared<GPSManager>();
  nightManager = std::make_shared<NightManager>();
  audioManager = std::make_shared<AudioManager>();
  bluetoothManager = std::make_shared<BluetoothManager>();
  bluetoothPairingManager = std::make_shared<BluetoothPairingManager>();
  navigationManager = std::make_shared<NavigationManager>();

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


}

void RPI::start(){

}

void RPI::stop(){

}

#endif

