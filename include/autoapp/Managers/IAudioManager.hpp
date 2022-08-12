#pragma once

#include <vector>
#include <autoapp/Managers/IManager.hpp>
#include <aasdk_proto/AudioFocusStateEnum.pb.h>
#include <aasdk_proto/AudioFocusTypeEnum.pb.h>
#include <aasdk/Messenger/ChannelId.hpp>
#include <asio.hpp>
#include <aasdk/IO/Promise.hpp>



class IAudioManager: public IManager{
 public:
  using Pointer = std::shared_ptr<IAudioManager>;
  using focusCallback = std::function<void(aasdk::messenger::ChannelId channelId, aasdk::proto::enums::AudioFocusState_Enum focus)>;

 private:
  std::vector<focusCallback> focusCallbacks;

 public:
  IAudioManager() = default;
  virtual ~IAudioManager() = default;

  virtual void start() = 0;
  virtual void stop() = 0;

  /// Request the given audio focus type for the given channel id
  /// \param channelId AASDK channel ID of affected channel
  /// \param focus type of audio focus requested
  virtual void requestFocus(aasdk::messenger::ChannelId channelId, aasdk::proto::enums::AudioFocusType_Enum focus, aasdk::io::Promise<void>::Pointer promise) = 0;

  /// Release the audio focus for the channel ID provided. Release all if channel id is aasdk::messenger::ChannelID::None
  /// \param channelId AASDK channel ID of affected channel
  virtual void releaseFocus(aasdk::messenger::ChannelId channelId) = 0;

  /// Register callback for audio focus changes
  /// \param callback function to be called on audio focus changes
  void registerFocusCallback(const focusCallback &callback) {
    focusCallbacks.emplace_back(callback);
  };

  /// Call audio focus callbacks to notify of audio focus changes
  /// \param channelId AASDK channel ID of affected channel
  /// \param focus Audio focus state
  void updateFocus(aasdk::messenger::ChannelId channelId, aasdk::proto::enums::AudioFocusState_Enum focus) {
    for (auto &callback : focusCallbacks) {
      callback(channelId, focus);
    }
  }

};