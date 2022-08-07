#include "Platforms/RPI/Managers/AudioManager.hpp"

AudioManager::AudioManager() {

}

AudioManager::~AudioManager() {

}

void AudioManager::requestFocus(aasdk::messenger::ChannelId channelId,
                                aasdk::proto::enums::AudioFocusType_Enum aa_type) {
  updateFocus(channelId, AudioFocusState::GAIN);
}

void AudioManager::releaseFocus(aasdk::messenger::ChannelId channelId) {
  updateFocus(channelId, AudioFocusState::LOSS);
}

void AudioManager::start() {

}

void AudioManager::stop() {
}
