#pragma once

#include "autoapp/Signals/Signals.hpp"

class IPlatform {
 public:
  using Pointer = std::shared_ptr<IPlatform>;

  IPlatform() = default;
  virtual ~IPlatform() = default;

  Signals::Pointer signals;

  /// Called when Android Auto connection starts
  virtual void start() = 0;

  /// Called when Android Auto connection stops
  virtual void stop() = 0;


};