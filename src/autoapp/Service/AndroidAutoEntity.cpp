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

#include <aasdk/Channel/Control/ControlServiceChannel.hpp>
#include <autoapp/Service/AndroidAutoEntity.hpp>
#include <easylogging++.h>

namespace autoapp::service {

AndroidAutoEntity::AndroidAutoEntity(asio::io_service &ioService,
                                     aasdk::messenger::ICryptor::Pointer cryptor,
                                     aasdk::transport::ITransport::Pointer transport,
                                     aasdk::messenger::IMessenger::Pointer messenger,
                                     configuration::IConfiguration::Pointer configuration,
                                     ServiceList serviceList,
                                     IPinger::Pointer pinger,
                                     const Signals &signals,
                                     AudioFocusRequest::Pointer audioFocusRequest)
    : strand_(ioService),
      cryptor_(std::move(cryptor)),
      transport_(std::move(transport)),
      messenger_(std::move(messenger)),
      controlServiceChannel_(std::make_shared<aasdk::channel::control::ControlServiceChannel>(strand_, messenger_)),
      configuration_(std::move(configuration)),
      serviceList_(std::move(serviceList)),
      pinger_(std::move(pinger)),
      eventHandler_(nullptr),
      signals_(signals),
      audioFocusRequest_(std::move(audioFocusRequest)) {
}

AndroidAutoEntity::~AndroidAutoEntity() {
  LOG(DEBUG) << "[AndroidAutoEntity] destroy.";
}

void AndroidAutoEntity::start(IAndroidAutoEntityEventHandler &eventHandler) {
  strand_.dispatch([this, self = this->shared_from_this(), eventHandler = &eventHandler]() {
    LOG(INFO) << "[AndroidAutoEntity] start.";

    eventHandler_ = eventHandler;
    std::for_each(serviceList_.begin(), serviceList_.end(), [](IService::Pointer &service) { service->start(); });
    this->schedulePing();

    signals_.audioSignals->focusChanged.connect(sigc::mem_fun(*this, &AndroidAutoEntity::onAudioFocusResponse));
    signals_.aaSignals->connected.emit(true);
    signals_.aaSignals->shutdown.connect(sigc::mem_fun(*this, &AndroidAutoEntity::sendShutdownRequest));

    auto versionRequestPromise = aasdk::channel::SendPromise::defer(strand_);
    versionRequestPromise->then([]() {}, [&](const aasdk::error::Error &e) { onChannelError(e); });
    controlServiceChannel_->sendVersionRequest(std::move(versionRequestPromise));
    controlServiceChannel_->receive(this->shared_from_this());
  });
}

void AndroidAutoEntity::stop() {
  strand_.dispatch([this, self = this->shared_from_this()]() {
    LOG(INFO) << "[AndroidAutoEntity] stop.";

    try {
      eventHandler_ = nullptr;
      std::for_each(serviceList_.begin(), serviceList_.end(), [](IService::Pointer &service) { service->stop(); });
      pinger_->cancel();
      messenger_->stop();
      transport_->stop();
      cryptor_->deinit();
      signals_.aaSignals->connected.emit(false);
    } catch (...) {
      LOG(INFO) << "[AndroidAutoEntity] exception in stop.";
    }
  });
}

void AndroidAutoEntity::pause() {
  strand_.dispatch([this, self = this->shared_from_this()]() {
    LOG(INFO) << "[AndroidAutoEntity] pause.";

    try {
      std::for_each(serviceList_.begin(), serviceList_.end(), [](IService::Pointer &service) { service->pause(); });
    } catch (...) {
      LOG(INFO) << "[AndroidAutoEntity] exception in pause.";
    }
  });
}

void AndroidAutoEntity::resume() {
  strand_.dispatch([this, self = this->shared_from_this()]() {
    LOG(INFO) << "[AndroidAutoEntity] resume.";

    try {
      std::for_each(serviceList_.begin(), serviceList_.end(), [](IService::Pointer &service) { service->pause(); });
    } catch (...) {
      LOG(INFO) << "[AndroidAutoEntity] exception in resume.";
    }
  });
}

void AndroidAutoEntity::onVersionResponse(uint16_t majorCode,
                                          uint16_t minorCode,
                                          aasdk::proto::enums::VersionResponseStatus::Enum status) {
  LOG(INFO) << "[AndroidAutoEntity] version response, version: " << majorCode
            << "." << minorCode
            << ", status: " << status;

  if (status == aasdk::proto::enums::VersionResponseStatus::MISMATCH) {
    LOG(ERROR) << "[AndroidAutoEntity] version mismatch.";
    this->triggerQuit();
  } else {
    LOG(INFO) << "[AndroidAutoEntity] Begin handshake.";

    try {
      cryptor_->doHandshake();

      auto handshakePromise = aasdk::channel::SendPromise::defer(strand_);
      handshakePromise->then([]() {}, [&](const aasdk::error::Error &e) { onChannelError(e); });
      controlServiceChannel_->sendHandshake(cryptor_->readHandshakeBuffer(), std::move(handshakePromise));
      controlServiceChannel_->receive(this->shared_from_this());
    }
    catch (const aasdk::error::Error &e) {
      this->onChannelError(e);
    }
  }
}

void AndroidAutoEntity::onHandshake(const aasdk::common::DataConstBuffer &payload) {
  LOG(INFO) << "[AndroidAutoEntity] Handshake, size: " << payload.size;

  try {
    cryptor_->writeHandshakeBuffer(payload);

    if (!cryptor_->doHandshake()) {
      LOG(INFO) << "[AndroidAutoEntity] continue handshake.";

      auto handshakePromise = aasdk::channel::SendPromise::defer(strand_);
      handshakePromise->then([]() {}, [&](const aasdk::error::Error &e) { onChannelError(e); });
      controlServiceChannel_->sendHandshake(cryptor_->readHandshakeBuffer(), std::move(handshakePromise));
    } else {
      LOG(INFO) << "[AndroidAutoEntity] Auth completed.";

      aasdk::proto::messages::AuthCompleteIndication authCompleteIndication;
      authCompleteIndication.set_status(aasdk::proto::enums::Status::OK);

      auto authCompletePromise = aasdk::channel::SendPromise::defer(strand_);
      authCompletePromise->then([]() {}, [&](const aasdk::error::Error &e) { onChannelError(e); });
      controlServiceChannel_->sendAuthComplete(authCompleteIndication, std::move(authCompletePromise));
    }

    controlServiceChannel_->receive(this->shared_from_this());
  }
  catch (const aasdk::error::Error &e) {
    this->onChannelError(e);
  }
}

void AndroidAutoEntity::onServiceDiscoveryRequest(const aasdk::proto::messages::ServiceDiscoveryRequest &request) {
  LOG(INFO) << "[AndroidAutoEntity] Discovery request, device name: " << request.device_name()
            << ", brand: " << request.device_brand();

  aasdk::proto::messages::ServiceDiscoveryResponse serviceDiscoveryResponse;
  serviceDiscoveryResponse.mutable_channels()->Reserve(256);
  serviceDiscoveryResponse.set_head_unit_name("Crankshaft-NG");
  serviceDiscoveryResponse.set_car_model("Universal");
  serviceDiscoveryResponse.set_car_year("2018");
  serviceDiscoveryResponse.set_car_serial("20180301");
  serviceDiscoveryResponse.set_left_hand_drive_vehicle(configuration_->leftHandDrive());
  serviceDiscoveryResponse.set_headunit_manufacturer("f1x");
  serviceDiscoveryResponse.set_headunit_model("Crankshaft-NG Autoapp");
  serviceDiscoveryResponse.set_sw_build("1");
  serviceDiscoveryResponse.set_sw_version("1.0");
  serviceDiscoveryResponse.set_can_play_native_media_during_vr(false);
  serviceDiscoveryResponse.set_hide_clock(configuration_->hideClock());

  std::for_each(serviceList_.begin(),
                serviceList_.end(),
                [&](IService::Pointer &service) { service->fillFeatures(serviceDiscoveryResponse); });

  auto promise = aasdk::channel::SendPromise::defer(strand_);
  promise->then([]() {}, [&](const aasdk::error::Error &e) { onChannelError(e); });
  controlServiceChannel_->sendServiceDiscoveryResponse(serviceDiscoveryResponse, std::move(promise));
  controlServiceChannel_->receive(this->shared_from_this());
}

void AndroidAutoEntity::onAudioFocusRequest(const aasdk::proto::messages::AudioFocusRequest &request) {
  LOG(INFO) << "[AndroidAutoEntity] requested audio focus, type: "
            << aasdk::proto::enums::AudioFocusType_Enum_Name(request.audio_focus_type());
  auto promise = AudioFocusRequest::Promise::defer(strand_);
  promise->then([]() {},
                [this, self = this->shared_from_this()](auto error) {
                  if (error != aasdk::error::ErrorCode::OPERATION_ABORTED &&
                      error != aasdk::error::ErrorCode::OPERATION_IN_PROGRESS) {
                    LOG(ERROR) << "[AndroidAutoEntity] AudioFocus timer exceeded.";
                    this->onAudioFocusResponse(aasdk::messenger::ChannelId::MEDIA_AUDIO,
                                               aasdk::proto::enums::AudioFocusState::NONE);
                  }
                });
  audioFocusRequest_->request(std::move(promise));

  switch (request.audio_focus_type()) {

    case aasdk::proto::enums::AudioFocusType_Enum_NONE:break;
    case aasdk::proto::enums::AudioFocusType_Enum_GAIN:
      signals_.audioSignals->focusRequest(aasdk::messenger::ChannelId::MEDIA_AUDIO,
                                          request.audio_focus_type());
      break;
    case aasdk::proto::enums::AudioFocusType_Enum_GAIN_TRANSIENT:
//      signals_.audioSignals->focusRequest(aasdk::messenger::ChannelId::SYSTEM_AUDIO,
//                                          request.audio_focus_type());
//      break;
    case aasdk::proto::enums::AudioFocusType_Enum_GAIN_NAVI:
      signals_.audioSignals->focusRequest(aasdk::messenger::ChannelId::SPEECH_AUDIO,
                                          request.audio_focus_type());
      break;
    case aasdk::proto::enums::AudioFocusType_Enum_RELEASE:
      onAudioFocusResponse(aasdk::messenger::ChannelId::NONE,
                           aasdk::proto::enums::AudioFocusState_Enum::AudioFocusState_Enum_LOSS);
      break;
  }
}

void AndroidAutoEntity::onAudioFocusResponse(aasdk::messenger::ChannelId channel_id,
                                             const aasdk::proto::enums::AudioFocusState_Enum state) {
  LOG(INFO) << "[AndroidAutoEntity] audio focus state: " << aasdk::proto::enums::AudioFocusState_Enum_Name(state);
  audioFocusRequest_->cancel();
  strand_.dispatch([this, self = this->shared_from_this(), state]() {
    aasdk::proto::messages::AudioFocusResponse response;
    response.set_audio_focus_state(state);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, [&](const aasdk::error::Error &e) { onChannelError(e); });
    controlServiceChannel_->sendAudioFocusResponse(response, std::move(promise));
    controlServiceChannel_->receive(this->shared_from_this());
  });
}

