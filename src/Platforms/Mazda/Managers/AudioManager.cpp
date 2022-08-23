#include "Platforms/Mazda/Managers/AudioManager.hpp"
#include "easylogging++.h"
#include <thread>
#include <utility>

void AudioManager::onRequestAudioFocusResult(json result, Stream *stream) {
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

void AudioManager::onAudioFocusChange(json result, Stream *stream) {
  std::lock_guard<std::mutex> lock(AudioMutex);
  auto focus = result["newFocus"].get<std::string>();
  if (focus == "lost") {
    stream->focus = false;
    json activeargs = {
        {"sessionId", stream->id},
        {"playing", false}
    };
    AudioProxy->Request("audioActive", activeargs.dump());
    updateFocus(stream->channelId, AudioFocusState::LOSS);
    LOG(DEBUG) << "Stream " << stream->id << ": " << stream->name << " Focus Lost";
  } else if (focus == "temporarilyLost") {
    stream->focus = false;
    json activeargs = {
        {"sessionId", stream->id},
        {"playing", false}
    };
    AudioProxy->Request("audioActive", activeargs.dump());
    updateFocus(stream->channelId, AudioFocusState::LOSS_TRANSIENT);
    LOG(DEBUG) << "Stream " << stream->id << ": " << stream->name << " Focus Temporarily Lost";
  } else if (focus == "gained") {
    stream->focus = true;
    switch (stream->channelId) {
      case aasdk::messenger::ChannelId::MEDIA_AUDIO:
        updateFocus(stream->channelId, AudioFocusState::GAIN);
        break;
      case aasdk::messenger::ChannelId::SPEECH_AUDIO:
//            updateFocus(stream->channelId, AudioFocusState::GAIN_TRANSIENT_GUIDANCE_ONLY);
//            break;
      case aasdk::messenger::ChannelId::SYSTEM_AUDIO:
        updateFocus(stream->channelId, AudioFocusState::GAIN_TRANSIENT);
        break;
      default:
        break;
    }
    LOG(DEBUG) << "Stream " << stream->id << ": " << stream->name << " Has Focus " << stream->focus;
  }
  if(!workQueue.empty()){
    doWork();
  }
}

std::string AudioManager::RequestHandler(const std::string &methodName, const std::string &arguments) {
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

void AudioManager::RegisterStream(std::string StreamName,
                                  aasdk::messenger::ChannelId ChannelId,
                                  std::string StreamMode,
                                  std::string StreamType,
                                  std::string StreamModeName) {
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
        {"streamModeName", StreamModeName},
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

void AudioManager::populateData() {
  json requestArgs = {
      {"svc", "DEST"},
      {"pretty", false}
  };
  std::string resultString = AudioProxy->Request("dumpState", requestArgs.dump());
  try {
    auto result = json::parse(resultString);
    for (json::iterator it = result["Cabin"]["reqMatrixIdx"].begin(); it != result["Cabin"]["reqMatrixIdx"].end();
         ++it) {
      MazdaDestinations.emplace_back(it.key());
    }
    for (const auto &streamtype : MazdaDestinations) {
      LOG(DEBUG) << "StreamType: " << streamtype;
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

AudioManager::AudioManager(std::shared_ptr<DBus::Connection> session_connection, asio::io_service &ioService)
    : dbusConnection(std::move(session_connection)), strand_(ioService), timer_(ioService) {

}

void AudioManager::doWork() {
  aasdk::messenger::ChannelId channelId;
  std::string focus;

  queueLock.lock();
  channelId = workQueue.front().channelId;
  focus = workQueue.front().focus;
  workQueue.pop_front();
  queueLock.unlock();
  LOG(DEBUG) << (int)channelId << " " << focus;

  json args = {
      {"sessionId", streams[channelId]->id},
      {"requestType", "request"}
  };
  LOG(DEBUG) << args;
  try {
    auto msg = AudioProxy->create_call_message("Request");
    msg->set_no_reply();
    msg << focus;
    msg << args.dump();
    AudioProxy->object()->connection().lock()->send(msg);
//      std::string result = AudioProxy->Request("requestAudioFocus", args.dump());
//      LOG(DEBUG) << "requestAudioFocus(" << args.dump().c_str() << ")\n" << result.c_str();
  }
  catch (DBus::Error &e) {
    LOG(ERROR) << e.what();
  }
}

void AudioManager::requestFocus(aasdk::messenger::ChannelId channelId,
                                AudioFocusType aa_type) {
  std::lock_guard<std::mutex> lock(AudioMutex);
  LOG(DEBUG) << "requestFocus()";
  if (streams.count(channelId) > 0) {
    LOG(DEBUG) << "Stream Exists";
    if (!streams[channelId]->focus) {
      LOG(DEBUG) << "Stream Does not already have focus";
      queueLock.lock();
      workQueue.emplace_back(workItem{channelId, "requestAudioFocus"});
      queueLock.unlock();
    } else {
      LOG(DEBUG) << "Stream already had focus";
      switch (channelId) {
        case aasdk::messenger::ChannelId::MEDIA_AUDIO:
          updateFocus(channelId, AudioFocusState::GAIN);
          break;
        case aasdk::messenger::ChannelId::SPEECH_AUDIO:
//            updateFocus(stream->channelId, AudioFocusState::GAIN_TRANSIENT_GUIDANCE_ONLY);
//            break;
        case aasdk::messenger::ChannelId::SYSTEM_AUDIO:
          updateFocus(channelId, AudioFocusState::GAIN_TRANSIENT);
          break;
        default:
          break;
      }
    }
  }
  if(workQueue.size() == 1){
    doWork();
  }
}

void AudioManager::releaseFocus(aasdk::messenger::ChannelId channelId) {
  LOG(INFO) << "releaseFocus()";
  std::lock_guard<std::mutex> lock(AudioMutex);
  sleep(1); //Delay to smooth out release, Android Auto doesn't account for latency
  if (channelId == aasdk::messenger::ChannelId::NONE) {
    for (auto &stream : streams) {
      LOG(DEBUG) << "Channel: " << int(stream.first) << " Stream: " << stream.second->id << " Focus: "
                 << stream.second->focus;
      if (stream.second->focus) {
        queueLock.lock();
        workQueue.emplace_back(workItem{stream.first, "abandonAudioFocus"});
        queueLock.unlock();
      }
    }
  } else {
    auto &stream = streams[channelId];
    LOG(DEBUG) << "Channel: " << aasdk::messenger::channelIdToString(channelId) << " Stream: " << stream->id
               << " Focus: "
               << stream->focus;
    if (stream->focus) {
      queueLock.lock();
      workQueue.emplace_back(workItem{channelId, "abandonAudioFocus"});
      queueLock.unlock();
    }
  }
  if(workQueue.size() == 1){
    doWork();
  }
}


void AudioManager::onNotify(const std::string &signalName, const std::string &payload) {
  LOG(DEBUG) << "onNotify " << signalName << " " << payload;
}
void AudioManager::start() {
  std::lock_guard<std::mutex> lock(AudioMutex);
  dbusConnection->request_name("com.androidauto", DBUSCXX_NAME_FLAG_REPLACE_EXISTING);
  AudioObject = dbusConnection->create_object("/com/androidauto/audio", DBus::ThreadForCalling::DispatcherThread);
  AudioRequestInterface = AudioObject->create_interface("com.xsembedded.ServiceProvider");
  AudioRequestInterface->create_method<std::string(std::string, std::string)>("Request",
                                                                              sigc::mem_fun(*this,
                                                                                            &AudioManager::RequestHandler));

  AudioInterface = com_xsembedded_ServiceProvider_objectProxy::create(dbusConnection,
                                                                      "com.xsembedded.service.AudioManagement",
                                                                      "/com/xse/service/AudioManagement/AudioApplication");
  AudioProxy = AudioInterface->getcom_xsembedded_ServiceProviderInterface();

  populateData();

  // If we don't have Android Auto audio destinations, use the old ones
  if (std::find(MazdaDestinations.begin(), MazdaDestinations.end(), "AAMedia") == MazdaDestinations.end()) {
    RegisterStream("MLENT", aasdk::messenger::ChannelId::MEDIA_AUDIO, "permanent", "Media", "Media");
    RegisterStream("Navi", aasdk::messenger::ChannelId::SPEECH_AUDIO, "transient", "InfoMix", "InfoMix");
  } else {
    RegisterStream("AAMedia", aasdk::messenger::ChannelId::MEDIA_AUDIO, "permanent", "AAMedia", "AAMedia");
    RegisterStream("AAGuidance", aasdk::messenger::ChannelId::SPEECH_AUDIO, "transient", "AAGuidance", "AAGuidance");
  }

  for (auto &stream : streams) {
    LOG(DEBUG) << aasdk::messenger::channelIdToString(stream.first) << " " << stream.second->id << ": "
               << stream.second->name;
  }

  notifyConnection = AudioProxy->signal_Notify()->connect(&AudioManager::onNotify);
}
void AudioManager::stop() {
  std::lock_guard<std::mutex> lock(AudioMutex);
  LOG(DEBUG) << "Stopping AudioManager";
  notifyConnection.disconnect();
  for (auto &stream : streams) {
    if (stream.second->id >= 0) {
      try {
        json args = {{"sessionId", stream.second->id}};
        std::string result = AudioProxy->Request("abandonAudioFocus", args.dump());
        LOG(DEBUG) << "abandonAudioFocus(" << args.dump() << ")\n" << result;
        result = AudioProxy->Request("closeSession", args.dump());
        LOG(DEBUG) << "closeSession(" << args.dump() << ")\n" << result;
      }
      catch (DBus::Error &error) {
        LOG(ERROR) << error.name() << ": " << error.message();
      }
      catch (json::exception &error) {
        LOG(ERROR) << error.what();
      }
    }
    delete stream.second;
  }
  streams.clear();
  streamsByID.clear();
  MazdaDestinations.clear();
  AudioProxy.reset();
  AudioInterface.reset();
  AudioRequestInterface->remove_method("Request");
  AudioObject.reset();
  dbusConnection->release_name("com.androidauto");
}
IAudioManager::focusType AudioManager::getFocusType(aasdk::messenger::ChannelId channelId) {
  if (streams[channelId]->mode == "permanent") {
    return IAudioManager::focusType::NORMAL;
  }
  return IAudioManager::focusType::TRANSIENT;
}
