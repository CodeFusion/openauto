#ifndef COM_JCI_BUCPSA_OBJECTPROXY_H
#define COM_JCI_BUCPSA_OBJECTPROXY_H

#include <dbus-cxx.h>
#include <memory>
#include <stdint.h>
#include <string>
#include "com_jci_bucpsaProxy.h"
class com_jci_bucpsa_objectProxy
    : public DBus::ObjectProxy {
 public:
  com_jci_bucpsa_objectProxy(std::shared_ptr<DBus::Connection> conn, std::string dest, std::string path);
 public:
  static std::shared_ptr<com_jci_bucpsa_objectProxy> create(std::shared_ptr<DBus::Connection> conn,
                                                            std::string dest,
                                                            std::string path,
                                                            DBus::ThreadForCalling signalCallingThread = DBus::ThreadForCalling::DispatcherThread);
  std::shared_ptr<com_jci_bucpsaProxy> getcom_jci_bucpsaInterface();
 protected:
  std::shared_ptr<com_jci_bucpsaProxy> m_com_jci_bucpsaProxy;
};
#endif /* COM_JCI_BUCPSA_OBJECTPROXY_H */
