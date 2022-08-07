#pragma once

#include <vector>
#include <functional>
#include <mutex>
#include <asio/io_service.hpp>
#include <asio/basic_waitable_timer.hpp>
#include "autoapp/Managers/INightManager.hpp"


class NightManager: public INightManager{
 public:
  NightManager();
  ~NightManager() override = default;

  void start() override;
  void stop() override;
};