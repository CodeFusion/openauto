#pragma once
#ifdef BUILD_RPI
#include "autoapp/Platform/IPlatform.hpp"

#include <Platforms/RPI/Managers/VideoManager.hpp>
#include <Platforms/RPI/Managers/AudioManager.hpp>
#include <Platforms/RPI/Managers/GPSManager.hpp>
#include <Platforms/RPI/Managers/NightManager.hpp>
#include <Platforms/RPI/Managers/BluetoothManager.hpp>
#include "autoapp/Configuration/Configuration.hpp"
#include "Platforms/RPI/Managers/BluetoothPairingManager.hpp"
#include "Platforms/RPI/Managers/NavigationManager.hpp"


#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>


class RPI: public IPlatform{
 public:
  explicit RPI(const autoapp::configuration::Configuration::Pointer &configuration);
  ~RPI() override;

  void start() override;
  void stop() override;

};

#endif