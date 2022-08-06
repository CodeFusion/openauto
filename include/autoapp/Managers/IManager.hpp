#pragma once

#include <memory>

class IManager {
 public:
  using Pointer = std::shared_ptr<IManager>;

  IManager() = default;
  virtual ~IManager() = default;

  virtual void start() = 0;
  virtual void stop() = 0;

};