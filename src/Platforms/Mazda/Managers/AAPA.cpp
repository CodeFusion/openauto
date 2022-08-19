#include "Platforms/Mazda/Managers/AAPA.hpp"
#include "easylogging++.h"

#include <utility>

AADBus::AADBus(std::function<void(bool)> FocusChanged) : focusChanged(std::move(FocusChanged)) {

}

uint8_t AADBus::GetAvailable() {
  LOG(DEBUG) << "";
  return 2;
}

void AADBus::VideoProjectionEventToMD(uint32_t videoProjectionEvent) {
  LOG(DEBUG) << "VideoProjectionEventToMD " << videoProjectionEvent;
  switch (videoProjectionEvent) {
    case 0:
      focusChanged(true);
      break;
    case 1:
      focusChanged(false);
      break;
    case 4:
      focusChanged(true);
      break;
    default:
      LOG(DEBUG) << "Unknown Event " << videoProjectionEvent;
      break;
  }
}

AADBus::NowPlayingInfo AADBus::GetNowPlayingInfo() {
  LOG(DEBUG) << "GetNowPlayingInfo";
  return {};
}

DBus::MultipleReturn<bool, std::string, uint32_t> AADBus::GetAOASessionStatus() {
  LOG(DEBUG) << "GetAOASessionStatus";
  return {};
}

std::tuple<int32_t> AADBus::SetNativeTurnByTurnStatus(std::tuple<bool> setNativeTurnByTurnStatus) {
  LOG(DEBUG) << "setNativeTurnByTurnStatus";
  return {};
}

int32_t AADBus::SetMP911EmergencyCallStatus(uint32_t status) {
  LOG(DEBUG) << "";
  return {};
}

void AADBus::SbnStatus(bool status) {
  LOG(DEBUG) << "SbnStatus " << status;
}
void AAPA::SetVehicleBtMacAddress(std::string macaddr, uint32_t maclen) {
  LOG(DEBUG) << macaddr;
  btMac.assign(macaddr);
}

AAPA::AAPA(std::shared_ptr<DBus::Connection> session_connection) : dbusConnection(std::move(session_connection)){
}

//void AAPA::DisplayMode(uint32_t DisplayMode) {
////   currentDisplayMode != 0 means backup camera wants the screen
////  if ((bool) DisplayMode) {
////    focusChanged(false);
////  }
//}

// Projection on = 0
// Projection off = 1
// Projection Resume = 5
// Projection on with Audio = 6

void AAPA::requestFocus() {
  LOG(DEBUG) << "requestFocus";
  adapter->signal_VideoProjectionRequestFromMD()->emit(0);
//  adapter->signal_ProjectionStatusResult()->emit(true);
//  focusChanged(true);
}

void AAPA::releaseFocus() {
  LOG(DEBUG) << "releaseFocus";
  adapter->signal_VideoProjectionRequestFromMD()->emit(1);
//  focusChanged(false);
}

void AAPA::start() {
  LOG(DEBUG) << "Starting AAPA";
  dbusConnection->request_name("com.jci.aapa", DBUSCXX_NAME_FLAG_REPLACE_EXISTING);
  dbusConnection->request_name("com.jci.AAPVIDEO", DBUSCXX_NAME_FLAG_REPLACE_EXISTING);

  aapVideoObject = dbusConnection->create_object("/com/jci/AAPVIDEO");
  aapVideo = new AAPVideo;
  aapVideoObject->create_method<void(void)>("NotifyEnable", sigc::mem_fun(*aapVideo, &AAPVideo::NotifyEnable) );
  aapVideoObject->create_method<int32_t(void)>("StopVideo", sigc::mem_fun(*this, &AAPA::StopVideo) );
  aapVideoObject->create_method<int32_t(void)>("CleanVideo", sigc::mem_fun(*aapVideo, &AAPVideo::CleanVideo) );


  androiddbus = new AADBus([this](bool focus) { this->focusChanged(focus); });

  adapter = com_jci_aapaInterface::create(androiddbus);
  adapter->remove_method("SetVehicleBtMacAddress");
  adapter->create_method<void(std::string macaddr, uint32_t maclen)>("SetVehicleBtMacAddress", sigc::mem_fun(*this, &AAPA::SetVehicleBtMacAddress));

  session_object = com_jci_aapa_objectAdapter::create(dbusConnection, adapter, "/com/jci/aapa");

  adapter->signal_Available()->emit(2);
  adapter->signal_AOASessionStatus()->emit(true);

//  bucpsa = com_jci_bucpsa_objectProxy::create(dbusConnection, "com.jci.bucpsa", "/com/jci/bucpsa");
//  displayModeConnection =
//      bucpsa->getcom_jci_bucpsaInterface()->signal_DisplayMode()->connect(sigc::mem_fun(*this, &AAPA::DisplayMode));

  registerFocus([this](bool state){
    if(state) {
      LOG(DEBUG) << "adapter->signal_ProjectionStatusResult()->emit(true);";
      adapter->signal_ProjectionStatusResult()->emit(true);
    }
    else {
      LOG(DEBUG) << "adapter->signal_ProjectionStatusResult()->emit(false);";
      adapter->signal_ProjectionStatusResult()->emit(false);
    }
  });
  adapter->signal_GetVehicleBtMacAddress()->emit();
}

void AAPA::stop() {
  LOG(DEBUG) << "Stopping AAPA";
  releaseFocus();
  adapter->signal_AOASessionStatus()->emit(false);
  adapter->signal_Available()->emit(0);
  sleep(1);
  displayModeConnection.disconnect();
//  bucpsa.reset();
  session_object.reset();
  adapter.reset();
  delete androiddbus;
  dbusConnection->release_name("com.jci.aapa");
}

void AAPVideo::NotifyEnable() {
  LOG(DEBUG) << "NotifyEnable";
}

int32_t AAPA::StopVideo() {
  LOG(DEBUG) << "StopVideo";
  focusChanged(false);
  return 0;
}

void AAPA::pairingRequest(std::string mac, aasdk::io::Promise<void>::Pointer promise) {
  adapter->signal_StartBtConnection()->emit();
  adapter->signal_BTPairingRequest()->emit(mac, mac.size(), true);
  adapter->signal_NotifyDeviceConnection()->emit(true);
  adapter->signal_NotifyBTConnectionComplete()->emit(true);
  promise->resolve();
}
std::string AAPA::getMac() {
  return btMac;
}

int32_t AAPVideo::CleanVideo() {
  LOG(DEBUG) << "CleanVideo";
  return 0;
}
