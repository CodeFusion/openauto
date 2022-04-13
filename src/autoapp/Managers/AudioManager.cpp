#include <autoapp/Managers/AudioManager.hpp>
#include <nlohmann/json.hpp>
#include <easylogging++.h>
#include <thread>

using json = nlohmann::json;
using AudioFocusState = aasdk::proto::enums::AudioFocusState;

std::string AudioManagerClient::RequestHandler(std::string methodName, std::string arguments) {
  VLOG(9) << methodName << " " << arguments;
  auto result = json::parse(arguments);
  if (streamsByID.count(result["sessionId"].get<int>())) {
    auto stream = streamsByID[result["sessionId"].get<int>()];
    if (methodName == "onRequestAudioFocusResult") {
      if (result["newFocus"].get<std::string>() == "granted") {
        json activeargs = {
            {"sessionId", stream->id},
            {"playing", true}
        };
        AudioProxy->Request("audioActive", activeargs.dump());
        LOG(DEBUG) << "Stream " << stream->id << ": " << stream->name << " Focus Granted";
      } else {
//        stream->focus = false;
//        audiosignals_->focusChanged.emit(stream->channelId, AudioFocusState::NONE);
      }
    } else if (methodName == "onAudioFocusChange") {
      auto focus = result["newFocus"].get<std::string>();
      if (focus == "lost") {
        stream->focus = false;
        json activeargs = {
            {"sessionId", stream->id},
            {"playing", false}
        };
        AudioProxy->Request("audioActive", activeargs.dump());
        if (stream->channelId == aasdk::messenger::ChannelId::MEDIA_AUDIO)
          audiosignals_->focusChanged.emit(stream->channelId, AudioFocusState::LOSS);
        LOG(DEBUG) << "Stream " << stream->id << ": " << stream->name << " Focus Lost";
      } else if (focus == "temporarilyLost") {
        stream->focus = false;
        json activeargs = {
            {"sessionId", stream->id},
            {"playing", false}
        };
        AudioProxy->Request("audioActive", activeargs.dump());
        if (stream->channelId == aasdk::messenger::ChannelId::MEDIA_AUDIO)
          audiosignals_->focusChanged.emit(stream->channelId, AudioFocusState::LOSS_TRANSIENT);
        LOG(DEBUG) << "Stream " << stream->id << ": " << stream->name << " Focus Temporarily Lost";
      } else if (result["newFocus"].get<std::string>() == "gained") {
        stream->focus = true;
        switch (stream->channelId) {
          case aasdk::messenger::ChannelId::MEDIA_AUDIO:
            audiosignals_->focusChanged.emit(stream->channelId,
                                             AudioFocusState::GAIN_MEDIA_ONLY);
            break;
          case aasdk::messenger::ChannelId::SPEECH_AUDIO:
//            audiosignals_->focusChanged.emit(stream->channelId, AudioFocusState::GAIN_TRANSIENT_GUIDANCE_ONLY);
//            break;
          case aasdk::messenger::ChannelId::SYSTEM_AUDIO:
            audiosignals_->focusChanged.emit(stream->channelId,
                                             AudioFocusState::GAIN_TRANSIENT);
            break;
          default:break;
        }
        LOG(DEBUG) << "Stream " << stream->id << ": " << stream->name << " Focus Gained";
      }
    }
  }
  return "";
}

