#ifndef COM_JCI_CD_OBJECTPROXY_H
#define COM_JCI_CD_OBJECTPROXY_H

#include <dbus-cxx.h>
#include <memory>
#include <stdint.h>
#include <string>
#include "com_jci_CDProxy.h"
class com_jci_CD_objectProxy
    : public DBus::ObjectProxy {
 public:
  com_jci_CD_objectProxy(std::shared_ptr<DBus::Connection> conn, std::string dest, std::string path);
 public:
  static std::shared_ptr<com_jci_CD_objectProxy> create(std::shared_ptr<DBus::Connection> conn,
                                                        std::string dest,
                                                        std::string path,
                                                        DBus::ThreadForCalling signalCallingThread = DBus::ThreadForCalling::DispatcherThread);
  std::shared_ptr<com_jci_CDProxy> getcom_jci_CDInterface();
 protected:
  std::shared_ptr<com_jci_CDProxy> m_com_jci_CDProxy;
};
#endif /* COM_JCI_CD_OBJECTPROXY_H */
