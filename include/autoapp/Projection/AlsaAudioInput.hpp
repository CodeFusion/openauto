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

#pragma once

#include <memory>
#include <aasdk/IO/Promise.hpp>
#include <aasdk/Common/Data.hpp>
#include <autoapp/Projection/IAudioInput.hpp>

#include <cstdio>
#include <alsa/asoundlib.h>
#include <thread>

namespace autoapp::projection {
class AlsaAudioInput : public IAudioInput {
  std::string micDevice;

 public:
  using StartPromise = aasdk::io::Promise<void, void>;
  using ReadPromise = aasdk::io::Promise<aasdk::common::Data, void>;

  explicit AlsaAudioInput(asio::io_service &ioService, std::string micDevice = "default");

  ~AlsaAudioInput() override;

  bool open() override;

  bool isActive() const override;

  void read(ReadPromise::Pointer error_code) override;

  void start(StartPromise::Pointer promise) override;

  void stop() override;

  uint32_t getSampleSize() const override { return sampleSize; };

  uint32_t getChannelCount() const override { return channels; };

  uint32_t getSampleRate() const override { return sampleRate; };

 private:
  asio::io_service &ioService_;
  ReadPromise::Pointer readPromise_;
  mutable std::mutex mutex_;
  snd_pcm_t *pcm_handle = nullptr;
  asio::posix::stream_descriptor *sd = nullptr;
  const uint32_t sampleSize = 16;
  const uint32_t channels = 1;
  const uint32_t sampleRate = 16000;
  const uint32_t latency = 500000; //in microseconds
  const snd_pcm_uframes_t buffer_size = 256;
  const snd_pcm_uframes_t period_size = 16;

  void handler(asio::error_code l_error_code);
};

}