#pragma once


#include <atomic>
#include <set>
#include "autoapp/Managers/IVideoManager.hpp"


class VideoManager: public IVideoManager{
 public:
  VideoManager();
  ~VideoManager() override;

  void start() override;
  void stop() override;

  void requestFocus() override ;
  void releaseFocus() override ;
};