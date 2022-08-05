#pragma once

#include <memory>
#include <utility>
#include <sigc++/sigc++.h>

#include "AudioSignals.hpp"
#include "GpsSignals.hpp"
#include "AASignals.hpp"
#include "NavigationSignals.hpp"
#include "autoapp/Managers/IVideoManager.hpp"

class Signals : public sigc::trackable {
 public:
  typedef std::shared_ptr<Signals> Pointer;

  IVideoManager::Pointer videoManager;
  AudioSignals::Pointer audioSignals = std::make_shared<AudioSignals>();
  GpsSignals::Pointer gpsSignals = std::make_shared<GpsSignals>();
  AASignals::Pointer aaSignals;
  NavigationSignals::Pointer navSignals = std::make_shared<NavigationSignals>();

  explicit Signals(IVideoManager::Pointer VideoManager, AASignals::Pointer AaSignals) : videoManager(std::move(
      VideoManager)), aaSignals(std::move(AaSignals)) {
  };
};