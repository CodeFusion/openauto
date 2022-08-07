#include "Platforms/RPI/Managers/VideoManager.hpp"

VideoManager::VideoManager() {
}

void VideoManager::start() {
}

void VideoManager::stop() {
}

void VideoManager::requestFocus() {
  focusChanged(true);
}

void VideoManager::releaseFocus() {
  focusChanged(false);
}

VideoManager::~VideoManager() {
}
