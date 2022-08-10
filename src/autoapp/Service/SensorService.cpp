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

#include <aasdk_proto/DrivingStatusEnum.pb.h>
#include <easylogging++.h>
#include <autoapp/Service/SensorService.hpp>
#include <cmath>

namespace autoapp::service {

SensorService::SensorService(asio::io_service &ioService, aasdk::messenger::IMessenger::Pointer messenger,
                             IGPSManager::Pointer gpsmanager, INightManager::Pointer NightManager)
    : timer_(ioService), strand_(ioService),
      channel_(std::make_shared<aasdk::channel::sensor::SensorServiceChannel>(strand_, std::move(messenger))),
      gpsManager(std::move(gpsmanager)), nightManger(std::move(NightManager)) {

}

void SensorService::start() {
  strand_.dispatch([this, self = this->shared_from_this()]() {
    gpsManager->registerLocationCallback([this](const aasdk::proto::data::GPSLocation &location) {
      this->sendGPSLocationData(location);
    });
    nightManger->registerNightCallback([this](bool isNight) { this->sendNightData(isNight); });

    LOG(INFO) << "[SensorService] start.";
    channel_->receive(this->shared_from_this());
  });

}

void SensorService::stop() {
  gpsManager->stop();
  nightManger->stop();
  LOG(INFO) << "[SensorService] stop.";
}

void SensorService::pause() {
  strand_.dispatch([this, self = this->shared_from_this()]() {
    LOG(INFO) << "[SensorService] pause.";
  });
}

void SensorService::resume() {
  strand_.dispatch([this, self = this->shared_from_this()]() {
    LOG(INFO) << "[SensorService] resume.";
  });
}

void SensorService::fillFeatures(aasdk::proto::messages::ServiceDiscoveryResponse &response) {
  LOG(INFO) << "[SensorService] fill features.";

  auto *channelDescriptor = response.add_channels();
  channelDescriptor->set_channel_id(static_cast<uint32_t>(channel_->getId()));

  auto *sensorChannel = channelDescriptor->mutable_sensor_channel();
  sensorChannel->add_sensors()->set_type(aasdk::proto::enums::SensorType::DRIVING_STATUS);
  sensorChannel->add_sensors()->set_type(aasdk::proto::enums::SensorType::LOCATION);
  sensorChannel->add_sensors()->set_type(aasdk::proto::enums::SensorType::NIGHT_DATA);
}

void SensorService::onChannelOpenRequest(const aasdk::proto::messages::ChannelOpenRequest &request) {
  LOG(INFO) << "[SensorService] open request, priority: " << request.priority();
  const aasdk::proto::enums::Status::Enum status = aasdk::proto::enums::Status::OK;
  LOG(INFO) << "[SensorService] open status: " << status;

  aasdk::proto::messages::ChannelOpenResponse response;
  response.set_status(status);

  auto promise = aasdk::channel::SendPromise::defer(strand_);
  promise->then([]() {}, [&](const aasdk::error::Error &error) { onChannelError(error); });
  channel_->sendChannelOpenResponse(response, std::move(promise));

  channel_->receive(this->shared_from_this());
}

void SensorService::onSensorStartRequest(const aasdk::proto::messages::SensorStartRequestMessage &request) {
  LOG(INFO) << "[SensorService] sensor start request, type: " << request.sensor_type();

  aasdk::proto::messages::SensorStartResponseMessage response;
  response.set_status(aasdk::proto::enums::Status::OK);

  auto promise = aasdk::channel::SendPromise::defer(strand_);

  if (request.sensor_type() == aasdk::proto::enums::SensorType::DRIVING_STATUS) {
    promise->then([&]() { sendDrivingStatusUnrestricted(); },
                  [&](const aasdk::error::Error &error) { onChannelError(error); });
  } else if (request.sensor_type() == aasdk::proto::enums::SensorType::NIGHT_DATA) {
    promise->then([&]() { nightManger->start(); },
                  [&](const aasdk::error::Error &error) { onChannelError(error); });
  } else if (request.sensor_type() == aasdk::proto::enums::SensorType::LOCATION) {
    promise->then([&]() { gpsManager->start(); },
                  [&](const aasdk::error::Error &error) { onChannelError(error); });
  } else {
    promise->then([]() {}, [&](const aasdk::error::Error &error) { onChannelError(error); });
  }

  channel_->sendSensorStartResponse(response, std::move(promise));
  channel_->receive(this->shared_from_this());
}

void SensorService::sendDrivingStatusUnrestricted() {
  aasdk::proto::messages::SensorEventIndication indication;
  indication.add_driving_status()->set_status(aasdk::proto::enums::DrivingStatus::UNRESTRICTED);

  auto promise = aasdk::channel::SendPromise::defer(strand_);
  promise->then([]() {}, [&](const aasdk::error::Error &error) { onChannelError(error); });
  channel_->sendSensorEventIndication(indication, std::move(promise));
}

void SensorService::sendNightData(bool isNight) {
  aasdk::proto::messages::SensorEventIndication indication;

  if (isNight) {
    LOG(DEBUG) << "[SensorService] Mode night triggered";
    indication.add_night_mode()->set_is_night(true);
  } else {
    LOG(DEBUG) << "[SensorService] Mode day triggered";
    indication.add_night_mode()->set_is_night(false);
  }

  auto promise = aasdk::channel::SendPromise::defer(strand_);
  promise->then([]() {}, [&](const aasdk::error::Error &error) { onChannelError(error); });
  channel_->sendSensorEventIndication(indication, std::move(promise));
}

void SensorService::sendGPSLocationData(const aasdk::proto::data::GPSLocation &location) {

  if (!location.has_accuracy()) {
    return;
  }

  aasdk::proto::messages::SensorEventIndication indication;
  auto *locInd = indication.add_gps_location();
  locInd->CopyFrom(location);

  auto promise = aasdk::channel::SendPromise::defer(strand_);
  promise->then([]() {}, [&](const aasdk::error::Error &error) { onChannelError(error); });
  channel_->sendSensorEventIndication(indication, std::move(promise));
}

void SensorService::onChannelError(const aasdk::error::Error &error) {
  LOG(ERROR) << "[SensorService] channel error: " << error.what();
}

}

