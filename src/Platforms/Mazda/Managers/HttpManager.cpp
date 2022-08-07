#include "Platforms/Mazda/Managers/HttpManager.hpp"
#include "easylogging++.h"
#include "version.h"

HttpManager::HttpManager(IVideoManager::Pointer videosignals, AASignals::Pointer aasignals)
    : videoManager(std::move(videosignals)), aasignals_(std::move(aasignals)) {
  server.config.port = 9999;

  // Add resources using path-regex and method-string, and an anonymous function
  // POST-example for the path /string, responds the posted string
  server.resource["^/status"]["GET"] =
      [this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
        SimpleWeb::CaseInsensitiveMultimap header;
        header.emplace("Access-Control-Allow-Origin", "*");
        header.emplace("Access-Control-Allow-Methods", "POST, GET");
        header.emplace("Content-Type", "application/json");

        nlohmann::json result;
        result["connected"] = aa_connected;
        result["videoFocus"] = has_video_focus;
        result["audioFocus"] = has_audio_focus;
////        std::string logPath = "";
////        if (logPath.length() > 0)
////        {
////            result["logPath"] = logPath;
////        }
        result["headunitVersion"] = OPENAUTO_VERSION;

        std::string result_string = result.dump();
        LOG(DEBUG) << "JSON " << result_string;

        header.emplace("Content-Length", std::to_string(result_string.length()));
        response->write(SimpleWeb::StatusCode::success_ok, result_string, header);
        LOG(DEBUG) << "Got /status call. response: " << result_string;

      };

  server.resource["^/takeVideoFocus"]["GET"] =
      [this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
        SimpleWeb::CaseInsensitiveMultimap header;
        header.emplace("Access-Control-Allow-Origin", "*");
        header.emplace("Access-Control-Allow-Methods", "POST, GET");
        header.emplace("Content-Type", "application/json");

        response->write(SimpleWeb::StatusCode::success_ok, header);

        videoManager->requestFocus();

        LOG(DEBUG) << "Got /takeVideoFocus call.";
      };

  server.default_resource["GET"] =
      [](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
        SimpleWeb::CaseInsensitiveMultimap header;
        header.emplace("Access-Control-Allow-Origin", "*");
        header.emplace("Access-Control-Allow-Methods", "POST, GET");

        header.emplace("Content-Length", std::to_string(strlen("Index")));

        response->write(SimpleWeb::StatusCode::client_error_not_found, header);
      };

  server.on_error = [](std::shared_ptr<HttpServer::Request> request, const SimpleWeb::error_code &ec) {
    // Handle errors here
    // Note that connection timeouts will also call this handle with ec set to SimpleWeb::errc::operation_canceled
    LOG(DEBUG) << "Webserver Error" << ec;
  };

  serverThread = std::thread([&]() { server.start(); });
  videoManager->registerFocus([this](bool focus){this->handle_video_focus(focus);});
  aasignals_->connected.connect(sigc::mem_fun(*this, &HttpManager::handle_aa_connect));
}

HttpManager::~HttpManager() {
  LOG(DEBUG) << "Stopping HttpManager";
  server.stop();
  serverThread.join();
//  server.stop();
}

void HttpManager::handle_video_focus(bool state) {
  has_video_focus = state;
}

void HttpManager::handle_aa_connect(bool state) {
  aa_connected = state;
}
