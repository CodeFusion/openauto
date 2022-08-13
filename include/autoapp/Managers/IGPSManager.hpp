#pragma once

#include <autoapp/Managers/IManager.hpp>
#include <aasdk_proto/GPSLocationData.pb.h>


class IGPSManager: public IManager{
 public:
  using Pointer = std::shared_ptr<IGPSManager>;
  using locationCallback = std::function<void(aasdk::proto::data::GPSLocation)>;

 private:
  std::vector<locationCallback> locationCallbacks;

 public:
  IGPSManager() = default;
  ~IGPSManager() override = default;

  void registerLocationCallback(const locationCallback &callback) {
    locationCallbacks.emplace_back(callback);
  };

  void updateLocation(const aasdk::proto::data::GPSLocation& locationData) {
    for (auto &callback : locationCallbacks) {
      callback(locationData);
    }
  }

};