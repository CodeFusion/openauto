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
  if (videoProjectionEvent == 0) {
    vs->focusChanged.emit(true);
  } else {
    vs->focusChanged.emit(false);
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
  FocusChangeConnection = vs->focusChanged.connect(sigc::mem_fun(*this, &AAPA::FocusChange));
  ConnectedConnection = as->connected.connect(sigc::mem_fun(*this, &AAPA::AAConnected));


  bucpsa = com_jci_bucpsa_objectProxy::create(session_connection, "com.jci.bucpsa", "/com/jci/bucpsa");
  bucpsa->getcom_jci_bucpsaInterface()->signal_DisplayMode()->connect(sigc::mem_fun(*this, &AAPA::DisplayMode));

}

void AAPA::DisplayMode(uint32_t DisplayMode) {
  // currentDisplayMode != 0 means backup camera wants the screen
  if ((bool) DisplayMode && _connected) {
    this->vs->focusRelease.emit(VIDEO_FOCUS_REQUESTOR::BACKUP_CAMERA);
    if (hasFocus) {
      this->waitsForFocus = true;
    }
  } else {
    if (waitsForFocus) {
      this->vs->focusRequest.emit(VIDEO_FOCUS_REQUESTOR::BACKUP_CAMERA);
      waitsForFocus = false;
    }
  }
}

void AAPA::FocusChange(bool focus) {
  hasFocus = focus;
}

void AAPA::requestFocus(VIDEO_FOCUS_REQUESTOR requestor) {
  adapter->signal_VideoProjectionRequestFromMD()->emit(0);
  adapter->signal_ProjectionStatusResult()->emit(true);
  vs->focusChanged.emit(true);
}

void AAPA::releaseFocus(VIDEO_FOCUS_REQUESTOR requestor) {
  adapter->signal_VideoProjectionRequestFromMD()->emit(1);
}

AAPA::~AAPA() {
  LOG(DEBUG) << "Stopping VideoManager";
  releaseFocusConnection.disconnect();
  requestFocusConnection.disconnect();
  FocusChangeConnection.disconnect();
  releaseFocus(VIDEO_FOCUS_REQUESTOR::HEADUNIT);
}
void AAPA::AAConnected(bool connected) {
  _connected = connected;
}
