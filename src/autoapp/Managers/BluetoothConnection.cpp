#include <autoapp/Managers/BluetoothConnection.hpp>
#include <utility>
#include "aasdk_proto/WifiInfoResponseMessage.pb.h"
#include "aasdk_proto/WifiSecurityResponseMessage.pb.h"

BluetoothConnection::BluetoothConnection(std::string SSID,
                                         std::string Password,
                                         std::string IpAddress,
                                         std::string MacAddress,
                                         uint32_t Port)
    : ipAddress(std::move(IpAddress)),
      macAddress(std::move(MacAddress)),
      ssid(std::move(SSID)),
      password(std::move(Password)),
      port(Port) {
  LOG(DEBUG) << "Got IP: " << ipAddress << " MAC: " << macAddress;
}

void BluetoothConnection::handleWifiInfoRequest() {
  aasdk::proto::messages::WifiInfoResponse response;
  response.set_ip_address(ipAddress.c_str());
  response.set_port(port);
  response.set_status(aasdk::proto::messages::WifiInfoResponse_Status_STATUS_SUCCESS);

  sendMessage(response, IBluetoothManager::wifiMessages::WifiInfoRequestResponse);
}

void BluetoothConnection::handleWifiSecurityRequest(__attribute__((unused)) uint8_t *buffer,
                                                    __attribute__((unused)) uint16_t length) {
  aasdk::proto::messages::WifiSecurityReponse response;

  response.set_ssid(ssid.c_str());
  response.set_bssid(macAddress.c_str());
  response.set_key(password.c_str());
  response.set_security_mode(aasdk::proto::messages::WifiSecurityReponse_SecurityMode_WPA2_PERSONAL);
  response.set_access_point_type(aasdk::proto::messages::WifiSecurityReponse_AccessPointType_DYNAMIC);

  sendMessage(response, IBluetoothManager::wifiMessages::WifiSecurityReponse);
}

int BluetoothConnection::handleWifiInfoRequestResponse(uint8_t *buffer, uint16_t length) {
  aasdk::proto::messages::WifiInfoResponse msg;
  msg.ParseFromArray(buffer, length);
  LOG(DEBUG) << "WifiInfoResponse: " << msg.DebugString();
  return msg.status();
}
void BluetoothConnection::messageHandler() {
  handleWifiInfoRequest();

  const ssize_t bufferLength = 100;
  std::array<char, bufferLength> buffer{};
  uint16_t msgLen;
  uint16_t msg;

  ssize_t bufferPos = 0;
  while (true) {
    ssize_t readCount = read(buffer.begin() + bufferPos, buffer.size() - bufferPos);
    if (readCount == 0) {
      break;
    }
    bufferPos += readCount;
    if (bufferPos < 4) {
      continue;
    }

    msgLen = static_cast<uint16_t>(be16toh(*(uint16_t *) buffer.begin()));
    msg = static_cast<uint16_t>(be16toh(*(uint16_t *) (buffer.begin() + 2)));
    LOG(DEBUG) << "MSG Type: " << msg << " Size: " << msgLen;

    if (static_cast<uint16_t>(bufferPos) < msgLen + 4) {
      continue;
    }

    switch (msg) {
      case IBluetoothManager::wifiMessages::WifiInfoRequest:handleWifiInfoRequest();
        break;
      case IBluetoothManager::wifiMessages::WifiSecurityRequest:
        handleWifiSecurityRequest(reinterpret_cast<uint8_t *>(buffer.begin() + 4),
                                  msgLen);
        break;
      case IBluetoothManager::wifiMessages::WifiInfoRequestResponse:
        handleWifiInfoRequestResponse(reinterpret_cast<uint8_t *>(buffer.begin() + 4), msgLen);
        break;
      default:break;
    }
    bufferPos = 0;
  }
}

void BluetoothConnection::sendMessage(google::protobuf::MessageLite &message, IBluetoothManager::wifiMessages type) {
  auto byteSize = static_cast<size_t>(message.ByteSizeLong());
  uint16_t sizeOut = htobe16(static_cast<uint16_t>(byteSize));
  uint16_t typeOut = htobe16(type);
  auto *out = new char[byteSize + 4];
  memcpy(out, &sizeOut, 2);
  memcpy(out + 2, &typeOut, 2);

  message.SerializeToArray(out + 4, (int)byteSize);

  auto written = write(out, byteSize + 4);
  if (written > -1) {
    LOG(DEBUG) << "Bytes written: " << written;
  } else {
    LOG(DEBUG) << "Could not write data";
  }
  delete[] out;
}
