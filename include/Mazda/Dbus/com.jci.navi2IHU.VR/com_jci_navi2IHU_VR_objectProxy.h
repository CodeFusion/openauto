#ifndef COM_JCI_NAVI_IHU_VR_OBJECTPROXY_H
#define COM_JCI_NAVI_IHU_VR_OBJECTPROXY_H

#include <dbus-cxx.h>
#include <memory>
#include <stdint.h>
#include <string>
#include "com_jci_navi2IHU_VRProxy.h"
class com_jci_navi2IHU_VR_objectProxy
    : public DBus::ObjectProxy {
 public:
  com_jci_navi2IHU_VR_objectProxy(std::shared_ptr<DBus::Connection> conn, std::string dest, std::string path);
 public:
  static std::shared_ptr<com_jci_navi2IHU_VR_objectProxy> create(std::shared_ptr<DBus::Connection> conn,
                                                                 std::string dest,
                                                                 std::string path,
                                                                 DBus::ThreadForCalling signalCallingThread = DBus::ThreadForCalling::DispatcherThread);
  std::shared_ptr<com_jci_navi2IHU_VRProxy> getcom_jci_navi2IHU_VRInterface();
 protected:
  std::shared_ptr<com_jci_navi2IHU_VRProxy> m_com_jci_navi2IHU_VRProxy;
};
#endif /* COM_JCI_NAVI_IHU_VR_OBJECTPROXY_H */
