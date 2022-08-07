#pragma once

#include <ctime>
#include "autoapp/Managers/IGPSManager.hpp"

class GPSManager: public IGPSManager{

 public:
  GPSManager();

  ~GPSManager() = default;

  void start() override;
  void stop() override;

};