void AndroidAutoEntity::onShutdownRequest(const aasdk::proto::messages::ShutdownRequest &request) {
  LOG(INFO) << "[AndroidAutoEntity] Shutdown request, reason: " << request.reason();

  aasdk::proto::messages::ShutdownResponse response;
  auto promise = aasdk::channel::SendPromise::defer(strand_);
  promise->then([&]() { triggerQuit(); },
                [&](const aasdk::error::Error &e) { onChannelError(e); });

  controlServiceChannel_->sendShutdownResponse(response, std::move(promise));
}

void AndroidAutoEntity::sendShutdownRequest() {
  aasdk::proto::messages::ShutdownRequest request;
  request.set_reason(aasdk::proto::enums::ShutdownReason::QUIT);
  auto promise = aasdk::channel::SendPromise::defer(strand_);
  promise->then([]() {},
                [&](const aasdk::error::Error &e) { onChannelError(e); });

  controlServiceChannel_->sendShutdownRequest(request, std::move(promise));
}

void AndroidAutoEntity::onShutdownResponse(const aasdk::proto::messages::ShutdownResponse &) {
  LOG(INFO) << "[AndroidAutoEntity] Shutdown response ";
  this->triggerQuit();
}

void AndroidAutoEntity::onNavigationFocusRequest(const aasdk::proto::messages::NavigationFocusRequest &request) {
  LOG(INFO) << "[AndroidAutoEntity] navigation focus request, type: "
            << aasdk::proto::enums::NavigationFocusType::Enum_Name(request.type());

  aasdk::proto::messages::NavigationFocusResponse response;
  response.set_type(aasdk::proto::enums::NavigationFocusType::NAVIGATION_FOCUS_PROJECTED);

  auto promise = aasdk::channel::SendPromise::defer(strand_);
  promise->then([]() {}, [&](const aasdk::error::Error &e) { onChannelError(e); });
  controlServiceChannel_->sendNavigationFocusResponse(response, std::move(promise));
  controlServiceChannel_->receive(this->shared_from_this());
}

