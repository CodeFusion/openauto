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

#include <cstdio>
#include <alsa/asoundlib.h>
#include <thread>

#include <memory>
#include <aasdk/Messenger/Timestamp.hpp>
#include <aasdk/Common/Data.hpp>
#include <autoapp/Projection/IAudioOutput.hpp>
#include <fdk-aac/aacdecoder_lib.h>
#include <fdk-aac/FDK_audio.h>

namespace autoapp::projection {
class AlsaAudioOutput : public IAudioOutput {
 private:
  snd_pcm_t *aud_handle = nullptr;
  unsigned int _channels = 0;
  unsigned int _rate = 0;
  const uint32_t latency = 500000; //in microseconds
  const uint32_t sampleSize = 16;
  HANDLE_AACDECODER  decoder = nullptr;
  std::string deviceName;
 public:
  AlsaAudioOutput(unsigned int channels, unsigned int rate, const char *outDev = "default");

  ~AlsaAudioOutput() override;

  bool open() override;

  void write(aasdk::messenger::Timestamp::ValueType timestamp,
             const aasdk::common::DataConstBuffer &buffer) override;

  void start() override {};

  void stop() override;

  void suspend() override {};

  [[nodiscard]] uint32_t getSampleSize() const override { return sampleSize; };

  [[nodiscard]] uint32_t getChannelCount() const override { return static_cast<uint32_t>(_channels); }

  [[nodiscard]] uint32_t getSampleRate() const override { return static_cast<uint32_t>(_rate); }
};
}