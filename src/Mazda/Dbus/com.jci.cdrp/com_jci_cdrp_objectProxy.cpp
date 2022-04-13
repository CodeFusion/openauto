#include "com_jci_cdrp_objectProxy.h"

com_jci_cdrp_objectProxy::com_jci_cdrp_objectProxy(std::shared_ptr <DBus::Connection> conn,
                                                   std::string dest,
                                                   std::string path) : DBus::ObjectProxy(conn, dest, path) {
  m_com_jci_cdrpProxy = com_jci_cdrpProxy::create("com.jci.cdrp");
  this->add_interface(m_com_jci_cdrpProxy);

}
std::shared_ptr <com_jci_cdrp_objectProxy> com_jci_cdrp_objectProxy::create(std::shared_ptr <DBus::Connection> conn,
                                                                            std::string dest,
                                                                            std::string path,
                                                                            DBus::ThreadForCalling signalCallingThread) {
  std::shared_ptr <com_jci_cdrp_objectProxy>
      created = std::shared_ptr<com_jci_cdrp_objectProxy>(new com_jci_cdrp_objectProxy(conn, dest, path));
  conn->register_object_proxy(created, signalCallingThread);
  return created;

}
std::shared_ptr <com_jci_cdrpProxy> com_jci_cdrp_objectProxy::getcom_jci_cdrpInterface() {
  return m_com_jci_cdrpProxy;

}
