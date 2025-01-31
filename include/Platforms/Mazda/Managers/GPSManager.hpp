#pragma once

#include <ctime>
#include "autoapp/Managers/IGPSManager.hpp"

#include <com_jci_lds_data_objectProxy.h>
#include <com_jci_lds_control_objectProxy.h>
#include <asio/io_service.hpp>
#include <asio/basic_waitable_timer.hpp>

class GPSManager: public IGPSManager{

 public:
  GPSManager(asio::io_service &IoService, const std::shared_ptr<DBus::Connection> &system_connection);

  ~GPSManager() override = default;

  void start() override;
  void stop() override;


 private:
  using gpsData = std::tuple<int32_t, uint64_t, double, double, int32_t, double, double, double, double>;
  asio::basic_waitable_timer<std::chrono::steady_clock> timer;
  std::shared_ptr<com_jci_lds_data_objectProxy> gpsclient;
  std::shared_ptr<com_jci_lds_control_objectProxy> gpscontrol;
  std::mutex timerMutex;

  int errorCount;
  std::time_t lastError;
  const int delay = 1000;

  void update_position(const asio::error_code &error);

};