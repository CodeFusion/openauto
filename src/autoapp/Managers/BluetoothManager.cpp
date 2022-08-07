#include <autoapp/Managers/BluetoothManager.hpp>

#include <easylogging++.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

BluetoothManager::BluetoothManager(autoapp::configuration::IConfiguration::Pointer configuration,
                                   std::shared_ptr<DBus::Connection> session_connection)
    : configuration_(std::move(configuration)), dbusConnection(std::move(session_connection)) {

}

void BluetoothManager::ConnectionStatusResp(uint32_t found_serviceId, uint32_t connStatus, uint32_t  /*btDeviceId*/,
                                            uint32_t  /*status*/, std::tuple<std::vector<uint8_t>> terminalPath) const {
  LOG(DEBUG) << "Saw Service: " << found_serviceId;
  if (found_serviceId == 1 && connStatus == 3) {
    LOG(DEBUG) << "Saw Service: Handsfree. Inititating connection";
    bcaClient->getcom_jci_bcaInterface()->StartAdd(serviceId);
  } else if (found_serviceId == serviceId && connStatus == 3) {
    std::vector<uint8_t> tpath = std::get<0>(terminalPath);
    std::string pty(tpath.begin(), tpath.end());
    LOG(DEBUG) << "PTY: " << pty;
    BluetoothConnection bconnection(configuration_);
    bconnection.handle_connect(pty);
  }
}

BluetoothManager::~BluetoothManager() = default;

void BluetoothManager::start() {
  LOG(DEBUG) << "Reading BdsConfiguration.xml";

  tinyxml2::XMLDocument doc;
  doc.LoadFile("/jci/bds/BdsConfiguration.xml");

  tinyxml2::XMLNode *docRoot = doc.FirstChild();

  tinyxml2::XMLElement *serviceconfig = docRoot->FirstChildElement("serviceConfiguration");

  if (serviceconfig == nullptr) {
    LOG(DEBUG) << "Couldn't find serviceConfiguration in /jci/bds/BdsConfiguration.xml";
  } else {
    for (tinyxml2::XMLElement *e = serviceconfig->FirstChildElement(); e != nullptr; e = e->NextSiblingElement()) {
      if (std::string(e->Attribute("name")) == "AndroidAuto") {
        LOG(INFO) << "BDSCONFIG: AndroidAuto Entry found";
        bdsconfigured = true;
        serviceId = static_cast<uint32_t>(e->IntAttribute("id"));
      }
      VLOG(9) << "BDSCONFIG: " << e->Attribute("name");
    }
  }

  if (bdsconfigured) {
    bcaClient = com_jci_bca_objectProxy::create(dbusConnection, "com.jci.bca", "/com/jci/bca");
    bcaClient->getcom_jci_bcaInterface()->signal_ConnectionStatusResp()->connect(sigc::mem_fun(*this,
                                                                                               &BluetoothManager::ConnectionStatusResp));
    bcaClient->getcom_jci_bcaInterface()->StartAdd(serviceId);
  }
}

void BluetoothManager::stop() {
  LOG(DEBUG) << "Stopping BluetoothManager";
  bcaClient.reset();
}

void BluetoothConnection::sendMessage(google::protobuf::MessageLite &message, uint16_t type) const {
  auto byteSize = static_cast<size_t>(message.ByteSizeLong());
  uint16_t sizeOut = htobe16(static_cast<uint16_t>(byteSize));
  uint16_t typeOut = htobe16(type);
  auto *out = new char[byteSize + 4];
  memcpy(out, &sizeOut, 2);
  memcpy(out + 2, &typeOut, 2);

  message.SerializeToArray(out + 4, byteSize);

  auto written = write(fd, out, byteSize + 4);
  if (written > -1) {
    LOG(DEBUG) << "Bytes written: " << written;
  } else {
    LOG(DEBUG) << "Could not write data";
  }
  delete[] out;
}

void BluetoothConnection::handleWifiInfoRequest(uint8_t *buffer, uint16_t length) {
  aasdk::proto::messages::WifiInfoRequest msg;
  msg.ParseFromArray(buffer, length);
  LOG(DEBUG) << "WifiInfoRequest: " << msg.DebugString();

  aasdk::proto::messages::WifiInfoResponse response;
  response.set_ip_address(info.ipAddress.c_str());
  response.set_port(configuration_->wifiPort());
  response.set_status(aasdk::proto::messages::WifiInfoResponse_Status_STATUS_SUCCESS);

  sendMessage(response, 7);
}