void AudioManagerClient::RegisterStream(std::string StreamName,
                                        aasdk::messenger::ChannelId ChannelId,
                                        std::string StreamMode,
                                        std::string StreamType) {
  if (!(StreamMode == "permanent" || StreamMode == "transient")) {
    return;
  }
  if (std::find(MazdaDestinations.begin(), MazdaDestinations.end(), StreamType) == MazdaDestinations.end()) {
    return;
  }
  // First open a new Stream
  json sessArgs = {
      {"busName", "com.androidauto"},
      {"objectPath", "/com/androidauto/audio"},
      {"destination", "Cabin"}
  };
  try {
    std::string sessString = AudioProxy->Request("openSession", sessArgs.dump());
    VLOG(9) << "openSession(" << sessArgs.dump().c_str() << ")\n" << sessString.c_str() << "\n";
    int SessionID = json::parse(sessString)["sessionId"];

    // Register the stream
    json regArgs = {
        {"sessionId", SessionID},
        {"streamName", StreamName},
        // { "streamModeName", aaStreamName },
        {"focusType", StreamMode},
        {"streamType", StreamType}
    };
    std::string regString = AudioProxy->Request("registerAudioStream", regArgs.dump());
    VLOG(9) << "registerAudioStream(" << regArgs.dump().c_str() << ")\n" << regString.c_str() << "\n";
    // Stream is registered add it to the array
    auto stream = new Stream;
    stream->name.assign(StreamName);
    stream->id = SessionID;
    stream->mode = StreamMode;
    stream->type = StreamType;
    stream->channelId = ChannelId;
    streams.insert(std::pair<aasdk::messenger::ChannelId, Stream *>(ChannelId, stream));
    streamsByID.insert(std::pair<int, Stream *>(SessionID, stream));
  }
  catch (const std::domain_error &ex) {
    LOG(ERROR) << "Failed to parse state json: " << ex.what();
  }
  catch (const std::invalid_argument &ex) {
    LOG(ERROR) << "Failed to parse state json: " << ex.what();
  }

}

void AudioManagerClient::populateData() {
  json requestArgs = {
      {"svc", "DEST"},
      {"pretty", false}
  };
  std::string resultString = AudioProxy->Request("dumpState", requestArgs.dump());
  LOG(DEBUG) << "dumpState(" << requestArgs.dump().c_str() << ")\n" << resultString.c_str() << "\n";
  try {
    auto result = json::parse(resultString);
    for (json::iterator it = result["Cabin"]["reqMatrixIdx"].begin(); it != result["Cabin"]["reqMatrixIdx"].end();
         ++it) {
      MazdaDestinations.emplace_back(it.key());
    }
  }
  catch (const std::domain_error &ex) {
    LOG(ERROR) << "Failed to parse state json: " << ex.what();
    LOG(ERROR) << resultString.c_str();
  }
  catch (const std::invalid_argument &ex) {
    LOG(ERROR) << "Failed to parse state json: " << ex.what();
    LOG(ERROR) << resultString.c_str();
  }
  for (auto &Dest : MazdaDestinations)
    VLOG(9) << Dest;
}

