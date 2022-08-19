/*
*  This file is part of openauto project.
*  Copyright (C) 2018 f1x.studio (Michal Szwaj)
*
*  openauto is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 3 of the License, or
*  (at your option) any later version.

*  openauto is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with openauto. If not, see <http://www.gnu.org/licenses/>.
*/

#include <aasdk/Channel/AV/MediaAudioServiceChannel.hpp>
#include <autoapp/Service/ServiceFactory.hpp>
#include <autoapp/Service/VideoService.hpp>
#include <autoapp/Service/AudioService.hpp>
#include <autoapp/Service/AudioInputService.hpp>
#include <autoapp/Service/SensorService.hpp>
#include <autoapp/Service/BluetoothService.hpp>
#include <autoapp/Service/InputService.hpp>
#include <autoapp/Projection/GSTVideoOutput.hpp>
#include <autoapp/Projection/AlsaAudio.hpp>
#include <autoapp/Projection/AlsaAudioInput.hpp>
#include <autoapp/Projection/InputDevice.hpp>
#include <autoapp/Service/NavigationService.hpp>

namespace autoapp::service {

ServiceFactory::ServiceFactory(asio::io_service &ioService,
                               configuration::Configuration::Pointer configuration,
                              IPlatform::Pointer Platform)
    : ioService_(ioService), configuration_(std::move(configuration)), platform(std::move(Platform)) {

}

ServiceList ServiceFactory::create(aasdk::messenger::IMessenger::Pointer messenger) {
  ServiceList serviceList;

  projection::IAudioInput::Pointer audioInput(new projection::AlsaAudioInput(ioService_));
  serviceList.emplace_back(std::make_shared<AudioInputService>(ioService_, messenger, std::move(audioInput)));
  this->createAudioServices(serviceList, messenger);
  serviceList.emplace_back(std::make_shared<SensorService>(ioService_, messenger, platform->gpsManager, platform->nightManager));
  serviceList.emplace_back(std::make_shared<NavigationService>(ioService_, messenger, platform->navigationManager));
  serviceList.emplace_back(this->createVideoService(messenger));
  serviceList.emplace_back(this->createBluetoothService(messenger));
  serviceList.emplace_back(this->createInputService(messenger));
  return serviceList;
}

IService::Pointer ServiceFactory::createVideoService(aasdk::messenger::IMessenger::Pointer messenger) {
  projection::IVideoOutput::Pointer videoOutput(new projection::GSTVideoOutput(ioService_));
  return std::make_shared<VideoService>(ioService_, messenger, std::move(videoOutput), platform->videoManager);
}

IService::Pointer ServiceFactory::createBluetoothService(aasdk::messenger::IMessenger::Pointer messenger) {
  return std::make_shared<BluetoothService>(ioService_, messenger, std::move(platform->bluetoothPairingManager));
}

IService::Pointer ServiceFactory::createInputService(aasdk::messenger::IMessenger::Pointer messenger) {
  projection::IInputDevice::Pointer
      inputDevice(std::make_shared<projection::InputDevice>(ioService_, platform->audioManager, platform->videoManager));

  return std::make_shared<InputService>(ioService_, messenger, std::move(inputDevice));
}

void ServiceFactory::createAudioServices(ServiceList &serviceList,
                                         const aasdk::messenger::IMessenger::Pointer &messenger) {
  std::vector<projection::IAudioOutput::Pointer> mediaOutputs;
  auto mediaConfig = configuration_->getAudioConfig().channels["Media"];
  for (auto &output: mediaConfig.outputs){
    auto audioOutput = std::make_shared<projection::AlsaAudioOutput>(mediaConfig.channels, mediaConfig.rate, output.c_str());
    mediaOutputs.emplace_back(std::move(audioOutput));
  }

  serviceList.emplace_back(std::make_shared<AudioService>(ioService_,
                                                          messenger,
                                                          aasdk::messenger::ChannelId::MEDIA_AUDIO,
                                                          std::move(mediaOutputs),
                                                          platform->audioManager));

  //Setup two outputs for this, as Android Auto doesn't mix the speech and entertainment channels automagically
  std::vector<projection::IAudioOutput::Pointer> speechOutputs;
  auto speechConfig = configuration_->getAudioConfig().channels["Speech"];
  for (auto &output: speechConfig.outputs){
    auto audioOutput = std::make_shared<projection::AlsaAudioOutput>(speechConfig.channels, speechConfig.rate, output.c_str());
    speechOutputs.emplace_back(std::move(audioOutput));
  }
  serviceList.emplace_back(std::make_shared<AudioService>(ioService_,
                                                          messenger,
                                                          aasdk::messenger::ChannelId::SPEECH_AUDIO,
                                                          std::move(speechOutputs),
                                                          platform->audioManager));

//  auto systemAudioOutput = std::make_shared<projection::AlsaAudioOutput>(1, 16000, "vrGeneric");
//  serviceList.emplace_back(std::make_shared<SystemAudioService>(ioService_,
//                                                                messenger,
//                                                                std::move(systemAudioOutput),
//                                                                signals_.audioSignals));
}

}
