#pragma once

#include "server_http.hpp"
#include <future>
#include <thread>

// Added for the default_resource example
#include <algorithm>
#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>

#include "autoapp/Signals/Signals.hpp"

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;

class HttpManager {
 public:
  HttpManager(IVideoManager::Pointer videosignals, AASignals::Pointer aasignals);
  ~HttpManager();
 private:
  bool has_video_focus = false;
  bool has_audio_focus = false;
  bool aa_connected = false;
  IVideoManager::Pointer videoManager;
  AASignals::Pointer aasignals_;
  HttpServer server;
  std::thread serverThread;

  void handle_video_focus(bool state);
  void handle_aa_connect(bool state);
};