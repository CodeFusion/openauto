#include "Platforms/Mazda/Managers/VideoManager.hpp"
#include "easylogging++.h"

VideoManager::VideoManager(const std::shared_ptr<DBus::Connection> &session_connection){
  gui =
      com_jci_nativeguictrl_objectProxy::create(session_connection, "com.jci.nativeguictrl", "/com/jci/nativeguictrl");
  bucpsa = com_jci_bucpsa_objectProxy::create(session_connection, "com.jci.bucpsa", "/com/jci/bucpsa");
  bucpsa->getcom_jci_bucpsaInterface()->signal_DisplayMode()->connect(sigc::mem_fun(*this, &VideoManager::DisplayMode));

  std::tuple<unsigned int, int> display_mode = bucpsa->getcom_jci_bucpsaInterface()->GetDisplayMode();

  currentDisplayMode = (bool) std::get<0>(display_mode);
}

void VideoManager::DisplayMode(uint32_t DisplayMode) {
  currentDisplayMode = (bool) DisplayMode;
  // currentDisplayMode != 0 means backup camera wants the screen
  if ((bool) DisplayMode) {
    if (hasFocus) {
      waitsForFocus = true;
    }
    releaseFocus();
  } else {
    requestFocus();
  }
}

void VideoManager::requestFocus() {
  if (currentDisplayMode) {
    // we can safely exit - backup camera will notice us when we finish; and we request focus back
    waitsForFocus = true;
    return;
  }
  if (waitsForFocus) {
    // need to wait for a second (maybe less but 100ms is too early) to make sure
    // the CMU has already changed the surface from backup camera to opera
    sleep(1);
    waitsForFocus = false;
  }
  LOG(DEBUG) << "Setting focus";
  hasFocus = true;
  gui->getcom_jci_nativeguictrlInterface()->SetRequiredSurfaces(std::to_string(SURFACES::TV_TOUCH_SURFACE), 1);
}

void VideoManager::releaseFocus() {
  LOG(DEBUG) << "Releasing focus";
  hasFocus = false;
  gui->getcom_jci_nativeguictrlInterface()->SetRequiredSurfaces(std::to_string(SURFACES::JCI_OPERA_PRIMARY), 1);
}

VideoManager::~VideoManager() {
  LOG(DEBUG) << "Stopping VideoManager";
  releaseFocus();
  bucpsa.reset();
}
