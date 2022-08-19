#include <iostream>
#include <iomanip>

#include "easylogging++.h"

#include <Platforms/Mazda/Managers/BluetoothPairingManager.hpp>


BluetoothPairingManager::BluetoothPairingManager()= default;

// We can retrieve MAC Address from Mazda CMU from two sys paths:
//    /sys/fsl_otp/HW_OCOTP_MAC1 for the begining
//    /sys/fsl_otp/HW_OCOTP_MAC0 for the second part
// The beginning returns first two bytes of MAC, the second part the rest of MAC address as a HEX string.
// Example:
// echo /sys/fsl_otp/HW_OCOTP_MAC1 -> 0x88c6
// echo /sys/fsl_otp/HW_OCOTP_MAC0 -> 0x261b82f2
// Actual MAC: 88:c6:26:1b:82:f2

#define SHIFT_AND_MASK(value, shift)   (((value) >> (shift)) & mask)

std::string BluetoothPairingManager::getMac() {


  std::ifstream macUpperFile("/sys/fsl_otp/HW_OCOTP_MAC1");
  std::ifstream macLowerFile("/sys/fsl_otp/HW_OCOTP_MAC0");

  std::stringstream macUpper;
  std::stringstream macLower;

  macUpper << macUpperFile.rdbuf();
  macLower << macLowerFile.rdbuf();

  // Check for I/O error
  if (macUpperFile.fail() || macLowerFile.fail()) {
    return "";
  }

  uint32_t macAddrUpper = std::stoul(macUpper.str(), nullptr, 16);
  uint32_t macAddrLower = std::stoul(macLower.str(), nullptr, 16);

  const uint8_t mask = 0xFF;

  std::stringstream macStream;
  macStream << std::setw(2) << std::setfill('0') << std::uppercase << std::hex;
  macStream << SHIFT_AND_MASK(macAddrUpper, 8);
  macStream << ":";
  macStream << SHIFT_AND_MASK(macAddrUpper, 0);
  macStream << ":";
  macStream << SHIFT_AND_MASK(macAddrLower, 24);
  macStream << ":";
  macStream << SHIFT_AND_MASK(macAddrLower, 16);
  macStream << ":";
  macStream << SHIFT_AND_MASK(macAddrLower, 8);
  macStream << ":";
  macStream << SHIFT_AND_MASK(macAddrLower, 0) + 1;  // BT address really always +1 from the base one in file

  LOG(DEBUG) << macStream.str();
  return macStream.str();
}

void BluetoothPairingManager::pairingRequest(std::string  /*mac*/, aasdk::io::Promise<void>::Pointer promise){
  promise->resolve();
}
