#pragma once

#include <vector>
#include <autoapp/Managers/IManager.hpp>
#include <aasdk_proto/AudioFocusStateEnum.pb.h>
#include <aasdk_proto/AudioFocusTypeEnum.pb.h>
#include <aasdk/Messenger/ChannelId.hpp>



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

  virtual void requestFocus(aasdk::messenger::ChannelId channelId, aasdk::proto::enums::AudioFocusType_Enum focus) = 0;
  virtual void releaseFocus(aasdk::messenger::ChannelId channelId) = 0;


  void registerFocusCallback(const focusCallback &callback) {
    focusCallbacks.emplace_back(callback);
  };

  void updateFocus(aasdk::messenger::ChannelId channelId, aasdk::proto::enums::AudioFocusState_Enum focus) {
    for (auto &callback : focusCallbacks) {
      callback(channelId, focus);
    }
  }

};