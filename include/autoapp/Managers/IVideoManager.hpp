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

  virtual void requestFocus() = 0;
  virtual void releaseFocus() = 0;

  void registerFocus(const focusCallback &callback) {
    focusCallbacks.emplace_back(callback);
  };

  void focusChanged(bool hasFocus) {
    for (auto &callback : focusCallbacks) {
      callback(hasFocus);
    }
  }

};