void AndroidAutoEntity::onVoiceSessionRequest(const aasdk::proto::messages::VoiceSessionRequest &request) {
  LOG(INFO) << "[AndroidAutoEntity] Voice session request, type: "
            << ((request.type() == 1) ? "START" : ((request.type() == 2) ? "STOP" : "UNKNOWN"));

  auto promise = aasdk::channel::SendPromise::defer(strand_);
  promise->then([]() {}, [&](const aasdk::error::Error &e) { onChannelError(e); });
  controlServiceChannel_->receive(this->shared_from_this());
}

void AndroidAutoEntity::onPingRequest(const aasdk::proto::messages::PingRequest &request) {
//        LOG(INFO) << "[AndroidAutoEntity] Ping Request";

  aasdk::proto::messages::PingResponse response;
  response.set_timestamp(request.timestamp());
  auto promise = aasdk::channel::SendPromise::defer(strand_);
  promise->then([]() {}, [&](const aasdk::error::Error &e) { onChannelError(e); });
  controlServiceChannel_->sendPingResponse(response, std::move(promise));
  controlServiceChannel_->receive(this->shared_from_this());
}

void AndroidAutoEntity::onPingResponse(const aasdk::proto::messages::PingResponse &response) {
  VLOG(9) << "[AndroidAutoEntity] Ping response, timestamp: " << response.timestamp();
  pinger_->pong();
  controlServiceChannel_->receive(this->shared_from_this());
}