void AudioManagerClient::populateStreamTable() {
  json requestArgs = {
      {"svc", "SRCS"},
      {"pretty", false}
  };
  // dbus-send --print-reply --type=method_call --address=unix:path=/tmp/dbus_service_socket --dest=com.xsembedded.service.AudioManagement /com/xse/service/AudioManagement/AudioApplication com.xsembedded.ServiceProvider.Request string:"dumpState" string:'{"svc":"SRCS", "pretty":false}'
//  method return sender=:1.40 -> dest=:1.173 reply_serial=2
//  string "{"HMI":["1..\/com\/jci\/audio\/am_hmi_client"],"APP":["29.CPAltAudio.CPALTAUDIO..NotPlaying","1.Media.AUX..NotPlaying","3.Media.DAB..NotPlaying","2.Media.CD..NotPlaying","5.Media.FM.gained.playing","4.Media.AM..NotPlaying","7.Media.HAR..NotPlaying","6.LowAlert.TA..NotPlaying","9.Media.XM..NotPlaying","8.Media.LM..NotPlaying","27.CPAlerts.CPALERTS..NotPlaying","17.Media.TV..NotPlaying","13.Media.BTMUSIC..NotPlaying","21.Media.USB..NotPlaying","11.InfoMix.NAVIVOL..NotPlaying","23.CPMedia.CPDEFAULTIN..NotPlaying","34.InfoUser.SMS..NotPlaying","28.CPSiri.CPSIRI..NotPlaying","33.InfoUser.PHONEBOOK..NotPlaying","32.HighAlert.BTECAALERT..NotPlaying","31.MP911.BTECA..NotPlaying","30.VR.SYSVR..NotPlaying","15.IncomingRg.BTHFALERT..NotPlaying","18.Media.Stitcher..NotPlaying","19.Media.AHA..NotPlaying","25.CPInCall.CPINCALL..NotPlaying","14.Telephony.BTHF..NotPlaying","24.CPMedia.CPMEDIA..NotPlaying","16.Media.DVD..NotPlaying","26.CPTel.CPTELEPHONY..NotPlaying","20.Media.Pandora..NotPlaying","12.InfoUser.RINGTONEVOL..NotPlaying","22.CPMedia.CPDEFAULT..NotPlaying","10.InfoUser.BTHFVOL..NotPlaying"]}"

  std::string resultString = AudioProxy->Request("dumpState", requestArgs.dump());
  VLOG(9) << "dumpState(" << requestArgs.dump().c_str() << ")\n" << resultString.c_str() << "\n";
  /*
       * An example resonse:
       *
      {
        "HMI": {
        },
        "APP": [
          "1.Media.Pandora.granted.NotPlaying",
          "2.Media.AM..NotPlaying"
        ]
      }
      */
  //Row format:
  //"%d.%s.%s.%s.%s", obj.sessionId, obj.stream.streamType, obj.stream.streamName, obj.focus, obj.stream.playing and "playing" or "NotPlaying")

  try {
    auto result = json::parse(resultString);
    for (auto &sessionRecord : result["APP"].get_ref<json::array_t &>()) {
      auto sessionStr = sessionRecord.get<std::string>();
      //Stream names have no spaces, so it's safe to do this
      std::replace(sessionStr.begin(), sessionStr.end(), '.', ' ');
      std::istringstream sessionIStr(sessionStr);

      int sessionId;
      std::string streamName, streamType;

      if (!(sessionIStr >> sessionId >> streamType >> streamName)) {
        LOG(WARNING) << "Can't parse line \"" << sessionRecord.get<std::string>().c_str() << "\"";
        continue;
      }

      VLOG(9) << "Found stream " << streamName.c_str() << " session id " << sessionId;
      ExistingStreams.insert(std::pair<std::string, int>(streamName, sessionId));
    }
  }
  catch (const std::domain_error &ex) {
    LOG(ERROR) << "Failed to parse state json: " << ex.what();
    LOG(ERROR) << resultString.c_str();
  }
  catch (const std::invalid_argument &ex) {
    LOG(ERROR) << "Failed to parse state json: " << ex.what();
    LOG(ERROR) << resultString.c_str();
  }
}

AudioManagerClient::AudioManagerClient(AudioSignals::Pointer audiosignals,
                                       const std::shared_ptr<DBus::Connection> &session_connection)
    : audiosignals_(std::move(audiosignals)) {
  session_connection->request_name("com.androidauto", DBUSCXX_NAME_FLAG_REPLACE_EXISTING);
  AudioObject = session_connection->create_object("/com/androidauto/audio", DBus::ThreadForCalling::DispatcherThread);
  std::shared_ptr<DBus::Interface>
      AudioRequestInterface = AudioObject->create_interface("com.xsembedded.ServiceProvider");
  AudioRequestInterface->create_method<std::string(std::string, std::string)>("Request",
                                                                              sigc::mem_fun(*this,
                                                                                            &AudioManagerClient::RequestHandler));

  AudioInterface = com_xsembedded_ServiceProvider_objectProxy::create(session_connection,
                                                                      "com.xsembedded.service.AudioManagement",
                                                                      "/com/xse/service/AudioManagement/AudioApplication");
  AudioProxy = AudioInterface->getcom_xsembedded_ServiceProviderInterface();

  populateData();
  populateStreamTable();
  RegisterStream("MLENT", aasdk::messenger::ChannelId::MEDIA_AUDIO, "permanent", "Media");
//  RegisterStream("SYSVR", aasdk::messenger::ChannelId::SYSTEM_AUDIO, "transient", "VR");
  RegisterStream("Navi", aasdk::messenger::ChannelId::SPEECH_AUDIO, "transient", "InfoMix");
  for (auto &stream : streams)
    LOG(DEBUG) << aasdk::messenger::channelIdToString(stream.first) << " " << stream.second->id << ": "
               << stream.second->name;

  AudioProxy->signal_Notify()->connect(sigc::mem_fun(*this, &AudioManagerClient::onNotify));

  audiosignals_->focusRelease.connect(sigc::mem_fun(*this, &AudioManagerClient::audioMgrReleaseAudioFocus));
  audiosignals_->focusRequest.connect(sigc::mem_fun(*this, &AudioManagerClient::audioMgrRequestAudioFocus));

//  dbus_thread = std::thread(&AudioManagerClient::listen_thread, this);

}

