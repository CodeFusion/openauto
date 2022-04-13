#ifndef COM_JCI_BLM_NMSPROXY_H
#define COM_JCI_BLM_NMSPROXY_H

#include <dbus-cxx.h>
#include <memory>
#include <stdint.h>
#include <string>
class com_jci_blm_nmsProxy
    : public DBus::InterfaceProxy {
 protected:
  com_jci_blm_nmsProxy(std::string name);
 public:
  static std::shared_ptr<com_jci_blm_nmsProxy> create(std::string name = "com.jci.blm_nms");
  DBus::MultipleReturn<int32_t, int32_t> GetConnectionState();
  DBus::MultipleReturn<int32_t, int32_t> GetConnectionState_Async();
  DBus::MultipleReturn<int32_t, int32_t> Connect(int32_t ifcId);
  DBus::MultipleReturn<int32_t, int32_t> Connect_Async(int32_t ifcId);
  int32_t Disconnect(int32_t ifcId);
  int32_t Disconnect_Async(int32_t ifcId);
  int32_t Abort(int32_t reqId);
  int32_t Abort_Async(int32_t reqId);
  int32_t SetWiFiAutoconnect(bool state);
  int32_t SetWiFiAutoconnect_Async(bool state);
  DBus::MultipleReturn<int32_t, bool> GetWiFiAutoconnect();
  DBus::MultipleReturn<int32_t, bool> GetWiFiAutoconnect_Async();
  int32_t SetModemAutoconnect(bool state);
  int32_t SetModemAutoconnect_Async(bool state);
  DBus::MultipleReturn<int32_t, bool> GetModemAutoconnect();
  DBus::MultipleReturn<int32_t, bool> GetModemAutoconnect_Async();
  DBus::MultipleReturn<int32_t, std::tuple<std::vector<int32_t>>> GetInterfaceList();
  DBus::MultipleReturn<int32_t, std::tuple<std::vector<int32_t>>> GetInterfaceList_Async();
  DBus::MultipleReturn<int32_t,
                       std::tuple<int32_t,
                                  int32_t,
                                  std::string,
                                  std::string,
                                  std::string,
                                  std::string,
                                  std::string,
                                  std::string,
                                  std::string,
                                  std::string,
                                  int32_t,
                                  int32_t,
                                  int32_t,
                                  int32_t>> GetInterfaceParams(int32_t ifcId);
  DBus::MultipleReturn<int32_t,
                       std::tuple<int32_t,
                                  int32_t,
                                  std::string,
                                  std::string,
                                  std::string,
                                  std::string,
                                  std::string,
                                  std::string,
                                  std::string,
                                  std::string,
                                  int32_t,
                                  int32_t,
                                  int32_t,
                                  int32_t>> GetInterfaceParams_Async(int32_t ifcId);
  int32_t SetInterfaceParams(int32_t ifcId,
                             std::tuple<std::string,
                                        std::string,
                                        std::string,
                                        std::string,
                                        std::string,
                                        std::string,
                                        int32_t,
                                        int32_t> ifcParams,
                             int32_t setMask);
  int32_t SetInterfaceParams_Async(int32_t ifcId,
                                   std::tuple<std::string,
                                              std::string,
                                              std::string,
                                              std::string,
                                              std::string,
                                              std::string,
                                              int32_t,
                                              int32_t> ifcParams,
                                   int32_t setMask);
  int32_t DHCP_ServerSetConfig(int32_t ifcId, std::tuple<std::string, std::string> dhcpConf);
  int32_t DHCP_ServerSetConfig_Async(int32_t ifcId, std::tuple<std::string, std::string> dhcpConf);
  DBus::MultipleReturn<int32_t, std::tuple<std::string, std::string>> DHCP_ServerGetConfig(int32_t ifcId);
  DBus::MultipleReturn<int32_t, std::tuple<std::string, std::string>> DHCP_ServerGetConfig_Async(int32_t ifcId);
  int32_t DHCP_ServerStart(int32_t ifcId);
  int32_t DHCP_ServerStart_Async(int32_t ifcId);
  int32_t DHCP_ServerStop(int32_t ifcId);
  int32_t DHCP_ServerStop_Async(int32_t ifcId);
  DBus::MultipleReturn<int32_t,
                       std::tuple<std::vector<std::tuple<std::string,
                                                         std::string,
                                                         std::string,
                                                         std::string>>>> DHCP_ServerGetClients(int32_t ifcId);
  DBus::MultipleReturn<int32_t,
                       std::tuple<std::vector<std::tuple<std::string,
                                                         std::string,
                                                         std::string,
                                                         std::string>>>> DHCP_ServerGetClients_Async(int32_t ifcId);
  int32_t ActivateNAT(bool enable);
  int32_t ActivateNAT_Async(bool enable);
  int32_t AdvancedRouting(bool enable);
  int32_t AdvancedRouting_Async(bool enable);
  int32_t SetHostname(std::string hostname);
  int32_t SetHostname_Async(std::string hostname);
  DBus::MultipleReturn<int32_t, std::string> GetHostname();
  DBus::MultipleReturn<int32_t, std::string> GetHostname_Async();
  int32_t MonitorInterface(int32_t ifcId, bool turn);
  int32_t MonitorInterface_Async(int32_t ifcId, bool turn);
  int32_t WIFI_SetMode(int32_t ifcId, int32_t mode);
  int32_t WIFI_SetMode_Async(int32_t ifcId, int32_t mode);
  DBus::MultipleReturn<int32_t, int32_t> WIFI_GetMode(int32_t ifcId);
  DBus::MultipleReturn<int32_t, int32_t> WIFI_GetMode_Async(int32_t ifcId);
  int32_t WIFI_SelectNetwork(int32_t ifcId, int32_t networkId);
  int32_t WIFI_SelectNetwork_Async(int32_t ifcId, int32_t networkId);
  DBus::MultipleReturn<int32_t, int32_t> WIFI_GetSelectNetwork(int32_t ifcId);
  DBus::MultipleReturn<int32_t, int32_t> WIFI_GetSelectNetwork_Async(int32_t ifcId);
  DBus::MultipleReturn<int32_t, int32_t> WIFI_AddNetwork(int32_t ifcId,
                                                         std::tuple<std::string,
                                                                    std::string,
                                                                    int32_t,
                                                                    bool,
                                                                    std::string> network);
  DBus::MultipleReturn<int32_t, int32_t> WIFI_AddNetwork_Async(int32_t ifcId,
                                                               std::tuple<std::string,
                                                                          std::string,
                                                                          int32_t,
                                                                          bool,
                                                                          std::string> network);
  DBus::MultipleReturn<int32_t, int32_t> WIFI_AddNetwork_Ex(int32_t ifcId, Invalid network);
  DBus::MultipleReturn<int32_t, int32_t> WIFI_AddNetwork_Ex_Async(int32_t ifcId, Invalid network);
  int32_t WIFI_RemoveConfiguredNetwork(int32_t ifcId, int32_t networkId);
  int32_t WIFI_RemoveConfiguredNetwork_Async(int32_t ifcId, int32_t networkId);
  int32_t WIFI_UpdateNetworkPass(int32_t ifcId, uint32_t networkId, std::string password);
  int32_t WIFI_UpdateNetworkPass_Async(int32_t ifcId, uint32_t networkId, std::string password);
  int32_t WIFI_UpdateConfiguredNetwork(int32_t ifcId, int32_t networkId, Invalid network);
  int32_t WIFI_UpdateNetwork_Async(int32_t ifcId, int32_t networkId, Invalid network);
  int32_t WIFI_SaveConfiguredNetworks(int32_t ifcId);
  int32_t WIFI_SaveConfiguredNetworks_Async(int32_t ifcId);
  int32_t WIFI_StartScan(int32_t ifcId, int32_t refreshTime);
  int32_t WIFI_StartScan_Async(int32_t ifcId, int32_t refreshTime);
  int32_t WIFI_StopScan(int32_t ifcId);
  int32_t WIFI_StopScan_Async(int32_t ifcId);
  DBus::MultipleReturn<int32_t,
                       std::tuple<std::vector<std::tuple<std::string,
                                                         std::string,
                                                         uint32_t,
                                                         uint32_t,
                                                         uint32_t,
                                                         uint32_t,
                                                         uint32_t,
                                                         uint32_t,
                                                         uint32_t,
                                                         int32_t>>>> WIFI_GetScanResults(int32_t ifcId);
  DBus::MultipleReturn<int32_t,
                       std::tuple<std::vector<std::tuple<std::string,
                                                         std::string,
                                                         uint32_t,
                                                         uint32_t,
                                                         uint32_t,
                                                         uint32_t,
                                                         uint32_t,
                                                         uint32_t,
                                                         uint32_t,
                                                         int32_t>>>> WIFI_GetScanResults_Async(int32_t ifcId);
  DBus::MultipleReturn<int32_t,
                       std::tuple<std::vector<std::tuple<uint32_t,
                                                         std::string,
                                                         std::string,
                                                         int32_t,
                                                         bool,
                                                         std::string,
                                                         int32_t>>>> WIFI_GetConfiguredNetworks(int32_t ifcId);
  DBus::MultipleReturn<int32_t,
                       std::tuple<std::vector<std::tuple<uint32_t,
                                                         std::string,
                                                         std::string,
                                                         int32_t,
                                                         bool,
                                                         std::string,
                                                         int32_t>>>> WIFI_GetConfiguredNetworks_Async(int32_t ifcId);
  DBus::MultipleReturn<int32_t, Invalid> WIFI_GetConfiguredNetworks_Ex(int32_t ifcId);
  DBus::MultipleReturn<int32_t, Invalid> WIFI_GetConfiguredNetworks_Ex_Async(int32_t ifcId);
  DBus::MultipleReturn<int32_t, int32_t, int32_t> WIFI_GetNetCount(int32_t ifcId);
  DBus::MultipleReturn<int32_t, int32_t, int32_t> WIFI_GetNetCount_Async(int32_t ifcId);
  DBus::MultipleReturn<int32_t,
                       std::tuple<uint32_t,
                                  std::tuple<std::string,
                                             std::string,
                                             uint32_t,
                                             uint32_t,
                                             uint32_t,
                                             uint32_t,
                                             uint32_t,
                                             uint32_t,
                                             uint32_t,
                                             int32_t>>> WIFI_GetConnectionInfo(int32_t ifcId);
  DBus::MultipleReturn<int32_t,
                       std::tuple<uint32_t,
                                  std::tuple<std::string,
                                             std::string,
                                             uint32_t,
                                             uint32_t,
                                             uint32_t,
                                             uint32_t,
                                             uint32_t,
                                             uint32_t,
                                             uint32_t,
                                             int32_t>>> WIFI_GetConnectionInfo_Async(int32_t ifcId);
  DBus::MultipleReturn<int32_t,
                       std::tuple<std::vector<std::tuple<uint32_t,
                                                         std::string,
                                                         std::string,
                                                         int32_t,
                                                         int32_t,
                                                         int32_t,
                                                         int32_t>>>> WIFI_GetCombinedNetworksList(int32_t ifcId);
  DBus::MultipleReturn<int32_t,
                       std::tuple<std::vector<std::tuple<uint32_t,
                                                         std::string,
                                                         std::string,
                                                         int32_t,
                                                         int32_t,
                                                         int32_t,
                                                         int32_t>>>> WIFI_GetCombinedNetworksList_Async(int32_t ifcId);
  int32_t AP_SelectConfiguration(int32_t secType);
  int32_t AP_SelectConfiguration_Async(int32_t secType);
  DBus::MultipleReturn<int32_t, int32_t> AP_GetSelectedConfiguration();
  DBus::MultipleReturn<int32_t, int32_t> AP_GetSelectedConfiguration_Async();
  int32_t AP_SetHostConfig(std::tuple<std::string, std::string, int32_t, bool> config);
  int32_t AP_SetHostConfig_Async(std::tuple<std::string, std::string, int32_t, bool> config);
  DBus::MultipleReturn<int32_t,
                       std::tuple<std::string, std::string, int32_t, bool>> AP_GetHostConfig(int32_t secSettings);
  DBus::MultipleReturn<int32_t,
                       std::tuple<std::string, std::string, int32_t, bool>> AP_GetHostConfig_Async(int32_t secSettings);
  DBus::MultipleReturn<int32_t,
                       std::tuple<std::vector<std::tuple<std::string,
                                                         std::string,
                                                         std::string,
                                                         std::string>>>> AP_GetClients(int32_t ifcId);
  DBus::MultipleReturn<int32_t,
                       std::tuple<std::vector<std::tuple<std::string,
                                                         std::string,
                                                         std::string,
                                                         std::string>>>> AP_GetClients_Async(int32_t ifcId);
  int32_t AP_AddToBlacklist(int32_t ifcId, std::string mac);
  int32_t AP_AddToBlacklist_Async(int32_t ifcId, std::string mac);
  int32_t AP_RemoveFromBlacklist(int32_t ifcId, std::string mac);
  int32_t AP_RemoveFromBlacklist_Async(int32_t ifcId, std::string mac);
  DBus::MultipleReturn<int32_t, std::tuple<std::vector<std::string>>> AP_GetBlacklist(int32_t ifcId);
  DBus::MultipleReturn<int32_t, std::tuple<std::vector<std::string>>> AP_GetBlacklist_Async(int32_t ifcId);
  int32_t AP_ClearBlacklist(int32_t ifcId);
  int32_t AP_ClearBlacklist_Async(int32_t ifcId);
  DBus::MultipleReturn<int32_t,
                       std::tuple<std::vector<std::tuple<int32_t,
                                                         int32_t,
                                                         std::string,
                                                         std::string,
                                                         std::string,
                                                         std::string,
                                                         std::string,
                                                         std::string,
                                                         std::string,
                                                         int32_t,
                                                         bool,
                                                         std::vector<std::string>>>>> Get_ModemList();
  DBus::MultipleReturn<int32_t,
                       std::tuple<std::vector<std::tuple<int32_t,
                                                         int32_t,
                                                         std::string,
                                                         std::string,
                                                         std::string,
                                                         std::string,
                                                         std::string,
                                                         std::string,
                                                         std::string,
                                                         int32_t,
                                                         bool,
                                                         std::vector<std::string>>>>> Get_ModemList_Async();
  DBus::MultipleReturn<int32_t,
                       std::tuple<int32_t,
                                  int32_t,
                                  std::string,
                                  std::string,
                                  std::string,
                                  std::string,
                                  std::string,
                                  std::string,
                                  std::string,
                                  int32_t,
                                  bool,
                                  std::vector<std::string>>> Get_ModemParams(int32_t mId);
  DBus::MultipleReturn<int32_t,
                       std::tuple<int32_t,
                                  int32_t,
                                  std::string,
                                  std::string,
                                  std::string,
                                  std::string,
                                  std::string,
                                  std::string,
                                  std::string,
                                  int32_t,
                                  bool,
                                  std::vector<std::string>>> Get_ModemParams_Async(int32_t mId);
  int32_t Set_ModemParams(int32_t mId,
                          std::tuple<std::string, std::string, std::string, std::string, std::string> params,
                          int32_t updateMask);
  int32_t Set_ModemParams_Async(int32_t mId,
                                std::tuple<std::string, std::string, std::string, std::string, std::string> params,
                                int32_t updateMask);
  int32_t AddModem(std::string modemId, std::string vendor, std::tuple<std::vector<std::string>> ports);
  int32_t RemoveModem(std::string modemId, std::string vendor);
  int32_t Debug_ModemDump();
  std::shared_ptr<DBus::SignalProxy<void(int32_t, int32_t, int32_t)>> signal_ConnectResult();
  std::shared_ptr<DBus::SignalProxy<void(int32_t)>> signal_InterfaceArrive();
  std::shared_ptr<DBus::SignalProxy<void(int32_t)>> signal_InterfaceDeparture();
  std::shared_ptr<DBus::SignalProxy<void(int32_t, int32_t)>> signal_InterfaceChanged();
  std::shared_ptr<DBus::SignalProxy<void(int32_t)>> signal_InterfaceConnecting();
  std::shared_ptr<DBus::SignalProxy<void(int32_t)>> signal_InterfaceDisconnecting();
  std::shared_ptr<DBus::SignalProxy<void(int32_t)>> signal_InterfaceConnected();
  std::shared_ptr<DBus::SignalProxy<void(int32_t, int32_t)>> signal_InterfaceDisconnected();
  std::shared_ptr<DBus::SignalProxy<void(int32_t)>> signal_WIFI_ScanResultsReady();
  std::shared_ptr<DBus::SignalProxy<void(int32_t, int32_t)>> signal_InterfaceWiFiModeChanged();
  std::shared_ptr<DBus::SignalProxy<void(int32_t, std::string)>> signal_WIFI_ApClientConnected();
  std::shared_ptr<DBus::SignalProxy<void(int32_t, std::string)>> signal_WIFI_ApClientDisconnected();
  std::shared_ptr<DBus::SignalProxy<void(int32_t)>> signal_ConnectionStateChanged();
  std::shared_ptr<DBus::SignalProxy<void(std::tuple<int32_t,
                                                    int32_t,
                                                    int32_t,
                                                    int32_t,
                                                    int32_t,
                                                    uint32_t>)>> signal_WifiStatus();
  std::shared_ptr<DBus::SignalProxy<void(std::tuple<int32_t>)>> signal_BLMStatus();
  std::shared_ptr<DBus::SignalProxy<void(std::tuple<int32_t,
                                                    int32_t,
                                                    int32_t,
                                                    uint64_t,
                                                    uint64_t>)>> signal_ConnectionStatus();
 protected:
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, int32_t>()>> m_method_GetConnectionState;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, int32_t>()>> m_method_GetConnectionState_Async;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, int32_t>(int32_t)>> m_method_Connect;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, int32_t>(int32_t)>> m_method_Connect_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t)>> m_method_Disconnect;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t)>> m_method_Disconnect_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t)>> m_method_Abort;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t)>> m_method_Abort_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(bool)>> m_method_SetWiFiAutoconnect;
  std::shared_ptr<DBus::MethodProxy<int32_t(bool)>> m_method_SetWiFiAutoconnect_Async;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, bool>()>> m_method_GetWiFiAutoconnect;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, bool>()>> m_method_GetWiFiAutoconnect_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(bool)>> m_method_SetModemAutoconnect;
  std::shared_ptr<DBus::MethodProxy<int32_t(bool)>> m_method_SetModemAutoconnect_Async;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, bool>()>> m_method_GetModemAutoconnect;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, bool>()>> m_method_GetModemAutoconnect_Async;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, std::tuple<std::vector<int32_t>>>()>>
      m_method_GetInterfaceList;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, std::tuple<std::vector<int32_t>>>()>>
      m_method_GetInterfaceList_Async;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t,
                                                         std::tuple<int32_t,
                                                                    int32_t,
                                                                    std::string,
                                                                    std::string,
                                                                    std::string,
                                                                    std::string,
                                                                    std::string,
                                                                    std::string,
                                                                    std::string,
                                                                    std::string,
                                                                    int32_t,
                                                                    int32_t,
                                                                    int32_t,
                                                                    int32_t>>(int32_t)>> m_method_GetInterfaceParams;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t,
                                                         std::tuple<int32_t,
                                                                    int32_t,
                                                                    std::string,
                                                                    std::string,
                                                                    std::string,
                                                                    std::string,
                                                                    std::string,
                                                                    std::string,
                                                                    std::string,
                                                                    std::string,
                                                                    int32_t,
                                                                    int32_t,
                                                                    int32_t,
                                                                    int32_t>>(int32_t)>>
      m_method_GetInterfaceParams_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t,
                                            std::tuple<std::string,
                                                       std::string,
                                                       std::string,
                                                       std::string,
                                                       std::string,
                                                       std::string,
                                                       int32_t,
                                                       int32_t>,
                                            int32_t)>> m_method_SetInterfaceParams;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t,
                                            std::tuple<std::string,
                                                       std::string,
                                                       std::string,
                                                       std::string,
                                                       std::string,
                                                       std::string,
                                                       int32_t,
                                                       int32_t>,
                                            int32_t)>> m_method_SetInterfaceParams_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t, std::tuple<std::string, std::string>)>>
      m_method_DHCP_ServerSetConfig;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t, std::tuple<std::string, std::string>)>>
      m_method_DHCP_ServerSetConfig_Async;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, std::tuple<std::string, std::string>>(int32_t)>>
      m_method_DHCP_ServerGetConfig;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, std::tuple<std::string, std::string>>(int32_t)>>
      m_method_DHCP_ServerGetConfig_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t)>> m_method_DHCP_ServerStart;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t)>> m_method_DHCP_ServerStart_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t)>> m_method_DHCP_ServerStop;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t)>> m_method_DHCP_ServerStop_Async;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t,
                                                         std::tuple<std::vector<std::tuple<std::string,
                                                                                           std::string,
                                                                                           std::string,
                                                                                           std::string>>>>(int32_t)>>
      m_method_DHCP_ServerGetClients;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t,
                                                         std::tuple<std::vector<std::tuple<std::string,
                                                                                           std::string,
                                                                                           std::string,
                                                                                           std::string>>>>(int32_t)>>
      m_method_DHCP_ServerGetClients_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(bool)>> m_method_ActivateNAT;
  std::shared_ptr<DBus::MethodProxy<int32_t(bool)>> m_method_ActivateNAT_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(bool)>> m_method_AdvancedRouting;
  std::shared_ptr<DBus::MethodProxy<int32_t(bool)>> m_method_AdvancedRouting_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(std::string)>> m_method_SetHostname;
  std::shared_ptr<DBus::MethodProxy<int32_t(std::string)>> m_method_SetHostname_Async;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, std::string>()>> m_method_GetHostname;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, std::string>()>> m_method_GetHostname_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t, bool)>> m_method_MonitorInterface;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t, bool)>> m_method_MonitorInterface_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t, int32_t)>> m_method_WIFI_SetMode;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t, int32_t)>> m_method_WIFI_SetMode_Async;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, int32_t>(int32_t)>> m_method_WIFI_GetMode;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, int32_t>(int32_t)>> m_method_WIFI_GetMode_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t, int32_t)>> m_method_WIFI_SelectNetwork;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t, int32_t)>> m_method_WIFI_SelectNetwork_Async;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, int32_t>(int32_t)>> m_method_WIFI_GetSelectNetwork;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, int32_t>(int32_t)>>
      m_method_WIFI_GetSelectNetwork_Async;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, int32_t>(int32_t,
                                                                           std::tuple<std::string,
                                                                                      std::string,
                                                                                      int32_t,
                                                                                      bool,
                                                                                      std::string>)>>
      m_method_WIFI_AddNetwork;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, int32_t>(int32_t,
                                                                           std::tuple<std::string,
                                                                                      std::string,
                                                                                      int32_t,
                                                                                      bool,
                                                                                      std::string>)>>
      m_method_WIFI_AddNetwork_Async;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, int32_t>(int32_t, Invalid)>>
      m_method_WIFI_AddNetwork_Ex;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, int32_t>(int32_t, Invalid)>>
      m_method_WIFI_AddNetwork_Ex_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t, int32_t)>> m_method_WIFI_RemoveConfiguredNetwork;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t, int32_t)>> m_method_WIFI_RemoveConfiguredNetwork_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t, uint32_t, std::string)>> m_method_WIFI_UpdateNetworkPass;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t, uint32_t, std::string)>> m_method_WIFI_UpdateNetworkPass_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t, int32_t, Invalid)>> m_method_WIFI_UpdateConfiguredNetwork;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t, int32_t, Invalid)>> m_method_WIFI_UpdateNetwork_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t)>> m_method_WIFI_SaveConfiguredNetworks;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t)>> m_method_WIFI_SaveConfiguredNetworks_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t, int32_t)>> m_method_WIFI_StartScan;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t, int32_t)>> m_method_WIFI_StartScan_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t)>> m_method_WIFI_StopScan;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t)>> m_method_WIFI_StopScan_Async;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t,
                                                         std::tuple<std::vector<std::tuple<std::string,
                                                                                           std::string,
                                                                                           uint32_t,
                                                                                           uint32_t,
                                                                                           uint32_t,
                                                                                           uint32_t,
                                                                                           uint32_t,
                                                                                           uint32_t,
                                                                                           uint32_t,
                                                                                           int32_t>>>>(int32_t)>>
      m_method_WIFI_GetScanResults;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t,
                                                         std::tuple<std::vector<std::tuple<std::string,
                                                                                           std::string,
                                                                                           uint32_t,
                                                                                           uint32_t,
                                                                                           uint32_t,
                                                                                           uint32_t,
                                                                                           uint32_t,
                                                                                           uint32_t,
                                                                                           uint32_t,
                                                                                           int32_t>>>>(int32_t)>>
      m_method_WIFI_GetScanResults_Async;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t,
                                                         std::tuple<std::vector<std::tuple<uint32_t,
                                                                                           std::string,
                                                                                           std::string,
                                                                                           int32_t,
                                                                                           bool,
                                                                                           std::string,
                                                                                           int32_t>>>>(int32_t)>>
      m_method_WIFI_GetConfiguredNetworks;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t,
                                                         std::tuple<std::vector<std::tuple<uint32_t,
                                                                                           std::string,
                                                                                           std::string,
                                                                                           int32_t,
                                                                                           bool,
                                                                                           std::string,
                                                                                           int32_t>>>>(int32_t)>>
      m_method_WIFI_GetConfiguredNetworks_Async;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, Invalid>(int32_t)>>
      m_method_WIFI_GetConfiguredNetworks_Ex;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, Invalid>(int32_t)>>
      m_method_WIFI_GetConfiguredNetworks_Ex_Async;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, int32_t, int32_t>(int32_t)>>
      m_method_WIFI_GetNetCount;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, int32_t, int32_t>(int32_t)>>
      m_method_WIFI_GetNetCount_Async;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t,
                                                         std::tuple<uint32_t,
                                                                    std::tuple<std::string,
                                                                               std::string,
                                                                               uint32_t,
                                                                               uint32_t,
                                                                               uint32_t,
                                                                               uint32_t,
                                                                               uint32_t,
                                                                               uint32_t,
                                                                               uint32_t,
                                                                               int32_t>>>(int32_t)>>
      m_method_WIFI_GetConnectionInfo;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t,
                                                         std::tuple<uint32_t,
                                                                    std::tuple<std::string,
                                                                               std::string,
                                                                               uint32_t,
                                                                               uint32_t,
                                                                               uint32_t,
                                                                               uint32_t,
                                                                               uint32_t,
                                                                               uint32_t,
                                                                               uint32_t,
                                                                               int32_t>>>(int32_t)>>
      m_method_WIFI_GetConnectionInfo_Async;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t,
                                                         std::tuple<std::vector<std::tuple<uint32_t,
                                                                                           std::string,
                                                                                           std::string,
                                                                                           int32_t,
                                                                                           int32_t,
                                                                                           int32_t,
                                                                                           int32_t>>>>(int32_t)>>
      m_method_WIFI_GetCombinedNetworksList;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t,
                                                         std::tuple<std::vector<std::tuple<uint32_t,
                                                                                           std::string,
                                                                                           std::string,
                                                                                           int32_t,
                                                                                           int32_t,
                                                                                           int32_t,
                                                                                           int32_t>>>>(int32_t)>>
      m_method_WIFI_GetCombinedNetworksList_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t)>> m_method_AP_SelectConfiguration;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t)>> m_method_AP_SelectConfiguration_Async;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, int32_t>()>> m_method_AP_GetSelectedConfiguration;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, int32_t>()>>
      m_method_AP_GetSelectedConfiguration_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(std::tuple<std::string, std::string, int32_t, bool>)>>
      m_method_AP_SetHostConfig;
  std::shared_ptr<DBus::MethodProxy<int32_t(std::tuple<std::string, std::string, int32_t, bool>)>>
      m_method_AP_SetHostConfig_Async;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, std::tuple<std::string, std::string, int32_t, bool>>(
      int32_t)>> m_method_AP_GetHostConfig;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, std::tuple<std::string, std::string, int32_t, bool>>(
      int32_t)>> m_method_AP_GetHostConfig_Async;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t,
                                                         std::tuple<std::vector<std::tuple<std::string,
                                                                                           std::string,
                                                                                           std::string,
                                                                                           std::string>>>>(int32_t)>>
      m_method_AP_GetClients;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t,
                                                         std::tuple<std::vector<std::tuple<std::string,
                                                                                           std::string,
                                                                                           std::string,
                                                                                           std::string>>>>(int32_t)>>
      m_method_AP_GetClients_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t, std::string)>> m_method_AP_AddToBlacklist;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t, std::string)>> m_method_AP_AddToBlacklist_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t, std::string)>> m_method_AP_RemoveFromBlacklist;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t, std::string)>> m_method_AP_RemoveFromBlacklist_Async;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, std::tuple<std::vector<std::string>>>(int32_t)>>
      m_method_AP_GetBlacklist;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t, std::tuple<std::vector<std::string>>>(int32_t)>>
      m_method_AP_GetBlacklist_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t)>> m_method_AP_ClearBlacklist;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t)>> m_method_AP_ClearBlacklist_Async;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t,
                                                         std::tuple<std::vector<std::tuple<int32_t,
                                                                                           int32_t,
                                                                                           std::string,
                                                                                           std::string,
                                                                                           std::string,
                                                                                           std::string,
                                                                                           std::string,
                                                                                           std::string,
                                                                                           std::string,
                                                                                           int32_t,
                                                                                           bool,
                                                                                           std::vector<std::string>>>>>()>>
      m_method_Get_ModemList;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t,
                                                         std::tuple<std::vector<std::tuple<int32_t,
                                                                                           int32_t,
                                                                                           std::string,
                                                                                           std::string,
                                                                                           std::string,
                                                                                           std::string,
                                                                                           std::string,
                                                                                           std::string,
                                                                                           std::string,
                                                                                           int32_t,
                                                                                           bool,
                                                                                           std::vector<std::string>>>>>()>>
      m_method_Get_ModemList_Async;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t,
                                                         std::tuple<int32_t,
                                                                    int32_t,
                                                                    std::string,
                                                                    std::string,
                                                                    std::string,
                                                                    std::string,
                                                                    std::string,
                                                                    std::string,
                                                                    std::string,
                                                                    int32_t,
                                                                    bool,
                                                                    std::vector<std::string>>>(int32_t)>>
      m_method_Get_ModemParams;
  std::shared_ptr<DBus::MethodProxy<DBus::MultipleReturn<int32_t,
                                                         std::tuple<int32_t,
                                                                    int32_t,
                                                                    std::string,
                                                                    std::string,
                                                                    std::string,
                                                                    std::string,
                                                                    std::string,
                                                                    std::string,
                                                                    std::string,
                                                                    int32_t,
                                                                    bool,
                                                                    std::vector<std::string>>>(int32_t)>>
      m_method_Get_ModemParams_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t,
                                            std::tuple<std::string, std::string, std::string, std::string, std::string>,
                                            int32_t)>> m_method_Set_ModemParams;
  std::shared_ptr<DBus::MethodProxy<int32_t(int32_t,
                                            std::tuple<std::string, std::string, std::string, std::string, std::string>,
                                            int32_t)>> m_method_Set_ModemParams_Async;
  std::shared_ptr<DBus::MethodProxy<int32_t(std::string, std::string, std::tuple<std::vector<std::string>>)>>
      m_method_AddModem;
  std::shared_ptr<DBus::MethodProxy<int32_t(std::string, std::string)>> m_method_RemoveModem;
  std::shared_ptr<DBus::MethodProxy<int32_t()>> m_method_Debug_ModemDump;
  std::shared_ptr<DBus::SignalProxy<void(int32_t, int32_t, int32_t)>> m_signalproxy_ConnectResult;
  std::shared_ptr<DBus::SignalProxy<void(int32_t)>> m_signalproxy_InterfaceArrive;
  std::shared_ptr<DBus::SignalProxy<void(int32_t)>> m_signalproxy_InterfaceDeparture;
  std::shared_ptr<DBus::SignalProxy<void(int32_t, int32_t)>> m_signalproxy_InterfaceChanged;
  std::shared_ptr<DBus::SignalProxy<void(int32_t)>> m_signalproxy_InterfaceConnecting;
  std::shared_ptr<DBus::SignalProxy<void(int32_t)>> m_signalproxy_InterfaceDisconnecting;
  std::shared_ptr<DBus::SignalProxy<void(int32_t)>> m_signalproxy_InterfaceConnected;
  std::shared_ptr<DBus::SignalProxy<void(int32_t, int32_t)>> m_signalproxy_InterfaceDisconnected;
  std::shared_ptr<DBus::SignalProxy<void(int32_t)>> m_signalproxy_WIFI_ScanResultsReady;
  std::shared_ptr<DBus::SignalProxy<void(int32_t, int32_t)>> m_signalproxy_InterfaceWiFiModeChanged;
  std::shared_ptr<DBus::SignalProxy<void(int32_t, std::string)>> m_signalproxy_WIFI_ApClientConnected;
  std::shared_ptr<DBus::SignalProxy<void(int32_t, std::string)>> m_signalproxy_WIFI_ApClientDisconnected;
  std::shared_ptr<DBus::SignalProxy<void(int32_t)>> m_signalproxy_ConnectionStateChanged;
  std::shared_ptr<DBus::SignalProxy<void(std::tuple<int32_t, int32_t, int32_t, int32_t, int32_t, uint32_t>)>>
      m_signalproxy_WifiStatus;
  std::shared_ptr<DBus::SignalProxy<void(std::tuple<int32_t>)>> m_signalproxy_BLMStatus;
  std::shared_ptr<DBus::SignalProxy<void(std::tuple<int32_t, int32_t, int32_t, uint64_t, uint64_t>)>>
      m_signalproxy_ConnectionStatus;
};
#endif /* COM_JCI_BLM_NMSPROXY_H */
