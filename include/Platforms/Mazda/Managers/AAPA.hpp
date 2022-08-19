#pragma once

#include <dbus-cxx.h>

#include <atomic>
#include <set>
#include "autoapp/Managers/IVideoManager.hpp"
#include "autoapp/Managers/IBluetoothPairingManager.hpp"
#include "autoapp/Service/VideoService.hpp"

#include <com_jci_aapa_objectAdapter.h>
#include <com_jci_bucpsa_objectProxy.h>
#include <com_jci_bca_objectProxy.h>


class AADBus final : public com_jci_aapa {
 private:
  std::function<void(bool)> focusChanged;

 public:
  explicit AADBus(std::function<void(bool)> FocusChanged);
  ~AADBus() = default;

  using NowPlayingInfo = DBus::MultipleReturn<std::tuple<uint8_t, std::string, std::string, std::string, std::string>, int32_t>;
  void MDSettingModeData(uint32_t selectMode) override {};
  void VideoProjectionEventToMD(uint32_t videoProjectionEvent) override;
  void InputKey(uint32_t keyCode, bool absolute, int32_t step) override {};
  void SetRouteState(uint32_t state) override {};
  NowPlayingInfo GetNowPlayingInfo() override;
  void BTPairingResult(bool result) override {};
  void BTReadyToPair(bool alreadyPaired) override {};
  void SendBTAuthenticationData(std::string btdata, uint32_t len) override {};
  void SetVehicleBtMacAddress(std::string macaddr, uint32_t maclen) override {};
  void NotifyBTHFCallStatus(bool status, std::string macaddr) override {};
  void IntentPhoneCall(std::string phonenumber) override {};
  void DialogWinkStatus(bool status) override {};
  void ClearLastFocus(std::tuple<bool, bool> focusInfo) override {};
  void SbnStatus(bool status) override;
  DBus::MultipleReturn<bool, std::string, uint32_t> GetAOASessionStatus() override;
  uint8_t GetAvailable() override;
  std::tuple<int32_t> SetNativeTurnByTurnStatus(std::tuple<bool> setNativeTurnByTurnStatus) override;
  int32_t SetMP911EmergencyCallStatus(uint32_t status) override;

};

class AAPVideo {
 public:
  void NotifyEnable();
  int32_t CleanVideo();
};

class AAPA : public IVideoManager, public IBluetoothPairingManager {
 private:
  std::shared_ptr<DBus::Connection> dbusConnection;
  sigc::connection ConnectedConnection;
  sigc::connection displayModeConnection;
  std::shared_ptr<com_jci_aapa_objectAdapter> session_object;
  std::shared_ptr<com_jci_aapaInterface> adapter;
  AADBus *androiddbus;
//  std::shared_ptr<com_jci_bucpsa_objectProxy> bucpsa;
  std::shared_ptr<DBus::Object> aapVideoObject;
  AAPVideo *aapVideo;
  std::string btMac;

//  void DisplayMode(uint32_t DisplayMode);
  int32_t StopVideo();
  void SetVehicleBtMacAddress(std::string macaddr, uint32_t maclen);


 public:
  using Pointer = std::shared_ptr<AAPA>;
  explicit AAPA(std::shared_ptr<DBus::Connection> session_connection);
  ~AAPA() override = default;

  void start() override;
  void stop() override;

  void requestFocus() override;
  void releaseFocus() override;

  void pairingRequest(std::string mac, aasdk::io::Promise<void>::Pointer promise) override;
  std::string getMac() override;


};