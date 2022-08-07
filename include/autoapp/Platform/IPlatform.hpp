#pragma once

#include "autoapp/Signals/Signals.hpp"

class IPlatform {
 public:
  using Pointer = std::shared_ptr<IPlatform>;

  IPlatform() = default;
  virtual ~IPlatform() = default;

  Signals::Pointer signals;


};