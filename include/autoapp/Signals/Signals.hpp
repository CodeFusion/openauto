#pragma once

#include <memory>
#include <utility>
#include <sigc++/sigc++.h>

#include "AASignals.hpp"
#include "NavigationSignals.hpp"
#include "autoapp/Managers/IVideoManager.hpp"
#include "autoapp/Managers/IGPSManager.hpp"
#include <autoapp/Managers/INightManager.hpp>
#include <autoapp/Managers/IAudioManager.hpp>

class Signals : public sigc::trackable {
 public:
  typedef std::shared_ptr<Signals> Pointer;

  IVideoManager::Pointer videoManager;
  IAudioManager::Pointer audioManager;
  IGPSManager::Pointer gpsManager;
  AASignals::Pointer aaSignals;
  NavigationSignals::Pointer navSignals = std::make_shared<NavigationSignals>();
  INightManager::Pointer nightManager;

  explicit Signals(IVideoManager::Pointer VideoManager,
                   IAudioManager::Pointer AudioManager,
                   IGPSManager::Pointer GPSManager,
                   AASignals::Pointer AaSignals,
                   INightManager::Pointer NightManager) :
      videoManager(std::move(VideoManager)),
      audioManager(std::move(AudioManager)),
      gpsManager(std::move(GPSManager)),
      aaSignals(std::move(AaSignals)),
      nightManager(std::move(NightManager)) {
  };
};