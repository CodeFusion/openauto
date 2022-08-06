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
#include <autoapp/Projection/MazdaBluetooth.hpp>
#include <autoapp/Service/NavigationService.hpp>

namespace autoapp::service {

ServiceFactory::ServiceFactory(asio::io_service &ioService,
                               configuration::IConfiguration::Pointer configuration,
                               const Signals &signals)
    : ioService_(ioService), configuration_(std::move(configuration)), signals_(signals) {

}

ServiceList ServiceFactory::create(aasdk::messenger::IMessenger::Pointer messenger) {
  ServiceList serviceList;

  projection::IAudioInput::Pointer audioInput(new projection::AlsaAudioInput(ioService_));
  serviceList.emplace_back(std::make_shared<AudioInputService>(ioService_, messenger, std::move(audioInput)));
  this->createAudioServices(serviceList, messenger);
  serviceList.emplace_back(std::make_shared<SensorService>(ioService_, messenger, signals_.gpsManager, signals_.nightManager));
  serviceList.emplace_back(std::make_shared<NavigationService>(ioService_, messenger, signals_.navSignals));
  serviceList.emplace_back(this->createVideoService(messenger));
  serviceList.emplace_back(this->createBluetoothService(messenger));
  serviceList.emplace_back(this->createInputService(messenger));
  return serviceList;
}

IService::Pointer ServiceFactory::createVideoService(aasdk::messenger::IMessenger::Pointer messenger) {
  projection::IVideoOutput::Pointer videoOutput(new projection::GSTVideoOutput(ioService_));
  return std::make_shared<VideoService>(ioService_, messenger, std::move(videoOutput), signals_.videoManager);
}

IService::Pointer ServiceFactory::createBluetoothService(aasdk::messenger::IMessenger::Pointer messenger) {
  projection::IBluetoothDevice::Pointer bluetoothDevice;
  bluetoothDevice = std::make_shared<projection::MazdaBluetooth>();

  return std::make_shared<BluetoothService>(ioService_, messenger, std::move(bluetoothDevice));
}

IService::Pointer ServiceFactory::createInputService(aasdk::messenger::IMessenger::Pointer messenger) {
  projection::IInputDevice::Pointer
      inputDevice(std::make_shared<projection::InputDevice>(ioService_, signals_.audioSignals, signals_.videoManager));

  return std::make_shared<InputService>(ioService_, messenger, std::move(inputDevice));
}

void ServiceFactory::createAudioServices(ServiceList &serviceList,
                                         const aasdk::messenger::IMessenger::Pointer &messenger) {
  auto mediaAudioOutput = std::make_shared<projection::AlsaAudioOutput>(2, 48000, "entertainmentMl");
  std::vector<projection::IAudioOutput::Pointer> mediaOutputs;
  mediaOutputs.emplace_back(std::move(mediaAudioOutput));
  serviceList.emplace_back(std::make_shared<AudioService>(ioService_,
                                                          messenger,
                                                          aasdk::messenger::ChannelId::MEDIA_AUDIO,
                                                          std::move(mediaOutputs),
                                                          signals_.audioSignals));

  //Setup two outputs for this, as Android Auto doesn't mix the speech and entertainment channels automagically
  auto speechAudioOutput1 = std::make_shared<projection::AlsaAudioOutput>(1, 16000, "informationNavi");
  auto speechAudioOutput2 = std::make_shared<projection::AlsaAudioOutput>(1, 16000, "entertainmentMl");
  std::vector<projection::IAudioOutput::Pointer> speechOutputs;
  speechOutputs.emplace_back(std::move(speechAudioOutput1));
  speechOutputs.emplace_back(std::move(speechAudioOutput2));
  serviceList.emplace_back(std::make_shared<AudioService>(ioService_,
                                                          messenger,
                                                          aasdk::messenger::ChannelId::SPEECH_AUDIO,
                                                          std::move(speechOutputs),
                                                          signals_.audioSignals));

//  auto systemAudioOutput = std::make_shared<projection::AlsaAudioOutput>(1, 16000, "vrGeneric");
//  serviceList.emplace_back(std::make_shared<SystemAudioService>(ioService_,
//                                                                messenger,
//                                                                std::move(systemAudioOutput),
//                                                                signals_.audioSignals));
}

}
