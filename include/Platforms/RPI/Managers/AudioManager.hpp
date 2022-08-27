#include <set>

#include "autoapp/Managers/IAudioManager.hpp"

#include <thread>
#include <mutex>

using AudioFocusState = aasdk::proto::enums::AudioFocusState;

class AudioManager : public IAudioManager {
 private:
 public:
  AudioManager();

  ~AudioManager() override = default;

  void start() override;
  void stop() override;

  //calling requestAudioFocus directly doesn't work on the audio mgr
  void requestFocus(aasdk::messenger::ChannelId channelId, aasdk::proto::enums::AudioFocusType_Enum aa_type) override;
  void releaseFocus(aasdk::messenger::ChannelId channelId) override;

  focusType getFocusType(aasdk::messenger::ChannelId channelId) override;


};