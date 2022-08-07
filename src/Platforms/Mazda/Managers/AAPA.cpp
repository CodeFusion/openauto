#include "Platforms/Mazda/Managers/AAPA.hpp"
#include "easylogging++.h"

#include <utility>

AADBus::AADBus(std::function<void(bool)> FocusChanged) : focusChanged(std::move(FocusChanged)) {

}

uint8_t AADBus::GetAvailable() {
  return 1;
}

void AADBus::VideoProjectionEventToMD(uint32_t videoProjectionEvent) {
  LOG(DEBUG) << "VideoProjectionEventToMD " << videoProjectionEvent;
  if (videoProjectionEvent == 1) {
    focusChanged(false);
  }
  if (videoProjectionEvent == 4) {
    focusChanged(true);
  }
}

AADBus::NowPlayingInfo AADBus::GetNowPlayingInfo() {
  return {};
}

DBus::MultipleReturn<bool, std::string, uint32_t> AADBus::GetAOASessionStatus() {
  return {};
}

std::tuple<int32_t> AADBus::SetNativeTurnByTurnStatus(std::tuple<bool> setNativeTurnByTurnStatus) {
  return {};
}

int32_t AADBus::SetMP911EmergencyCallStatus(uint32_t status) {
  return {};
}

void AADBus::SbnStatus(bool status) {
  LOG(DEBUG) << "SbnStatus " << status;
}

AAPA::AAPA(std::shared_ptr<DBus::Connection> session_connection) : dbusConnection(std::move(session_connection)){
}

void AAPA::DisplayMode(uint32_t DisplayMode) {
//   currentDisplayMode != 0 means backup camera wants the screen
  if ((bool) DisplayMode) {
    focusChanged(false);
  }
}

void AAPA::requestFocus() {
  adapter->signal_VideoProjectionRequestFromMD()->emit(0);
  adapter->signal_ProjectionStatusResult()->emit(true);
  focusChanged(true);
}

void AAPA::releaseFocus() {
  adapter->signal_VideoProjectionRequestFromMD()->emit(1);
  focusChanged(false);
}

AAPA::~AAPA() {

}

void AAPA::start() {
  LOG(DEBUG) << "Starting AAPA";
  dbusConnection->request_name("com.jci.aapa", DBUSCXX_NAME_FLAG_REPLACE_EXISTING);

  androiddbus = new AADBus([this](bool focus) { this->focusChanged(focus); });

  adapter = com_jci_aapaInterface::create(androiddbus);

  session_object = com_jci_aapa_objectAdapter::create(dbusConnection, adapter, "/com/jci/aapa");

  adapter->signal_Available()->emit(1);

  bucpsa = com_jci_bucpsa_objectProxy::create(dbusConnection, "com.jci.bucpsa", "/com/jci/bucpsa");
  displayModeConnection =
      bucpsa->getcom_jci_bucpsaInterface()->signal_DisplayMode()->connect(sigc::mem_fun(*this, &AAPA::DisplayMode));
}

void AAPA::stop() {
  LOG(DEBUG) << "Stopping AAPA";
  releaseFocus();
  displayModeConnection.disconnect();
  bucpsa.reset();
  session_object.reset();
  adapter.reset();
  delete androiddbus;
  dbusConnection->release_name("com.jci.aapa");
}
