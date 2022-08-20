#include "Platforms/Mazda/Managers/AAPA.hpp"
#include "easylogging++.h"

#include <utility>

AADBus::AADBus(std::function<void(bool)> FocusChanged) : focusChanged(std::move(FocusChanged)) {

}

uint8_t AADBus::GetAvailable() {
  // TODO: Actually reflect real session status
  LOG(DEBUG) << "";
  return 2;
}


/// Handle call from MMUI_ANDROIDAUTO to let us know if we should display or not.
/// \param videoProjectionEvent between 0 and 4
void AADBus::VideoProjectionEventToMD(uint32_t videoProjectionEvent) {
  LOG(DEBUG) << "VideoProjectionEventToMD " << videoProjectionEvent;
  switch (videoProjectionEvent) {
    case AAPA::VideoProjectionEventTo::VideoProjectionEventTo_START:
      focusChanged(true);
      break;
    case AAPA::VideoProjectionEventTo::VideoProjectionEventTo_STOP:
      focusChanged(false);
      break;
    case AAPA::VideoProjectionEventTo::VideoProjectionEventTo_RESUME:
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

/// Handle recieving vehicles BlueTooth Address from Mazda Systems
/// \param macaddr
/// \param maclen
void AAPA::SetVehicleBtMacAddress(std::string macaddr, uint32_t maclen) {
  LOG(DEBUG) << macaddr;
  btMac.assign(macaddr);
}

AAPA::AAPA(std::shared_ptr<DBus::Connection> session_connection) : dbusConnection(std::move(session_connection)){
}

void AAPA::requestFocus() {
  LOG(DEBUG) << "requestFocus";
  adapter->signal_VideoProjectionRequestFromMD()->emit(AAPA::VideoProjectionEventFrom::VideoProjectionEventFrom_START);
}

void AAPA::releaseFocus() {
  LOG(DEBUG) << "releaseFocus";
  adapter->signal_VideoProjectionRequestFromMD()->emit(AAPA::VideoProjectionEventFrom::VideoProjectionEventFrom_STOP);
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

  //Initiate the class for most of our DBUS methods used by MMUI_ANDROIDAUTO
  androiddbus = new AADBus([this](bool focus) { this->focusChanged(focus); });
  adapter = com_jci_aapaInterface::create(androiddbus);

  // TODO: Probably a better way of handling this
  adapter->remove_method("SetVehicleBtMacAddress");
  adapter->create_method<void(std::string macaddr, uint32_t maclen)>("SetVehicleBtMacAddress", sigc::mem_fun(*this, &AAPA::SetVehicleBtMacAddress));

  session_object = com_jci_aapa_objectAdapter::create(dbusConnection, adapter, "/com/jci/aapa");

  // Emit that Android Auto is availible to rest of CMU Software
  adapter->signal_Available()->emit(2);
  // Emit that we have an established Androud Auto session
  adapter->signal_AOASessionStatus()->emit(true);

  // Handle sending our Projection status to MMUI_ANDROIDAUTO after we start video
  // TODO: Probably a neater way to handle this too.
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
  // Reqeust that something send us what the CMU Bluetooth Address is. Is then recieved by AAPA::SetVehicleBtMacAddress
  adapter->signal_GetVehicleBtMacAddress()->emit();
}

void AAPA::stop() {
  LOG(DEBUG) << "Stopping AAPA";
  releaseFocus();
  adapter->signal_AOASessionStatus()->emit(false);
  adapter->signal_Available()->emit(0);
  sleep(1);
  displayModeConnection.disconnect();
  session_object.reset();
  adapter.reset();
  delete androiddbus;
  dbusConnection->release_name("com.jci.aapa");
}

/// Called by MMUI_ANDROIDAUTO when video can start again. Redundant by AADBus::VideoProjectionEventToMD
void AAPVideo::NotifyEnable() {
  LOG(DEBUG) << "NotifyEnable";
}

/// Called by MMUI_ANDROIDAUTO when video needs to stop. Possibly redundant by AADBus::VideoProjectionEventToMD
int32_t AAPA::StopVideo() {
  LOG(DEBUG) << "StopVideo";
  focusChanged(false);
  return 0;
}

/// Handle pairing request
/// \param mac
/// \param promise
void AAPA::pairingRequest(std::string mac, aasdk::io::Promise<void>::Pointer promise) {
  // TODO: Actually handle pairing requests if the phone isn't connected via bluetooth already
  // This notifies the bluetooth/MMUI_PHONE systems that Android Auto is handling calls.
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
