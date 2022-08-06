#pragma once

#include <autoapp/Managers/IManager.hpp>
#include <aasdk_proto/GPSLocationData.pb.h>


class IGPSManager: public IManager{
 public:
  using Pointer = std::shared_ptr<IGPSManager>;
  using locationCallback = std::function<void(aasdk::proto::data::GPSLocation)>;

 private:
  std::vector<locationCallback> locationCallbacks;

 protected:
  int delay = 1000;
 public:
  IGPSManager() = default;
  virtual ~IGPSManager() = default;

  virtual void start() = 0;
  virtual void stop() = 0;


  void registerLocationCallback(const locationCallback &callback) {
    locationCallbacks.emplace_back(callback);
  };

  void updateLocation(aasdk::proto::data::GPSLocation locationData) {
    for (auto &callback : locationCallbacks) {
      callback(locationData);
    }
  }

};