#include <autoapp/Managers/NightManager.hpp>
#include <easylogging++.h>

NightManager::NightManager(asio::io_service &IoService) : timer(IoService) {

};

void NightManager::start() {
  std::lock_guard<std::mutex> lock(timerMutex);
  LOG(DEBUG) << "Starting NightManager";

  timer.expires_after(std::chrono::milliseconds(delay));
  timer.async_wait([this](const asio::error_code &error) { this->checkNight(error); });
}

void NightManager::stop() {
  std::lock_guard<std::mutex> lock(timerMutex);
  LOG(DEBUG) << "Stopping NightManager";
  timer.cancel();
}

void NightManager::checkNight(const asio::error_code &error) {
  std::lock_guard<std::mutex> lock(timerMutex);
  if (error == asio::error::operation_aborted) {
    return;
  }

  bool nightmodenow = false;
  std::array<char, 3> gpio_value{};
  FILE *fileDescriptor = fopen("/sys/class/gpio/CAN_Day_Mode/value", "r");
  if (fileDescriptor == nullptr) {
    LOG(ERROR) << "Failed to open CAN_Day_Mode gpio value for reading";
  } else {
    size_t r = fread(&gpio_value, 1, 2, fileDescriptor);
    if (r == 2) {
      nightmodenow = (gpio_value[0] == '0');
    } else {
      LOG(ERROR) << "Read CAN_Day_Mode gpio value failed";
    }
  }
  fclose(fileDescriptor);

  updateNight(nightmodenow);

  timer.expires_after(std::chrono::milliseconds(delay));
  timer.async_wait([this](const asio::error_code &error) { this->checkNight(error); });

}
