#include <easylogging++.h>
#include <autoapp/Projection/AlsaAudioInput.hpp>
#include <utility>

namespace autoapp::projection {
void AlsaAudioInput::read(ReadPromise::Pointer promise) {
  std::lock_guard<decltype(mutex_)> lock(mutex_);

  if (readPromise_ != nullptr || sd == nullptr) {
    promise->reject();
  } else {
    readPromise_ = std::move(promise);
    sd->async_wait(asio::posix::stream_descriptor::wait_read,
                   [this](asio::error_code error_code) { this->handler(error_code); });
  }
}

void AlsaAudioInput::handler(asio::error_code error_code) {
  std::lock_guard<decltype(mutex_)> lock(mutex_);

  VLOG(9) << "AlsaAudioInput::handler called";

  if (snd_pcm_state(pcm_handle) != SND_PCM_STATE_RUNNING || error_code == asio::error::operation_aborted) {
    VLOG(9) << "mic handler stopped";
    return;
  }

  snd_pcm_sframes_t avail = snd_pcm_avail_update(pcm_handle);
  VLOG(9) << "Avail: " << avail;
  if (avail > 0) {
    auto readSize = snd_pcm_frames_to_bytes(pcm_handle, avail);
    VLOG(9) << "Read:  " << readSize;
    auto *tempBuffer = new uint8_t[readSize];
    snd_pcm_sframes_t frames = snd_pcm_readi(pcm_handle, tempBuffer, static_cast<snd_pcm_uframes_t>(avail));
    if (frames < 0) {
      LOG(ERROR) << "Mic read error: " << snd_strerror(frames);
    }
    avail = snd_pcm_avail_update(pcm_handle);
    aasdk::common::Data data(readSize, 0);
    data.reserve(static_cast<unsigned int>(readSize));
    data.insert(data.begin(), tempBuffer, tempBuffer + readSize);

    if (readSize != -1) {
      data.resize(static_cast<unsigned int>(readSize));
      readPromise_->resolve(std::move(data));
      readPromise_.reset();
    } else {
      readPromise_->reject();
      readPromise_.reset();
    }
    VLOG(9) << "left: " << avail;

  } else if (avail < 0 || sd == nullptr) {
    LOG(ERROR) << "Mic read error: " << snd_strerror(avail);
  } else {
    sd->async_wait(asio::posix::stream_descriptor::wait_read,
                   [this](asio::error_code l_error_code) { this->handler(l_error_code); });
  }
}

AlsaAudioInput::AlsaAudioInput(asio::io_service &ioService, std::string micDevice)
    : micDevice(std::move(micDevice)), ioService_(ioService) {
}

AlsaAudioInput::~AlsaAudioInput() = default;

void AlsaAudioInput::start(StartPromise::Pointer promise) {
  int err;
  if (pcm_handle == nullptr) {

    if ((err = snd_pcm_open(&pcm_handle, micDevice.c_str(), SND_PCM_STREAM_CAPTURE, 0)) < 0) {
      LOG(ERROR) << "Playback open error: " << snd_strerror(err);
      promise->reject();
      return;
    }

    if ((err = snd_pcm_set_params(pcm_handle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED,
                                  getChannelCount(), getSampleRate(), 1, latency)) < 0) {
      LOG(ERROR) << "Mic open error: " << snd_strerror(err);
      promise->reject();
      return;
    }

    snd_pcm_sw_params_t *sw_params;

    snd_pcm_sw_params_malloc(&sw_params);
    snd_pcm_sw_params_current(pcm_handle, sw_params);

    snd_pcm_sw_params_set_start_threshold(pcm_handle, sw_params, buffer_size - period_size);
    snd_pcm_sw_params_set_avail_min(pcm_handle, sw_params, period_size);

    if ((err = snd_pcm_sw_params(pcm_handle, sw_params)) < 0) {   /* 0.25sec */
      LOG(ERROR) << "Mic SW config error: " << snd_strerror(err);
      snd_pcm_close(pcm_handle);
      promise->reject();
      return;
    }

    snd_pcm_sw_params_free(sw_params);

    struct pollfd *ufds;
    int count;
    count = snd_pcm_poll_descriptors_count(pcm_handle);
    if (count <= 0) {
      LOG(ERROR) << "Invalid poll descriptors count";
      promise->reject();
      return;
    }
    ufds = (pollfd *) malloc(sizeof(struct pollfd) * count);
    if (ufds == nullptr) {
      LOG(ERROR) << "No enough memory";
      promise->reject();
      return;
    }

    if ((err = snd_pcm_poll_descriptors(pcm_handle, ufds, static_cast<unsigned int>(count))) < 0) {
      LOG(ERROR) << "Unable to obtain poll descriptors for recording: " << snd_strerror(err);
      free(ufds);
      promise->reject();
      return;
    }
    LOG(DEBUG) << "Pollfd count: " << count;
    sd = new asio::posix::stream_descriptor(ioService_, ufds[0].fd);
    free(ufds);
  }

  if ((err = snd_pcm_prepare(pcm_handle)) < 0) {
    LOG(ERROR) << "snd_pcm_prepare err: " << snd_strerror(err);
    promise->reject();
    return;
  }

  if ((err = snd_pcm_start(pcm_handle)) < 0) {
    LOG(ERROR) << "snd_pcm_start err: " << snd_strerror(err);
    promise->reject();
    return;
  }
  promise->resolve();
}

void AlsaAudioInput::stop() {
  if (sd != nullptr) {
    sd->cancel();
  }
  if (pcm_handle != nullptr) {
    snd_pcm_drop(pcm_handle);
    snd_pcm_close(pcm_handle);
    pcm_handle = nullptr;
  }
  if (readPromise_ != nullptr) {
    readPromise_->reject();
    readPromise_.reset();
  }
}

bool AlsaAudioInput::open() {
  return true;
}

bool AlsaAudioInput::isActive() const {
  return true;
}
}