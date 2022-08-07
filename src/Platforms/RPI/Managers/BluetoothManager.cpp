#include "Platforms/RPI/Managers/BluetoothManager.hpp"

#include "easylogging++.h"
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

BluetoothManager::BluetoothManager(){

}

BluetoothManager::~BluetoothManager() = default;

void BluetoothManager::start() {
}

void BluetoothManager::stop() {
}