#include <autoapp/Projection/AlsaAudio.hpp>
#include <easylogging++.h>

namespace autoapp::projection {

AlsaAudioOutput::AlsaAudioOutput(unsigned int channels, unsigned int rate, const char *outDev) {
  _channels = channels;
  _rate = rate;
  deviceName.assign(outDev);
  LOG(INFO) << "snd_asoundlib_version: " << snd_asoundlib_version();
  LOG(INFO) << "Device name " << outDev;
  int err;
  if ((err = snd_pcm_open(&aud_handle, outDev, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    LOG(ERROR) << "Playback open error: " << snd_strerror(err);
  }
}

bool AlsaAudioOutput::open() {
  int err;
  if ((err = snd_pcm_set_params(aud_handle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED,
                                _channels, _rate, 1, latency)) < 0) {
    LOG(ERROR) << "Playback open error: " << snd_strerror(err);
    return false;
  }
  if ((err = snd_pcm_prepare(aud_handle)) < 0) {
    LOG(ERROR) << "snd_pcm_prepare error: " << snd_strerror(err);
    return false;
  }

  // Initialize AAC Decoder for our AAC ADTS Stream
  decoder = aacDecoder_Open(TT_MP4_ADTS, 1);

  return true;
}

void AlsaAudioOutput::stop() {

}

void AlsaAudioOutput::write(__attribute__((unused)) aasdk::messenger::Timestamp::ValueType timestamp,
                            const aasdk::common::DataConstBuffer &buffer) {
  int ret;
  if (buffer.size > 2) {

    //Allocate our output buffer. The largest it should be is (1024*16*2)/8 or (PCM Frames * 16bit * 2 channels)/8
    const int outBufSize = 4096;
    auto *outBuf = static_cast<short int *>(malloc(outBufSize));

    //Set up the nessesary variables for aacDecoder_Fill()
    unsigned int bytesLeft = buffer.size;
    auto *bufPtr = const_cast<unsigned char *>(buffer.cdata);


    //Fill the decoder buffer with one audio packet
    ret = aacDecoder_Fill(decoder, &bufPtr, &buffer.size, &bytesLeft);
    VLOG(9) << "aacDecoder_Fill bytes left " << bytesLeft;
    if (ret != 0) {
      LOG(ERROR) << "aacDecoder_Fill failed with " << ret;
    }

    //Decode the PCM audio frames from the AAC bitstream. This should return either 960 or 1024 PCM frames
    ret = aacDecoder_DecodeFrame(decoder, outBuf, outBufSize, 0);
    if (ret != 0) {
      LOG(ERROR) << "aacDecoder_DecodeFrame failed with " << ret;
    } else {
      CStreamInfo *streaminfo = aacDecoder_GetStreamInfo(decoder);

      //Write the data to the ALSA buffer for playback
      snd_pcm_sframes_t frames = snd_pcm_writei(aud_handle, outBuf, streaminfo->frameSize);
      if (frames < 0) {
        LOG(ERROR) << deviceName <<  " snd_pcm_writei:  " << snd_strerror(frames);
        frames = snd_pcm_recover(aud_handle, frames, 1);
        if (frames < 0) {
          LOG(ERROR) << "snd_pcm_recover failed: " << snd_strerror(frames);
        } else {
          frames = snd_pcm_writei(aud_handle, outBuf, streaminfo->frameSize);
        }
      }
      if (frames >= 0 && frames < streaminfo->frameSize) {
        LOG(ERROR) << "Short write (expected " << (int) streaminfo->frameSize
                   << ", wrote " << (int) frames;
      }
    }

    free(outBuf);
  }
}

AlsaAudioOutput::~AlsaAudioOutput() {
  snd_pcm_close(aud_handle);
}
}