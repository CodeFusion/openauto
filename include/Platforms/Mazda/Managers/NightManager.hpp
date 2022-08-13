#pragma once

#include <vector>
#include <functional>
#include <mutex>
#include <asio/io_service.hpp>
#include <asio/basic_waitable_timer.hpp>
#include "autoapp/Managers/INightManager.hpp"


class NightManager: public INightManager{
 public:
  explicit NightManager(asio::io_service &IoService );
  ~NightManager() override = default;

  void start() override;
  void stop() override;


 private:
  asio::basic_waitable_timer<std::chrono::steady_clock> timer;
  std::mutex timerMutex;
  const int delay = 1000;

  void checkNight(const asio::error_code &error);

};