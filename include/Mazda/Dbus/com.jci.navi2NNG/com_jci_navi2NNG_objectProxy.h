#ifndef COM_JCI_NAVI_NNG_OBJECTPROXY_H
#define COM_JCI_NAVI_NNG_OBJECTPROXY_H

#include <dbus-cxx.h>
#include <memory>
#include <stdint.h>
#include <string>
#include "com_jci_navi2NNGProxy.h"
class com_jci_navi2NNG_objectProxy
    : public DBus::ObjectProxy {
 public:
  com_jci_navi2NNG_objectProxy(std::shared_ptr<DBus::Connection> conn, std::string dest, std::string path);
 public:
  static std::shared_ptr<com_jci_navi2NNG_objectProxy> create(std::shared_ptr<DBus::Connection> conn,
                                                              std::string dest,
                                                              std::string path,
                                                              DBus::ThreadForCalling signalCallingThread = DBus::ThreadForCalling::DispatcherThread);
  std::shared_ptr<com_jci_navi2NNGProxy> getcom_jci_navi2NNGInterface();
 protected:
  std::shared_ptr<com_jci_navi2NNGProxy> m_com_jci_navi2NNGProxy;
};
#endif /* COM_JCI_NAVI_NNG_OBJECTPROXY_H */
