#include <set>
#include <dbus-cxx.h>
#include <nlohmann/json.hpp>

#include "autoapp/Managers/IAudioManager.hpp"

#include <com_xsembedded_ServiceProvider_objectProxy.h>
#include <aasdk/IO/Promise.hpp>


#include <thread>
#include <mutex>
#include <deque>

using json = nlohmann::json;
using AudioFocusState = aasdk::proto::enums::AudioFocusState;
using AudioFocusType = aasdk::proto::enums::AudioFocusType_Enum;

struct Stream {
  std::string name;
  int id;
  aasdk::messenger::ChannelId channelId;
  bool focus = false;
  std::string mode;
  std::string type;
};

class AudioManager: public IAudioManager {
 private:
  struct workItem {
    aasdk::messenger::ChannelId channelId;
    std::string focus;
  };
  std::deque<workItem> workQueue;
  std::mutex queueLock;
  void doWork();

  std::vector<std::string> MazdaDestinations;
  std::map<aasdk::messenger::ChannelId, Stream *> streams;
//  std::map<std::string, int> ExistingStreams;
  std::map<int, Stream *> streamsByID;

  std::shared_ptr<DBus::Connection> dbusConnection;
  std::shared_ptr<com_xsembedded_ServiceProvider_objectProxy> AudioInterface;
  std::shared_ptr<com_xsembedded_ServiceProviderProxy> AudioProxy;
  std::shared_ptr<DBus::Object> AudioObject;
  std::shared_ptr<DBus::Interface> AudioRequestInterface;

  sigc::connection notifyConnection;

  std::shared_ptr<DBus::ObjectProxy> object;

  std::mutex AudioMutex;

  asio::io_service::strand strand_;
  asio::basic_waitable_timer<std::chrono::steady_clock> timer_;


  void RegisterStream(std::string StreamName,
                      aasdk::messenger::ChannelId ChannelId,
                      std::string StreamMode,
                      std::string StreamType,
                      std::string StreamModeName);

  void populateData();

//  void populateStreamTable();

  std::string RequestHandler(const std::string &methodName, const std::string &arguments);

 public:
  AudioManager(std::shared_ptr<DBus::Connection> , asio::io_service &ioService);

  ~AudioManager() override = default;

  void start() override;
  void stop() override;

  void requestFocus(aasdk::messenger::ChannelId channelId, aasdk::proto::enums::AudioFocusType_Enum aa_type) override;

  void releaseFocus(aasdk::messenger::ChannelId channelId) override;

  static void onNotify(const std::string &signalName, const std::string &payload);

  void onRequestAudioFocusResult(json result, Stream *stream);

  void onAudioFocusChange(json result, Stream *stream);

  focusType getFocusType(aasdk::messenger::ChannelId channelId) override;

};