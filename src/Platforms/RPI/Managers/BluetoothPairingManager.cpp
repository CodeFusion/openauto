#include <iostream>
#include <iomanip>

#include "easylogging++.h"

#include <Platforms/RPI/Managers/BluetoothPairingManager.hpp>


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
  return "00:00:00:00:00:00";
}

void BluetoothPairingManager::pairingRequest(std::string  /*mac*/, aasdk::io::Promise<void>::Pointer promise){
  promise->resolve();
}
