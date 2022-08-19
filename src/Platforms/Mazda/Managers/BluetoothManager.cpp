#include "Platforms/Mazda/Managers/BluetoothManager.hpp"

#include "easylogging++.h"
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include  <iomanip>
#include <utility>

BluetoothManager::BluetoothManager(autoapp::configuration::Configuration::Pointer configuration,
                                   std::shared_ptr<DBus::Connection> session_connection, asio::io_service &ioService)
    : configuration_(std::move(configuration)), dbusConnection(std::move(session_connection)), timer(ioService) {

}

void BluetoothManager::ConnectionStatusResp(uint32_t found_serviceId, uint32_t connStatus, uint32_t  /*btDeviceId*/,
                                            uint32_t  /*status*/, std::tuple<std::vector<uint8_t>> terminalPath) {
  LOG(DEBUG) << "Saw Service: " << found_serviceId;
  if (found_serviceId == 1 && connStatus == 3) {
    LOG(DEBUG) << "Saw Service: Handsfree. Inititating connection";
    bcaClient->getcom_jci_bcaInterface()->StartAdd(serviceId);
  } else if (found_serviceId == serviceId && connStatus == 3) {
    std::vector<uint8_t> tpath = std::get<0>(terminalPath);
    std::string pty(tpath.begin(), tpath.end());
    LOG(DEBUG) << "PTY: " << pty;
    update_connection_info();
    MazdaBluetoothConnection bconnection(configuration_->getWifiConfig());
    bconnection.handle_connect(pty);
  }
}

void BluetoothManager::start() {
  LOG(DEBUG) << "Reading BdsConfiguration.xml";

  tinyxml2::XMLDocument doc;
  doc.LoadFile("/jci/bds/BdsConfiguration.xml");

  tinyxml2::XMLNode *docRoot = doc.FirstChild();

  tinyxml2::XMLElement *serviceconfig = docRoot->FirstChildElement("serviceConfiguration");

  if (serviceconfig == nullptr) {
    LOG(DEBUG) << "Couldn't find serviceConfiguration in /jci/bds/BdsConfiguration.xml";
  } else {
    for (tinyxml2::XMLElement *childElement = serviceconfig->FirstChildElement(); childElement != nullptr; childElement = childElement->NextSiblingElement()) {
      if (std::string(childElement->Attribute("name")) == "AndroidAuto") {
        LOG(INFO) << "BDSCONFIG: AndroidAuto Entry found";
        bdsconfigured = true;
        serviceId = static_cast<uint32_t>(childElement->IntAttribute("id"));
      }
      VLOG(9) << "BDSCONFIG: " << childElement->Attribute("name");
    }
  }

  if (bdsconfigured) {
    bcaClient = com_jci_bca_objectProxy::create(dbusConnection, "com.jci.bca", "/com/jci/bca");
    bcaClient->getcom_jci_bcaInterface()->signal_ConnectionStatusResp()->connect(sigc::mem_fun(*this, &BluetoothManager::ConnectionStatusResp));
    timer.expires_from_now(std::chrono::seconds(1));
    timer.async_wait([this](const asio::error_code &error) { retryTimer(error); });
  }
}

void BluetoothManager::stop() {
  LOG(DEBUG) << "Stopping BluetoothManager";
  bcaClient.reset();
}

MazdaBluetoothConnection::MazdaBluetoothConnection(autoapp::configuration::WifiConfiguration::pointer WifiConfig) : BluetoothConnection(std::move(WifiConfig)) {
}

ssize_t MazdaBluetoothConnection::read(char *buf, ssize_t size){
  fd_set set;
  timeval timeout{1, 0};

  FD_ZERO(&set);
  FD_SET(fd, &set);
  if (select(fd + 1, &set, nullptr, nullptr, &timeout) <= 0) {
    return 0;
  }
  return ::read(fd, buf, size);
}

ssize_t MazdaBluetoothConnection::write(char *buf, ssize_t size){
  return ::write(fd, buf, size);
}

void MazdaBluetoothConnection::handle_connect(const std::string &pty) {
  LOG(DEBUG) << "PTY: " << pty;
  fd = open(pty.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);

  std::ostringstream check_iptables;
  check_iptables << "iptables -L |grep -c " << wifiConfig->port;
  LOG(DEBUG) << check_iptables.str();
  int result = system(check_iptables.str().c_str());
  if (result != 0) {
    std::ostringstream iptables_cmd;
    iptables_cmd << "iptables -A INPUT -p tcp --dport " << wifiConfig->port
                 << " -m state --state NEW,ESTABLISHED -j ACCEPT";
    LOG(DEBUG) << iptables_cmd.str();
    system(iptables_cmd.str().c_str());
  }

  messageHandler();

  close(fd);
}


int BluetoothManager::update_connection_info() {
  int fileDescriptor;
  struct ifreq ifr{};
  const std::string iface = "wlan0";
  autoapp::configuration::WifiConfiguration::pointer wifiConfig = configuration_->getWifiConfig();

  fileDescriptor = socket(AF_INET, SOCK_DGRAM, 0);

  ifr.ifr_addr.sa_family = AF_INET;
  strncpy(ifr.ifr_name, iface.c_str(), IFNAMSIZ - 1);

  ioctl(fileDescriptor, SIOCGIFHWADDR, &ifr);
  std::ostringstream macString;
  macString << std::setfill('0') << std::setw(2) << std::hex << std::uppercase;
  const int macLength = 5;
  for (int position = 0; position <= macLength; position++) {
    if(!macString.str().empty()){
      macString << ":";
    }
    macString << static_cast<int>(ifr.ifr_hwaddr.sa_data[position]);
  }
  wifiConfig->bssid.assign(macString.str());
  ioctl(fileDescriptor, SIOCGIFADDR, &ifr);
  wifiConfig->ipAddress = inet_ntoa(((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr);

  close(fileDescriptor);

  return 0;
}
void BluetoothManager::aaConnect(bool connected) {
  if(connected){
    timer.cancel();
  }
  else{
    timer.expires_from_now(std::chrono::seconds(1));
    timer.async_wait([this](const asio::error_code &error) { retryTimer(error); });
  }

}
void BluetoothManager::retryTimer(const asio::error_code &error) {
  if (error == asio::error::operation_aborted){
    return;
  }
  bcaClient->getcom_jci_bcaInterface()->StartAdd(serviceId);
  timer.expires_from_now(std::chrono::seconds(30));
  timer.async_wait([this](const asio::error_code &error) { retryTimer(error); });
}
