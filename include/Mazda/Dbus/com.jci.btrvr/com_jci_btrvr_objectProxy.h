#ifndef COM_JCI_BTRVR_OBJECTPROXY_H
#define COM_JCI_BTRVR_OBJECTPROXY_H

#include <dbus-cxx.h>
#include <memory>
#include <stdint.h>
#include <string>
#include "com_jci_btrvrProxy.h"
class com_jci_btrvr_objectProxy
    : public DBus::ObjectProxy {
 public:
  com_jci_btrvr_objectProxy(std::shared_ptr<DBus::Connection> conn, std::string dest, std::string path);
 public:
  static std::shared_ptr<com_jci_btrvr_objectProxy> create(std::shared_ptr<DBus::Connection> conn,
                                                           std::string dest,
                                                           std::string path,
                                                           DBus::ThreadForCalling signalCallingThread = DBus::ThreadForCalling::DispatcherThread);
  std::shared_ptr<com_jci_btrvrProxy> getcom_jci_btrvrInterface();
 protected:
  std::shared_ptr<com_jci_btrvrProxy> m_com_jci_btrvrProxy;
};
#endif /* COM_JCI_BTRVR_OBJECTPROXY_H */
