#include "Platforms/Mazda/Managers/NavigationManager.hpp"
#include "easylogging++.h"

NavigationManager::NavigationManager(NavigationSignals::Pointer navSignals,
                                     const std::shared_ptr<DBus::Connection> &systemconnection) : navSignals_(std::move(
    navSignals)) {
//  auto hudSettingsConnection = sdbus::createSessionBusConnection();
//  hudSettings_ = new HUDSettingsCLient(hudSettingsConnection, "com.jci.navi2IHU", "/com/jci/navi2IHU"); // on hmi bus

  std::shared_ptr<com_jci_vbs_navi_tmc_objectProxy>
      tmcProxy = com_jci_vbs_navi_tmc_objectProxy::create(systemconnection, "com.jci.vbs.navi", "/com/jci/vbs/navi");
  std::shared_ptr<com_jci_vbs_navi_objectProxy>
      naviProxy = com_jci_vbs_navi_objectProxy::create(systemconnection, "com.jci.vbs.navi", "/com/jci/vbs/navi");

  tmcClient_ = tmcProxy->getcom_jci_vbs_navi_tmcInterface();
  naviClient_ = naviProxy->getcom_jci_vbs_naviInterface();
  navi_data = new NaviData;

  AA2MAZ.insert(std::pair(aasdk::proto::enums::NavigationTurnEvent::DEPART,
                          TurnIcon{MazdaIcons::FLAG_LEFT, MazdaIcons::FLAG_RIGHT, MazdaIcons::FLAG}));
  AA2MAZ.insert(std::pair(aasdk::proto::enums::NavigationTurnEvent::NAME_CHANGE,
                          TurnIcon{MazdaIcons::STRAIGHT, MazdaIcons::STRAIGHT,
                                   MazdaIcons::STRAIGHT})); //TURN_NAME_CHANGE
  AA2MAZ.insert(std::pair(aasdk::proto::enums::NavigationTurnEvent::SLIGHT_TURN,
                          TurnIcon{MazdaIcons::SLIGHT_LEFT, MazdaIcons::SLIGHT_RIGHT,
                                   MazdaIcons::STRAIGHT})); //TURN_SLIGHT_TURN
  AA2MAZ.insert(std::pair(aasdk::proto::enums::NavigationTurnEvent::TURN,
                          TurnIcon{MazdaIcons::LEFT, MazdaIcons::RIGHT, MazdaIcons::NONE})); //TURN_TURN
  AA2MAZ.insert(std::pair(aasdk::proto::enums::NavigationTurnEvent::SHARP_TURN,
                          TurnIcon{MazdaIcons::SHARP_LEFT, MazdaIcons::SHARP_RIGHT,
                                   MazdaIcons::NONE})); //TURN_SHARP_TURN
  AA2MAZ.insert(std::pair(aasdk::proto::enums::NavigationTurnEvent::U_TURN,
                          TurnIcon{MazdaIcons::U_TURN_LEFT, MazdaIcons::U_TURN_RIGHT, MazdaIcons::NONE})); //TURN_U_TURN
  AA2MAZ.insert(std::pair(aasdk::proto::enums::NavigationTurnEvent::ON_RAMP,
                          TurnIcon{MazdaIcons::LEFT, MazdaIcons::RIGHT, MazdaIcons::STRAIGHT})); //TURN_ON_RAMP
  AA2MAZ.insert(std::pair(aasdk::proto::enums::NavigationTurnEvent::OFF_RAMP,
                          TurnIcon{MazdaIcons::SLIGHT_LEFT, MazdaIcons::SLIGHT_RIGHT,
                                   MazdaIcons::STRAIGHT})); //TURN_OFF_RAMP
  AA2MAZ.insert(std::pair(aasdk::proto::enums::NavigationTurnEvent::FORK,
                          TurnIcon{MazdaIcons::FORK_LEFT, MazdaIcons::FORK_RIGHT, MazdaIcons::NONE})); //TURN_FORK
  AA2MAZ.insert(std::pair(aasdk::proto::enums::NavigationTurnEvent::MERGE,
                          TurnIcon{MazdaIcons::MERGE_LEFT, MazdaIcons::MERGE_RIGHT, MazdaIcons::NONE})); //TURN_MERGE
  AA2MAZ.insert(std::pair(aasdk::proto::enums::NavigationTurnEvent::ROUNDABOUT_ENTER,
                          TurnIcon{MazdaIcons::NONE, MazdaIcons::NONE, MazdaIcons::NONE}));  //TURN_ROUNDABOUT_ENTER
  AA2MAZ.insert(std::pair(aasdk::proto::enums::NavigationTurnEvent::ROUNDABOUT_EXIT,
                          TurnIcon{MazdaIcons::NONE, MazdaIcons::NONE, MazdaIcons::NONE})); // TURN_ROUNDABOUT_EXIT
  AA2MAZ.insert(std::pair(aasdk::proto::enums::NavigationTurnEvent::ROUNDABOUT_ENTER_AND_EXIT,
                          TurnIcon{MazdaIcons::NONE, MazdaIcons::NONE,
                                   MazdaIcons::NONE})); //TURN_ROUNDABOUT_ENTER_AND_EXIT (Will have to handle seperatly)
  AA2MAZ.insert(std::pair(aasdk::proto::enums::NavigationTurnEvent::STRAIGHT,
                          TurnIcon{MazdaIcons::STRAIGHT, MazdaIcons::STRAIGHT, MazdaIcons::STRAIGHT})); //TURN_STRAIGHT
  AA2MAZ.insert(std::pair(aasdk::proto::enums::NavigationTurnEvent::DESTINATION,
                          TurnIcon{MazdaIcons::DESTINATION_LEFT, MazdaIcons::DESTINATION_RIGHT,
                                   MazdaIcons::DESTINATION})); //TURN_DESTINATION

//  if (hudSettings_->GetHUDIsInstalled()) {
  LOG(INFO) << "HUD DETECTED";
  navSignals_->onNavigationDistance.connect(sigc::mem_fun(*this, &NavigationManager::onNavigationDistance));
  navSignals_->onNavigationTurn.connect(sigc::mem_fun(*this, &NavigationManager::onNavigationTurn));
  navSignals_->onNavigationStart.connect(&NavigationManager::onNavigationStart);
  navSignals_->onNavigationStop.connect(sigc::mem_fun(*this, &NavigationManager::onNavigationStop));
//  }
//  else {
//    LOG(INFO) << "HUD NOT DETECTED";
//  }

}

