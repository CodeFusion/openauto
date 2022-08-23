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

#include <easylogging++.h>
#include <autoapp/Service/AudioService.hpp>
#include "aasdk/Channel/AV/AudioServiceChannel.hpp"

namespace autoapp::service {

AudioService::AudioService(asio::io_service &ioService,
                           aasdk::messenger::IMessenger::Pointer messenger,
                           aasdk::messenger::ChannelId channelID,
                           std::vector<projection::IAudioOutput::Pointer> audioOutput,
                           IAudioManager::Pointer AudioManager)
    : strand_(ioService),
      WriterStrand(ioService),
      audioOutput_(std::move(audioOutput)),
      session_(-1),
      audioManager(std::move(AudioManager)){
  channel_ = std::make_shared<aasdk::channel::av::AudioServiceChannel>(strand_, std::move(messenger), channelID);
}

void AudioService::start() {
  strand_.dispatch([this, self = this->shared_from_this()]() {
    LOG(INFO) << "[AudioService] start, channel: " << aasdk::messenger::channelIdToString(channel_->getId());
    channel_->receive(this->shared_from_this());
  });
}

void AudioService::stop() {
  strand_.dispatch([this, self = this->shared_from_this()]() {
    LOG(INFO) << "[AudioService] stop, channel: " << aasdk::messenger::channelIdToString(channel_->getId());
    for(auto &audioOutput: audioOutput_) {
      audioOutput->stop();
    }
  });
}

void AudioService::pause() {
  strand_.dispatch([this, self = this->shared_from_this()]() {
    LOG(INFO) << "[AudioService] pause.";
  });
}

void AudioService::resume() {
  strand_.dispatch([this, self = this->shared_from_this()]() {
    LOG(INFO) << "[AudioService] resume.";
  });
}

void AudioService::fillFeatures(aasdk::proto::messages::ServiceDiscoveryResponse &response) {
  LOG(INFO) << "[AudioService] fill features, channel: " << aasdk::messenger::channelIdToString(channel_->getId());

  auto *channelDescriptor = response.add_channels();
  channelDescriptor->set_channel_id(static_cast<uint32_t>(channel_->getId()));

  auto *audioChannel = channelDescriptor->mutable_av_channel();
  audioChannel->set_stream_type(aasdk::proto::enums::AVStreamType::AUDIO_AAC_ADTS);

  switch (channel_->getId()) {
    case aasdk::messenger::ChannelId::SYSTEM_AUDIO:audioChannel->set_audio_type(aasdk::proto::enums::AudioType::SYSTEM);
      break;

    case aasdk::messenger::ChannelId::MEDIA_AUDIO:audioChannel->set_audio_type(aasdk::proto::enums::AudioType::MEDIA);
      break;

    case aasdk::messenger::ChannelId::SPEECH_AUDIO:audioChannel->set_audio_type(aasdk::proto::enums::AudioType::SPEECH);
      break;
    default:break;
  }

  audioChannel->set_available_while_in_call(true);

  auto *audioConfig = audioChannel->add_audio_configs();
  audioConfig->set_sample_rate(audioOutput_.front()->getSampleRate());
  audioConfig->set_bit_depth(audioOutput_.front()->getSampleSize());
  audioConfig->set_channel_count(audioOutput_.front()->getChannelCount());
}

void AudioService::onChannelOpenRequest(const aasdk::proto::messages::ChannelOpenRequest &request) {
  LOG(INFO) << "[AudioService] open request"
            << ", channel: " << aasdk::messenger::channelIdToString(channel_->getId())
            << ", priority: " << request.priority();

  LOG(DEBUG) << "[AudioService] channel: " << aasdk::messenger::channelIdToString(channel_->getId())
             << " audio output sample rate: " << audioOutput_.front()->getSampleRate()
             << ", sample size: " << audioOutput_.front()->getSampleSize()
             << ", channel count: " << audioOutput_.front()->getChannelCount();

  bool openStatus = true;
  for(auto &audioOutput: audioOutput_){
    openStatus = (audioOutput->open() && openStatus);
  }

  const aasdk::proto::enums::Status::Enum
      status = openStatus ? aasdk::proto::enums::Status::OK : aasdk::proto::enums::Status::FAIL;
  LOG(INFO) << "[AudioService] open status: " << status
            << ", channel: " << aasdk::messenger::channelIdToString(channel_->getId());

  aasdk::proto::messages::ChannelOpenResponse response;
  response.set_status(status);

  auto promise = aasdk::channel::SendPromise::defer(strand_);
  promise->then(std::function<void(void)>([]() {}),
                [this, self =
                this->shared_from_this()](const aasdk::error::Error &error) -> void { this->onChannelError(error); });
  channel_->sendChannelOpenResponse(response, std::move(promise));
  channel_->receive(this->shared_from_this());
}

void AudioService::onAVChannelSetupRequest(const aasdk::proto::messages::AVChannelSetupRequest &request) {
  LOG(INFO) << "[AudioService] setup request"
            << ", channel: " << aasdk::messenger::channelIdToString(channel_->getId())
            << ", config index: " << request.config_index();
  const aasdk::proto::enums::AVChannelSetupStatus::Enum status = aasdk::proto::enums::AVChannelSetupStatus::OK;
  LOG(INFO) << "[AudioService] setup status: " << status
            << ", channel: " << aasdk::messenger::channelIdToString(channel_->getId());

  aasdk::proto::messages::AVChannelSetupResponse response;
  response.set_media_status(status);
  response.set_max_unacked(10);
  response.add_configs(0);

  auto promise = aasdk::channel::SendPromise::defer(strand_);
  promise->then(std::function<void(void)>([]() {}),
                [this, self =
                this->shared_from_this()](const aasdk::error::Error &error) -> void { this->onChannelError(error); });
  channel_->sendAVChannelSetupResponse(response, std::move(promise));
  channel_->receive(this->shared_from_this());
}

void AudioService::onAVChannelStartIndication(const aasdk::proto::messages::AVChannelStartIndication &indication) {
  LOG(INFO) << "[AudioService] start indication"
            << ", channel: " << aasdk::messenger::channelIdToString(channel_->getId())
            << ", session: " << indication.session();
  session_ = indication.session();
  for(auto &audioOutput: audioOutput_){
    audioOutput->start();
  }
  channel_->receive(this->shared_from_this());
}

void AudioService::onAVChannelStopIndication(const aasdk::proto::messages::AVChannelStopIndication &indication) {
  LOG(INFO) << "[AudioService] stop indication"
            << ", channel: " << aasdk::messenger::channelIdToString(channel_->getId())
            << ", session: " << session_;
  session_ = -1;
  for(auto &audioOutput: audioOutput_) {
    audioOutput->suspend();
  }
  channel_->receive(this->shared_from_this());
}

void AudioService::onAVMediaWithTimestampIndication(aasdk::messenger::Timestamp::ValueType timestamp,
                                                    const aasdk::common::DataConstBuffer &buffer) {

  //Copy the buffer, so we can control its lifecycle.
  std::vector<uint8_t> dataBuffer;
  std::copy(buffer.cdata, buffer.cdata+buffer.size, std::back_inserter(dataBuffer));

  //post this task, so that we don't block here.
  WriterStrand.post([this, timestamp, databuffer=std::move(dataBuffer), size=buffer.size](){
    aasdk::common::DataConstBuffer tempBuffer(aasdk::common::DataConstBuffer(databuffer.data(), size));
    VLOG(9) << "Wrote " << tempBuffer.size;
    for(auto &audioOutput: audioOutput_) {
      audioOutput->write(timestamp, tempBuffer);
    }
  });

  aasdk::proto::messages::AVMediaAckIndication indication;
  indication.set_session(session_);
  indication.set_value(1);

  auto promise = aasdk::channel::SendPromise::defer(strand_);
  promise->then(std::function<void(void)>([]() {}),
                [this, self =
                this->shared_from_this()](const aasdk::error::Error &error) -> void { this->onChannelError(error); });
  channel_->sendAVMediaAckIndication(indication, std::move(promise));
  channel_->receive(this->shared_from_this());
}

void AudioService::onAVMediaIndication(const aasdk::common::DataConstBuffer &buffer) {
  this->onAVMediaWithTimestampIndication(0, buffer);
}

void AudioService::onChannelError(const aasdk::error::Error &error) {
  LOG(ERROR) << "[AudioService] channel error: " << error.what()
             << ", channel: " << aasdk::messenger::channelIdToString(channel_->getId());
}

AudioService::~AudioService() noexcept {
  LOG(DEBUG) << "Audio Service Destroyed, Channel " << aasdk::messenger::channelIdToString(channel_->getId());
}

}