void BluetoothConnection::handleWifiSecurityRequest(__attribute__((unused)) uint8_t *buffer,
                                                    __attribute__((unused)) uint16_t length) {
  aasdk::proto::messages::WifiSecurityReponse response;

  response.set_ssid(configuration_->wifiSSID().c_str());
  response.set_bssid(info.macAddress.c_str());
  response.set_key(configuration_->wifiPassword().c_str());
  response.set_security_mode(aasdk::proto::messages::WifiSecurityReponse_SecurityMode_WPA2_PERSONAL);
  response.set_access_point_type(aasdk::proto::messages::WifiSecurityReponse_AccessPointType_DYNAMIC);

  sendMessage(response, 3);
}

int BluetoothConnection::handleWifiInfoRequestResponse(uint8_t *buffer, uint16_t length) {
  aasdk::proto::messages::WifiInfoResponse msg;
  msg.ParseFromArray(buffer, length);
  LOG(DEBUG) << "WifiInfoResponse: " << msg.DebugString();
  return msg.status();
}

BluetoothConnection::BluetoothConnection(autoapp::configuration::IConfiguration::Pointer configuration)
    : configuration_(std::move(configuration)) {
  update_connection_info(info);
  LOG(DEBUG) << "Got IP: " << info.ipAddress << " MAC: " << info.macAddress;

}

void BluetoothConnection::handle_connect(const std::string &pty) {
//  char buf[100];
  std::vector<char> buf;
  LOG(DEBUG) << "PTY: " << pty;
  fd = open(pty.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);

  char iptables[100];
  sprintf(iptables, "iptables -L |grep -c %u", configuration_->wifiPort());
  LOG(DEBUG) << iptables;
  int result = system(iptables);
  if (result) {
    sprintf(iptables,
            "iptables -A INPUT -p tcp --dport %u -m state --state NEW,ESTABLISHED -j ACCEPT",
            configuration_->wifiPort());
    LOG(DEBUG) << iptables;
    system(iptables);
  }

  aasdk::proto::messages::WifiInfoRequest request;
  request.set_ip_address(info.ipAddress.c_str());
  request.set_port(configuration_->wifiPort());

  sendMessage(request, 1);
  char tmp[100];
  uint16_t msgLen;
  uint16_t msg;

  fd_set set;
  timeval timeout{1, 0};

  FD_ZERO(&set);
  FD_SET(fd, &set);

  while (true) {
    if (select(fd + 1, &set, nullptr, nullptr, &timeout) <= 0) {
      break;
    }
    ssize_t i = read(fd, &tmp, 100);
    if (i > 0) {
      buf.insert(buf.cend(), tmp, tmp + i);
    }
    if (buf.size() < 4) {
      continue;
    }

    msgLen = static_cast<uint16_t>(be16toh(*(uint16_t *) buf.data()));
    msg = static_cast<uint16_t>(be16toh(*(uint16_t *) (buf.data() + 2)));
    LOG(DEBUG) << "MSG Type: " << msg << " Size: " << msgLen;

    if (static_cast<uint16_t>(buf.size()) < msgLen + 4) {
      continue;
    }

    auto *buffer = new uint8_t[msgLen];
    std::copy(buf.cbegin() + 4, buf.cbegin() + msgLen, buffer);
    switch (msg) {
      case 1:handleWifiInfoRequest(buffer, msgLen);
        break;
      case 2:handleWifiSecurityRequest(buffer, msgLen);
        break;
      case 7:handleWifiInfoRequestResponse(buffer, msgLen);
        break;
      default:break;
    }
    delete[] buffer;
    buf.erase(buf.cbegin(), buf.cbegin() + msgLen + 4);
  }

  close(fd);
}

int update_connection_info(connectionInfo &info) {
  int fd;
  struct ifreq ifr{};
  char iface[] = "wlan0";
  unsigned char *mac;

  fd = socket(AF_INET, SOCK_DGRAM, 0);

  ifr.ifr_addr.sa_family = AF_INET;
  strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);

  ioctl(fd, SIOCGIFHWADDR, &ifr);
  mac = (unsigned char *) ifr.ifr_hwaddr.sa_data;
  info.macAddress.resize(18);
  sprintf((info.macAddress).data(), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  ioctl(fd, SIOCGIFADDR, &ifr);
  info.ipAddress = inet_ntoa(((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr);

  close(fd);

  return 0;
}