void AndroidAutoEntity::onChannelError(const aasdk::error::Error &e) {
  LOG(ERROR) << "[AndroidAutoEntity] channel error: " << e.what();
  this->triggerQuit();
}

void AndroidAutoEntity::triggerQuit() {
  if (eventHandler_ != nullptr) {
    eventHandler_->onAndroidAutoQuit();
  }
}

void AndroidAutoEntity::schedulePing() {
  auto promise = IPinger::Promise::defer(strand_);
  promise->then([this, self = this->shared_from_this()]() {
                  this->sendPing();
                  this->schedulePing();
                },
                [this, self = this->shared_from_this()](auto error) {
                  if (error != aasdk::error::ErrorCode::OPERATION_ABORTED &&
                      error != aasdk::error::ErrorCode::OPERATION_IN_PROGRESS) {
                    LOG(ERROR) << "[AndroidAutoEntity] ping timer exceeded.";
                    this->triggerQuit();
                  }
                });

  pinger_->ping(std::move(promise));
}

void AndroidAutoEntity::sendPing() {
  auto promise = aasdk::channel::SendPromise::defer(strand_);
  promise->then([]() {}, [&](const aasdk::error::Error &e) { onChannelError(e); });

  aasdk::proto::messages::PingRequest request;
  auto timestamp =
      std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
  request.set_timestamp(timestamp.count());
  controlServiceChannel_->sendPingRequest(request, std::move(promise));
}

}
