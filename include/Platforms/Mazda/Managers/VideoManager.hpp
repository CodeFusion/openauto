#pragma once

#include <dbus-cxx.h>

#include <atomic>
#include <set>
#include "autoapp/Managers/IVideoManager.hpp"
#include "autoapp/Service/VideoService.hpp"

#include <com_jci_nativeguictrl_objectProxy.h>
#include <com_jci_bucpsa_objectProxy.h>

class VideoManager: public IVideoManager{
 private:
  std::shared_ptr<DBus::Connection> dbusConnection;
  std::shared_ptr<com_jci_bucpsa_objectProxy> bucpsa;
  sigc::connection displayModeConnection;
  bool waitsForFocus = false;
  bool currentDisplayMode = false;
  bool hasFocus = false;
  std::shared_ptr<com_jci_nativeguictrl_objectProxy> gui;
  void DisplayMode(uint32_t);

 public:
  explicit VideoManager(std::shared_ptr<DBus::Connection> session_connection);
  ~VideoManager() override;

  void start() override;
  void stop() override;

  void requestFocus() override;
  void releaseFocus() override;

  enum SURFACES {
    NNG_NAVI_ID [[maybe_unused]] = 0,
    TV_TOUCH_SURFACE [[maybe_unused]] = 1,
    NATGUI_SURFACE [[maybe_unused]] = 2,
    LOOPLOGO_SURFACE [[maybe_unused]] = 3,
    TRANLOGOEND_SURFACE [[maybe_unused]] = 4,
    TRANLOGO_SURFACE [[maybe_unused]] = 5,
    QUICKTRANLOGO_SURFACE [[maybe_unused]] = 6,
    EXITLOGO_SURFACE [[maybe_unused]] = 8,
    JCI_OPERA_PRIMARY [[maybe_unused]] = 9,
    JCI_OPERA_SECONDARY [[maybe_unused]] = 10,
    lvdsSurface [[maybe_unused]] = 11,
    SCREENREP_IVI_NAME [[maybe_unused]]= 12,
    NNG_NAVI_MAP1 [[maybe_unused]] = 13,
    NNG_NAVI_MAP2 [[maybe_unused]] = 14,
    NNG_NAVI_HMI [[maybe_unused]]= 15,
    NNG_NAVI_TWN [[maybe_unused]] = 16,
  };

};