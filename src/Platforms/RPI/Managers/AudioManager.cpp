#include "Platforms/RPI/Managers/AudioManager.hpp"

AudioManager::AudioManager() {

}

void AudioManager::requestFocus(aasdk::messenger::ChannelId channelId,
                                aasdk::proto::enums::AudioFocusType_Enum aa_type,  aasdk::io::Promise<void>::Pointer promise) {
  promise->resolve();
}

void AudioManager::releaseFocus(aasdk::messenger::ChannelId channelId) {
  updateFocus(channelId, AudioFocusState::LOSS);
}

void AudioManager::start() {

}

void AudioManager::stop() {
}
