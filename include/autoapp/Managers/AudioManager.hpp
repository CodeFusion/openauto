#include <set>
#include <dbus-cxx.h>
#include <nlohmann/json.hpp>

#include <autoapp/Signals/AudioSignals.hpp>

#include <com_xsembedded_ServiceProvider_objectProxy.h>

#include <thread>
#include <mutex>

using json = nlohmann::json;
using AudioFocusState = aasdk::proto::enums::AudioFocusState;

struct Stream {
  std::string name;
  int id;
  aasdk::messenger::ChannelId channelId;
  bool focus = false;
  std::string mode;
  std::string type;
};

class AudioManagerClient {
 private:
  AudioSignals::Pointer audiosignals_;
  bool inCall = false;
  std::vector<std::string> MazdaDestinations;
  std::map<aasdk::messenger::ChannelId, Stream *> streams;
//  std::map<std::string, int> ExistingStreams;
  std::map<int, Stream *> streamsByID;

  std::shared_ptr<com_xsembedded_ServiceProvider_objectProxy> AudioInterface;
  std::shared_ptr<com_xsembedded_ServiceProviderProxy> AudioProxy;
  std::shared_ptr<DBus::Object> AudioObject;

  std::shared_ptr<DBus::Connection> connection;

  std::shared_ptr<DBus::ObjectProxy> object;

  std::mutex AudioMutex;

  void RegisterStream(std::string StreamName,
                      aasdk::messenger::ChannelId ChannelId,
                      std::string StreamMode,
                      std::string StreamType);

  void populateData();

//  void populateStreamTable();

  std::string RequestHandler(const std::string &methodName, const std::string &arguments);

 public:
  AudioManagerClient(AudioSignals::Pointer audiosignals, const std::shared_ptr<DBus::Connection> &);

  ~AudioManagerClient();

  //calling requestAudioFocus directly doesn't work on the audio mgr
  void audioMgrRequestAudioFocus(aasdk::messenger::ChannelId, aasdk::proto::enums::AudioFocusType_Enum aa_type);

  void audioMgrReleaseAudioFocus(aasdk::messenger::ChannelId);

  static void onNotify(const std::string &signalName, const std::string &payload);

  void onRequestAudioFocusResult(json result, Stream *stream);

  void onAudioFocusChange(json result, Stream *stream);

};