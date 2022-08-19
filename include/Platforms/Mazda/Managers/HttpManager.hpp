#pragma once

#include "server_http.hpp"
#include "autoapp/Managers/IVideoManager.hpp"
#include <future>
#include <thread>

// Added for the default_resource example
#include <algorithm>
#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>



using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;

class HttpManager {
 public:
  explicit HttpManager(IVideoManager::Pointer videosignals);
  ~HttpManager();
  void handle_aa_connect(bool state);
 private:
  bool has_video_focus = false;
  bool has_audio_focus = false;
  bool aa_connected = false;
  IVideoManager::Pointer videoManager;
  HttpServer server;
  std::thread serverThread;

  void handle_video_focus(bool state);

};