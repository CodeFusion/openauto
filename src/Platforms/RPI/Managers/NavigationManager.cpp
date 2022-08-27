#include "Platforms/RPI/Managers/NavigationManager.hpp"
#include "easylogging++.h"

NavigationManager::NavigationManager() = default;

void NavigationManager::start() {
}

void NavigationManager::stop() {
}

void NavigationManager::NavigationTurn(int turn_number,
                                       std::string turn_name,
                                       aasdk::proto::enums::NavigationTurnSide_Enum turn_side,
                                       aasdk::proto::enums::NavigationTurnEvent_Enum turn_event,
                                       int turn_angle) {
}

void NavigationManager::NavigationDistance([[maybe_unused]] int distance,
                                           [[maybe_unused]] int time,
                                           int displayDistance,
                                           aasdk::proto::enums::NavigationDistanceUnit_Enum distanceUnit) {
}

void NavigationManager::NavigationStart() {

}
void NavigationManager::NavigationStop() {

}