uint32_t NavigationManager::roundabout(int degrees, aasdk::proto::enums::NavigationTurnSide_Enum side) {
  uint8_t nearest = (degrees + 15) / 30;
  uint8_t offset = side == 0 ? 49 : 37;
  return (nearest + offset);

}

void NavigationManager::onNavigationTurn(int turn_number,
                                         const std::string &turn_name,
                                         aasdk::proto::enums::NavigationTurnSide_Enum turn_side,
                                         aasdk::proto::enums::NavigationTurnEvent_Enum turn_event,
                                         int turn_angle) {
  if (turn_number != navi_data->turn_number || turn_name != navi_data->turn_name) {
    navi_data->turn_number = turn_number;
    navi_data->turn_name = turn_name;
    navi_data->msg++;
    if (navi_data->msg >= 8) {
      navi_data->msg = 1;
    }
  }
  navi_data->turn_side = turn_side;
  navi_data->turn_event = turn_event;
  navi_data->turn_angle = turn_angle;

  LOG(DEBUG) << "msg" << navi_data->msg << " " << turn_name;

  tmcClient_->SetHUD_Display_Msg2(guidancePointData(turn_name, navi_data->msg));

}

void NavigationManager::onNavigationDistance([[maybe_unused]] int distance,
                                             [[maybe_unused]] int time,
                                             int displayDistance,
                                             aasdk::proto::enums::NavigationDistanceUnit_Enum distanceUnit) {
  uint32_t diricon = 0;
  uint16_t nowDistance;
  MazdaDistanceUnits nowUnit;
  if (navi_data->turn_event == aasdk::proto::enums::NavigationTurnEvent::ROUNDABOUT_ENTER_AND_EXIT) {
    LOG(DEBUG) << "Roundabout";
    diricon = roundabout(navi_data->turn_angle, navi_data->turn_side);
  } else {
    if (AA2MAZ.count(navi_data->turn_event) > 0) {
      diricon = AA2MAZ[navi_data->turn_event][navi_data->turn_side - 1];
    } else {
      LOG(DEBUG) << "No matching navigation icon for " << navi_data->turn_event << " " << navi_data->turn_side;
    }
  }

  switch (distanceUnit) {
    case aasdk::proto::enums::NavigationDistanceUnit::METERS:nowDistance = displayDistance / 100;
      nowUnit = MazdaDistanceUnits::METERS;
      break;
    case aasdk::proto::enums::NavigationDistanceUnit::KILOMETERS10:
    case aasdk::proto::enums::NavigationDistanceUnit::KILOMETERS:nowDistance = displayDistance / 100;
      nowUnit = MazdaDistanceUnits::KILOMETERS;
      break;
    case aasdk::proto::enums::NavigationDistanceUnit::MILES10:
    case aasdk::proto::enums::NavigationDistanceUnit::MILES:nowDistance = displayDistance / 100;
      nowUnit = MazdaDistanceUnits::MILES;
      break;
    case aasdk::proto::enums::NavigationDistanceUnit::FEET:nowDistance = displayDistance / 100;
      nowUnit = MazdaDistanceUnits::FEET;
      break;
    case aasdk::proto::enums::NavigationDistanceUnit::YARDS:nowDistance = displayDistance / 100;
      nowUnit = MazdaDistanceUnits::YARDS;
      break;
    default:break;
  }

  LOG(DEBUG) << "msg" << navi_data->msg << " " << static_cast<int>(diricon) << " " << nowDistance << " " << nowUnit;

  naviClient_->SetHUDDisplayMsgReq(hudDisplayMsg(diricon,
                                                 nowDistance,
                                                 nowUnit,
                                                 0,
                                                 0,
                                                 navi_data->msg));

}

void NavigationManager::onNavigationStart() {
  LOG(DEBUG) << "Navigation Started";
  system("smctl -s -n NNG");
  system("smctl -s -n jcinavi");
}
void NavigationManager::onNavigationStop() {
  tmcClient_->SetHUD_Display_Msg2(guidancePointData("", 0));
  naviClient_->SetHUDDisplayMsgReq(hudDisplayMsg(0, 0, 0, 0, 0, 0));

  LOG(DEBUG) << "Navigation Stopped";
  system("smctl -l -n jcinavi &");
}

NavigationManager::~NavigationManager() = default;
