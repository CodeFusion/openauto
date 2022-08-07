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
  std::shared_ptr<com_jci_bucpsa_objectProxy> bucpsa;
  bool waitsForFocus = false;
  bool currentDisplayMode;
  bool hasFocus = false;
  std::shared_ptr<com_jci_nativeguictrl_objectProxy> gui;
  void DisplayMode(uint32_t);

 public:
  explicit VideoManager(const std::shared_ptr<DBus::Connection> &);
  ~VideoManager();

  void start() override {};
  void stop() override {};

  void requestFocus() override;
  void releaseFocus() override;

  enum SURFACES {
    NNG_NAVI_ID = 0,
    TV_TOUCH_SURFACE = 1,
    NATGUI_SURFACE = 2,
    LOOPLOGO_SURFACE = 3,
    TRANLOGOEND_SURFACE = 4,
    TRANLOGO_SURFACE = 5,
    QUICKTRANLOGO_SURFACE = 6,
    EXITLOGO_SURFACE = 8,
    JCI_OPERA_PRIMARY = 9,
    JCI_OPERA_SECONDARY = 10,
    lvdsSurface = 11,
    SCREENREP_IVI_NAME = 12,
    NNG_NAVI_MAP1 = 13,
    NNG_NAVI_MAP2 = 14,
    NNG_NAVI_HMI = 15,
    NNG_NAVI_TWN = 16,
  };

};