#pragma once

#include <memory>
#include <sigc++/sigc++.h>

class AASignals : public sigc::trackable {
 public:
  typedef std::shared_ptr<AASignals> Pointer;
  sigc::signal<void(bool)> connected;
};