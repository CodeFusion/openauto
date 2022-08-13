/*
*  This file is part of openauto project.
*  Copyright (C) 2018 f1x.studio (Michal Szwaj)
*
*  openauto is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 3 of the License, or
*  (at your option) any later version.

*  openauto is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with openauto. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <autoapp/Projection/IBluetoothDevice.hpp>
#include <aasdk/IO/Promise.hpp>

namespace autoapp::projection {
class MazdaBluetooth : public IBluetoothDevice {
 public:
  using PairingPromise = aasdk::io::Promise<void, void>;

  void stop() override;

  [[nodiscard]] bool isPaired(const std::string &address) const override;

  void pair(const std::string &address, PairingPromise::Pointer promise) override;

  [[nodiscard]] std::string getLocalAddress() const override;

  [[nodiscard]] bool isAvailable() const override;
};
}



