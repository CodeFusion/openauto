#include <autoapp/Managers/AudioManager.hpp>
#include <easylogging++.h>
#include <thread>

void AudioManagerClient::onRequestAudioFocusResult(json result, Stream *stream) {
  std::lock_guard<std::mutex> lock(AudioMutex);
  if (result["newFocus"].get<std::string>() == "granted") {
    json activeargs = {
        {"sessionId", stream->id},
        {"playing", true}
    };
    AudioProxy->Request("audioActive", activeargs.dump());
    LOG(DEBUG) << "Stream " << stream->id << ": " << stream->name << " Focus Granted";
  }
}

void AudioManagerClient::onAudioFocusChange(json result, Stream *stream) {
  std::lock_guard<std::mutex> lock(AudioMutex);
  auto focus = result["newFocus"].get<std::string>();
  if (focus == "lost") {
    stream->focus = false;
    json activeargs = {
        {"sessionId", stream->id},
        {"playing", false}
    };
    AudioProxy->Request("audioActive", activeargs.dump());
    audiosignals_->focusChanged.emit(stream->channelId, AudioFocusState::LOSS);
    LOG(DEBUG) << "Stream " << stream->id << ": " << stream->name << " Focus Lost";
  } else if (focus == "temporarilyLost") {
    stream->focus = false;
    json activeargs = {
        {"sessionId", stream->id},
        {"playing", false}
    };
    AudioProxy->Request("audioActive", activeargs.dump());
    audiosignals_->focusChanged.emit(stream->channelId, AudioFocusState::LOSS_TRANSIENT);
    LOG(DEBUG) << "Stream " << stream->id << ": " << stream->name << " Focus Temporarily Lost";
  } else if (focus == "gained") {
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
    LOG(DEBUG) << "Stream " << stream->id << ": " << stream->name << " Has Focus " << stream->focus;
  }
}

std::string AudioManagerClient::RequestHandler(const std::string &methodName, const std::string &arguments) {
  LOG(DEBUG) << methodName << " " << arguments;
  auto result = json::parse(arguments);
  if (streamsByID.count(result["sessionId"].get<int>()) > 0) {
    Stream *stream = streamsByID[result["sessionId"].get<int>()];
    if (methodName == "onRequestAudioFocusResult") {
      onRequestAudioFocusResult(result, stream);
    } else if (methodName == "onAudioFocusChange") {
      onAudioFocusChange(result, stream);
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
    auto *stream = new Stream;
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
  for (auto &Dest : MazdaDestinations) {
    VLOG(9) << Dest;
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
  RegisterStream("MLENT", aasdk::messenger::ChannelId::MEDIA_AUDIO, "permanent", "Media");
  RegisterStream("Navi", aasdk::messenger::ChannelId::SPEECH_AUDIO, "transient", "InfoMix");
  for (auto &stream : streams) {
    LOG(DEBUG) << aasdk::messenger::channelIdToString(stream.first) << " " << stream.second->id << ": "
               << stream.second->name;
  }

  AudioProxy->signal_Notify()->connect(&AudioManagerClient::onNotify);

  audiosignals_->focusRelease.connect(sigc::mem_fun(*this, &AudioManagerClient::audioMgrReleaseAudioFocus));
  audiosignals_->focusRequest.connect(sigc::mem_fun(*this, &AudioManagerClient::audioMgrRequestAudioFocus));

}

AudioManagerClient::~AudioManagerClient() {
  LOG(DEBUG) << "Stopping AudioManager";
  for (auto &stream : streams) {
    if (stream.second->id >= 0) {
      try {
        audioMgrReleaseAudioFocus(stream.first);
        json args = {{"sessionId", stream.second->id}};
        std::string result = AudioProxy->Request("closeSession", args.dump());
        LOG(DEBUG) << "closeSession(" << args.dump().c_str() << ")\n" << result.c_str() << "\n";
      }
      catch (DBus::Error &error) {
        LOG(ERROR) << error.name() << ": " << error.message();
      }
      catch (json::exception &error) {
        LOG(ERROR) << error.what();
      }
    }
    stream.second->id = -1;
  }
}

void AudioManagerClient::audioMgrRequestAudioFocus(aasdk::messenger::ChannelId channel_id,
                                                   aasdk::proto::enums::AudioFocusType_Enum aa_type) {
  std::lock_guard<std::mutex> lock(AudioMutex);
  if (streams.count(channel_id) > 0) {
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
  std::lock_guard<std::mutex> lock(AudioMutex);
  try {
    for (auto &stream : streams) {
      LOG(DEBUG) << "Channel: " << int(stream.first) << " Stream: " << stream.second->id << " Focus: " << stream.second->focus;
      if (stream.second->focus) {
//        audioMgrStopPlaying(stream.first);
        json args = {{"sessionId", stream.second->id}};
        std::string result = AudioProxy->Request("abandonAudioFocus", args.dump());
        LOG(DEBUG) << "abandonAudioFocus(" << args.dump().c_str() << ")\n" << result.c_str();
      }
    }
  }
  catch (DBus::Error &e) {
    LOG(ERROR) << e.what();
  }
}

void AudioManagerClient::onNotify(const std::string &signalName, const std::string &payload) {
  LOG(DEBUG) << "onNotify " << signalName << " " << payload;
}