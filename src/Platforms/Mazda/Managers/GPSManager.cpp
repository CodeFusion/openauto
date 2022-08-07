#include "Platforms/Mazda/Managers/GPSManager.hpp"
#include "easylogging++.h"

void GPSManager::update_position(const asio::error_code &error) {
  std::lock_guard<std::mutex> lock(timerMutex);
  if (error == asio::error::operation_aborted) {
    return;
  }

  aasdk::proto::data::GPSLocation loc;
  try {
    std::tuple<int32_t, uint64_t, double, double, int32_t, double, double, double, double>
        data = gpsclient->getcom_jci_lds_dataInterface()->GetPosition();

    //timestamp 0 means "invalid" and positionAccuracy 0 means "no lock"
    if (std::get<1>(data) == 0 || std::get<0>(data) == 0) {
      std::time_t now = time(nullptr);
      if ((now - lastError) > 60 || lastError == 0) {
        LOG(DEBUG) << "No GPS Fix";
        lastError = now;
      }
      errorCount++;
    } else {
      errorCount = 0;
      timeval currentTime{};
      gettimeofday(&currentTime, nullptr);
      time_t timestamp = (time_t) std::get<1>(data) * 1000000 + currentTime.tv_usec;

      loc.set_timestamp(static_cast<uint64_t>(timestamp));
      loc.set_latitude(static_cast<int32_t>(std::get<2>(data) * 1E7));
      loc.set_longitude(static_cast<int32_t>(std::get<3>(data) * 1E7));
      loc.set_accuracy(static_cast<unsigned int>(std::get<7>(data) * 1E3));
      loc.set_altitude(static_cast<int32_t>(std::get<4>(data) * 1E2));
      loc.set_speed(static_cast<int32_t>((std::get<6>(data) * 0.277778) * 1E3));
      loc.set_bearing(static_cast<int32_t>(std::get<5>(data) * 1E6));
      updateLocation(loc);
    }
  }
  catch (DBus::Error &error) {
    LOG(ERROR) << "DBUS: GetPosition failed " << error.name() << ": " << error.message();
  }
  timer.expires_after(std::chrono::milliseconds(delay));
  timer.async_wait([this](const asio::error_code &error) { this->update_position(error); });
}

GPSManager::GPSManager(asio::io_service &IoService, const std::shared_ptr<DBus::Connection> &system_connection)
    : timer(IoService), errorCount(0), lastError(0) {

  gpsclient = com_jci_lds_data_objectProxy::create(system_connection, "com.jci.lds.data", "/com/jci/lds/data");
  gpscontrol =
      com_jci_lds_control_objectProxy::create(system_connection, "com.jci.lds.control", "/com/jci/lds/control");

}

void GPSManager::start() {
  std::lock_guard<std::mutex> lock(timerMutex);
  try {
    gpscontrol->getcom_jci_lds_controlInterface()->ReadControl(0);
  }
  catch (DBus::Error &error) {
    LOG(ERROR) << "DBUS: ReadControl failed " << error.name() << " : " << error.message();
  }
  timer.expires_after(std::chrono::milliseconds(delay));
  timer.async_wait([this](const asio::error_code &error) { this->update_position(error); });
}

void GPSManager::stop() {
  std::lock_guard<std::mutex> lock(timerMutex);
  LOG(DEBUG) << "Stopping GPSManager";
  try {
    gpscontrol->getcom_jci_lds_controlInterface()->ReadControl(1);
  }
  catch (DBus::Error &error) {
    LOG(ERROR) << "DBUS: ReadControl failed " << error.name() << " : " << error.message();
  }
  timer.cancel();
}
