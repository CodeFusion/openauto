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
#include <autoapp/Service/VideoService.hpp>

namespace autoapp::service {

VideoService::VideoService(asio::io_service &ioService,
                           aasdk::messenger::IMessenger::Pointer messenger,
                           projection::IVideoOutput::Pointer videoOutput,
                           VideoSignals::Pointer videoSignals)
    : videoSignals_(std::move(videoSignals)),
      strand_(ioService),
      channel_(std::make_shared<aasdk::channel::av::VideoServiceChannel>(strand_, std::move(messenger))),
      videoOutput_(std::move(videoOutput)),
      session_(-1) {

}

void VideoService::start() {
  strand_.dispatch([this, self = this->shared_from_this()]() {
    LOG(INFO) << "[VideoService] start.";
    channel_->receive(this->shared_from_this());
  });
}

void VideoService::stop() {
  strand_.dispatch([this, self = this->shared_from_this()]() {
    LOG(INFO) << "[VideoService] stop.";
    focusChanged.disconnect();
    videoSignals_->focusRelease.emit();
  });
}

void VideoService::pause() {
  strand_.dispatch([this, self = this->shared_from_this()]() {
    LOG(INFO) << "[VideoService] pause.";
  });
}

void VideoService::resume() {
  strand_.dispatch([this, self = this->shared_from_this()]() {
    LOG(INFO) << "[VideoService] resume.";
  });
}

void VideoService::onChannelOpenRequest(const aasdk::proto::messages::ChannelOpenRequest &request) {
  LOG(INFO) << "[VideoService] open request, priority: " << request.priority();
  const aasdk::proto::enums::Status::Enum status = aasdk::proto::enums::Status::OK;
  focusChanged = videoSignals_->focusChanged.connect([this](bool focus) {
    if (focus) {
      sendVideoFocusIndication();
    }
    else { sendVideoFocusLost(); }
  });
  LOG(INFO) << "[VideoService] open status: " << status;

  aasdk::proto::messages::ChannelOpenResponse response;
  response.set_status(status);

  auto promise = aasdk::channel::SendPromise::defer(strand_);
  promise->then(std::function<void(void)>([]() {}),
                [this, self =
                this->shared_from_this()](const aasdk::error::Error &e) -> void { this->onChannelError(e); });
  channel_->sendChannelOpenResponse(response, std::move(promise));

  channel_->receive(this->shared_from_this());
}

void VideoService::onAVChannelSetupRequest(const aasdk::proto::messages::AVChannelSetupRequest &request) {
  LOG(INFO) << "[VideoService] setup request, config index: " << request.config_index();
  const aasdk::proto::enums::AVChannelSetupStatus::Enum status =
      videoOutput_->init() ? aasdk::proto::enums::AVChannelSetupStatus::OK
                           : aasdk::proto::enums::AVChannelSetupStatus::FAIL;
  LOG(INFO) << "[VideoService] setup status: " << status;

  videoSignals_->focusRequest.emit();

  aasdk::proto::messages::AVChannelSetupResponse response;
  response.set_media_status(status);
  response.set_max_unacked(10);
  response.add_configs(0);

  auto promise = aasdk::channel::SendPromise::defer(strand_);
  promise->then([this, self =
                this->shared_from_this()]() -> void { this->sendVideoFocusIndication(); },
                [this, self =
                this->shared_from_this()](const aasdk::error::Error &e) -> void { this->onChannelError(e); });
  channel_->sendAVChannelSetupResponse(response, std::move(promise));
  channel_->receive(this->shared_from_this());
}

void VideoService::onAVChannelStartIndication(const aasdk::proto::messages::AVChannelStartIndication &indication) {
  LOG(INFO) << "[VideoService] start indication, session: " << indication.session();
  session_ = indication.session();
  videoOutput_->open();

  channel_->receive(this->shared_from_this());
}

void VideoService::onAVChannelStopIndication(const aasdk::proto::messages::AVChannelStopIndication &indication) {
  LOG(INFO) << "[VideoService] stop indication, session: " << session_;
  videoOutput_->stop();

  channel_->receive(this->shared_from_this());
}

void VideoService::onAVMediaWithTimestampIndication(aasdk::messenger::Timestamp::ValueType timestamp,
                                                    const aasdk::common::DataConstBuffer &buffer) {
  videoOutput_->write(timestamp, buffer);

  aasdk::proto::messages::AVMediaAckIndication indication;
  indication.set_session(session_);
  indication.set_value(1);

  auto promise = aasdk::channel::SendPromise::defer(strand_);
  promise->then(std::function<void(void)>([]() {}),
                [this, self =
                this->shared_from_this()](const aasdk::error::Error &e) -> void { this->onChannelError(e); });
  channel_->sendAVMediaAckIndication(indication, std::move(promise));

  channel_->receive(this->shared_from_this());
}

void VideoService::onAVMediaIndication(const aasdk::common::DataConstBuffer &buffer) {
  videoOutput_->write(0, buffer);

  aasdk::proto::messages::AVMediaAckIndication indication;
  indication.set_session(session_);
  indication.set_value(1);

  auto promise = aasdk::channel::SendPromise::defer(strand_);
  promise->then(std::function<void(void)>([]() {}),
                [this, self =
                this->shared_from_this()](const aasdk::error::Error &e) -> void { this->onChannelError(e); });
  channel_->sendAVMediaAckIndication(indication, std::move(promise));

  channel_->receive(this->shared_from_this());
}

void VideoService::onChannelError(const aasdk::error::Error &e) {
  LOG(ERROR) << "[VideoService] channel error: " << e.what();
}

void VideoService::fillFeatures(aasdk::proto::messages::ServiceDiscoveryResponse &response) {
  LOG(INFO) << "[VideoService] fill features.";

  auto *channelDescriptor = response.add_channels();
  channelDescriptor->set_channel_id(static_cast<uint32_t>(channel_->getId()));

  auto *videoChannel = channelDescriptor->mutable_av_channel();
  videoChannel->set_stream_type(aasdk::proto::enums::AVStreamType::VIDEO);
  videoChannel->set_available_while_in_call(true);

  auto *videoConfig1 = videoChannel->add_video_configs();
  videoConfig1->set_video_resolution(videoOutput_->getVideoResolution());
  videoConfig1->set_video_fps(videoOutput_->getVideoFPS());

  const auto &videoMargins = videoOutput_->getVideoMargins();
  videoConfig1->set_margin_height(videoMargins.height());
  videoConfig1->set_margin_width(videoMargins.width());
  videoConfig1->set_dpi(videoOutput_->getScreenDPI());
}

void VideoService::onVideoFocusRequest(const aasdk::proto::messages::VideoFocusRequest &request) {
  LOG(INFO) << "[VideoService] video focus request, display index: " << request.disp_index()
            << ", focus mode: " << request.focus_mode()
            << ", focus reason: " << request.focus_reason();

  videoSignals_->focusRequest.emit();
  channel_->receive(this->shared_from_this());
}

void VideoService::sendVideoFocusIndication() {
  LOG(INFO) << "[VideoService] video focus indication.";

  aasdk::proto::messages::VideoFocusIndication videoFocusIndication;
  videoFocusIndication.set_focus_mode(aasdk::proto::enums::VideoFocusMode::FOCUSED);
  videoFocusIndication.set_unrequested(false);

  auto promise = aasdk::channel::SendPromise::defer(strand_);
  promise->then(std::function<void(void)>([]() {}),
                [this, self =
                this->shared_from_this()](const aasdk::error::Error &e) -> void { this->onChannelError(e); });
  channel_->sendVideoFocusIndication(videoFocusIndication, std::move(promise));
}

void VideoService::sendVideoFocusLost() {
  LOG(INFO) << "[VideoService] video focus indication.";
  videoOutput_->stop();

  aasdk::proto::messages::VideoFocusIndication videoFocusIndication;
  videoFocusIndication.set_focus_mode(aasdk::proto::enums::VideoFocusMode::UNFOCUSED);
  videoFocusIndication.set_unrequested(false);

  auto promise = aasdk::channel::SendPromise::defer(strand_);
  promise->then(std::function<void(void)>([]() {}),
                [this, self =
                this->shared_from_this()](const aasdk::error::Error &e) -> void { this->onChannelError(e); });
  channel_->sendVideoFocusIndication(videoFocusIndication, std::move(promise));
}

VideoService::~VideoService() noexcept {
  LOG(DEBUG) << "Video Service Destroyed";
}

}
