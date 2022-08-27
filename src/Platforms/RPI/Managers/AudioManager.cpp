#include "Platforms/RPI/Managers/AudioManager.hpp"

AudioManager::AudioManager() {

}

void AudioManager::requestFocus(aasdk::messenger::ChannelId channelId,
                                aasdk::proto::enums::AudioFocusType_Enum aa_type) {
  switch (channelId) {
    case aasdk::messenger::ChannelId::MEDIA_AUDIO:
      updateFocus(channelId, AudioFocusState::GAIN);
      break;
    case aasdk::messenger::ChannelId::SPEECH_AUDIO:
//            updateFocus(stream->channelId, AudioFocusState::GAIN_TRANSIENT_GUIDANCE_ONLY);
//            break;
    case aasdk::messenger::ChannelId::SYSTEM_AUDIO:
      updateFocus(channelId, AudioFocusState::GAIN_TRANSIENT);
      break;
    default:
      break;
  }
}

void AudioManager::releaseFocus(aasdk::messenger::ChannelId channelId) {
  updateFocus(channelId, AudioFocusState::LOSS);
}

void AudioManager::start() {

}

void AudioManager::stop() {
}

IAudioManager::focusType AudioManager::getFocusType(aasdk::messenger::ChannelId channelId) {
  if (channelId == aasdk::messenger::ChannelId::MEDIA_AUDIO) {
    return IAudioManager::focusType::NORMAL;
  }
  return IAudioManager::focusType::TRANSIENT;
}
