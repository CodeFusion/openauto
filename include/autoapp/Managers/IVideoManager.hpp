#pragma once

#include <vector>
#include <functional>
#include <autoapp/Managers/IManager.hpp>

class IVideoManager: public IManager{
 public:
  using Pointer = std::shared_ptr<IVideoManager>;
  using focusCallback = std::function<void(bool)>;

 private:
  std::vector<focusCallback> focusCallbacks;

 public:
  IVideoManager() = default;
  virtual ~IVideoManager() = default;

  virtual void start() = 0;
  virtual void stop() = 0;

  /// Requests video focus from the VideoManager
  virtual void requestFocus() = 0;

  /// Releases video focus
  virtual void releaseFocus() = 0;

  /// Register for video focus change events
  /// \param callback to be called when focus changes
  void registerFocus(const focusCallback &callback) {
    focusCallbacks.emplace_back(callback);
  };

  /// Sends video focus status to all listeners
  /// \param hasFocus true if video in focus, false otherwise
  void focusChanged(bool hasFocus) {
    for (auto &callback : focusCallbacks) {
      callback(hasFocus);
    }
  }

};