AudioManagerClient::~AudioManagerClient() {
  LOG(DEBUG) << "Stopping AudioManager";
  for (auto &stream : streams) {
    if (stream.second->id >= 0) {
      audioMgrReleaseAudioFocus(stream.first);
      json args = {{"sessionId", stream.second->id}};
      std::string result = AudioProxy->Request("closeSession", args.dump());
      LOG(DEBUG) << "closeSession(" << args.dump().c_str() << ")\n" << result.c_str() << "\n";
    }
    stream.second->id = -1;
  }
}

void AudioManagerClient::audioMgrRequestAudioFocus(aasdk::messenger::ChannelId channel_id,
                                                   aasdk::proto::enums::AudioFocusType_Enum aa_type) {
////  for (auto stream: streams) {
//    if (stream.second->focus && stream.second->channelId != channel_id) {
//      audioMgrReleaseAudioFocus(stream.second->channelId);
//    }
////  }
  if (streams.count(channel_id)) {
    if (!streams[channel_id]->focus) {
      json args = {
          {"sessionId", streams[channel_id]->id},
          {"requestType", "request"}
      };
      LOG(DEBUG) << args;
      try {
        std::string result = AudioProxy->Request("requestAudioFocus", args.dump());
        LOG(DEBUG) << "requestAudioFocus(" << args.dump().c_str() << ")\n" << result.c_str();
      }
      catch (DBus::Error &e) {
        LOG(ERROR) << e.what();
      }
    } else {
      switch (channel_id) {
        case aasdk::messenger::ChannelId::MEDIA_AUDIO:
          audiosignals_->focusChanged.emit(channel_id,
                                           AudioFocusState::GAIN_MEDIA_ONLY);
          break;
        case aasdk::messenger::ChannelId::SPEECH_AUDIO:
//          audiosignals_->focusChanged.emit(channel_id, AudioFocusState::GAIN_TRANSIENT_GUIDANCE_ONLY);
//          break;
        case aasdk::messenger::ChannelId::SYSTEM_AUDIO:
          audiosignals_->focusChanged.emit(channel_id,
                                           AudioFocusState::GAIN_TRANSIENT);
          break;
        default:break;
      }
    }
  }
}

void AudioManagerClient::audioMgrReleaseAudioFocus(aasdk::messenger::ChannelId channel_id) {
  LOG(INFO) << "audioMgrReleaseAudioFocus()";
  if (streams.count(channel_id)) {
    if (streams[channel_id]->focus) {
      json args = {{"sessionId", streams[channel_id]->id}};
      LOG(DEBUG) << args;
      try {
        std::string result = AudioProxy->Request("abandonAudioFocus", args.dump());
        LOG(DEBUG) << "abandonAudioFocus(" << args.dump().c_str() << ")\n" << result.c_str();
      }
      catch (DBus::Error &e) {
        LOG(ERROR) << e.what();
      }
    }
  }
}

void AudioManagerClient::onNotify(const std::string &signalName, const std::string &payload) {
  LOG(DEBUG) << "onNotify " << signalName << " " << payload;
}