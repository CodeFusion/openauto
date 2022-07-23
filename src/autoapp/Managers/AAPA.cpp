#include "autoapp/Managers/AAPA.hpp"
#include <easylogging++.h>

#include <utility>

AADBus::AADBus(VideoSignals::Pointer videosignals) : vs(videosignals) {

}

uint8_t AADBus::GetAvailable() {
  return 1;
}

void AADBus::VideoProjectionEventToMD(uint32_t videoProjectionEvent) {
  LOG(DEBUG) << "VideoProjectionEventToMD " << videoProjectionEvent;
  if (videoProjectionEvent == 1) {
    vs->focusChanged.emit(false);
  }
  if (videoProjectionEvent == 4){
    vs->focusChanged.emit(true);
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

AAPA::AAPA(VideoSignals::Pointer videosignals, AASignals::Pointer aasignals,
           const std::shared_ptr<DBus::Connection> &session_connection) :
    vs(std::move(videosignals)), as(std::move(aasignals)) {
  session_connection->request_name("com.jci.aapa", DBUSCXX_NAME_FLAG_REPLACE_EXISTING);

  androiddbus = new AADBus(vs);

  adapter = com_jci_aapaInterface::create(androiddbus);

  session_object = com_jci_aapa_objectAdapter::create(session_connection, adapter, "/com/jci/aapa");

  adapter->signal_Available()->emit(1);

  releaseFocusConnection = vs->focusRelease.connect(sigc::mem_fun(*this, &AAPA::releaseFocus));
  requestFocusConnection = vs->focusRequest.connect(sigc::mem_fun(*this, &AAPA::requestFocus));
  ConnectedConnection = as->connected.connect(sigc::mem_fun(*this, &AAPA::AAConnected));


  bucpsa = com_jci_bucpsa_objectProxy::create(session_connection, "com.jci.bucpsa", "/com/jci/bucpsa");
  bucpsa->getcom_jci_bucpsaInterface()->signal_DisplayMode()->connect(sigc::mem_fun(*this, &AAPA::DisplayMode));

}

void AAPA::DisplayMode(uint32_t DisplayMode) {
//   currentDisplayMode != 0 means backup camera wants the screen
  if ((bool) DisplayMode && _connected) {
    vs->focusChanged.emit(false);
  }
}

void AAPA::requestFocus() {
  adapter->signal_VideoProjectionRequestFromMD()->emit(0);
  adapter->signal_ProjectionStatusResult()->emit(true);
  vs->focusChanged.emit(true);
}

void AAPA::releaseFocus() {
  adapter->signal_VideoProjectionRequestFromMD()->emit(1);
  vs->focusChanged.emit(false);
}

AAPA::~AAPA() {
  LOG(DEBUG) << "Stopping VideoManager";
  releaseFocusConnection.disconnect();
  requestFocusConnection.disconnect();
  FocusChangeConnection.disconnect();
  ConnectedConnection.disconnect();
  releaseFocus();
}
void AAPA::AAConnected(bool connected) {
  LOG(DEBUG) << "AAConnected" << connected;
  _connected = connected;
}
