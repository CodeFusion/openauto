#include <set>
#include <dbus-cxx.h>

#include "autoapp/Signals/NavigationSignals.hpp"

#include <com_jci_navi2IHU_HUDSettings_objectProxy.h>
#include <com_jci_vbs_navi_tmc_objectProxy.h>
#include <com_jci_vbs_navi_objectProxy.h>

struct NaviData {
  std::string turn_name;
  aasdk::proto::enums::NavigationTurnSide_Enum turn_side;
  aasdk::proto::enums::NavigationTurnEvent_Enum turn_event;
  int turn_number;
  int turn_angle;
  int distance; // distance * 10, encoded like that to store one digit after decimal dot in int type
  aasdk::proto::enums::NavigationDistanceUnit_Enum distance_unit;
  int time_until;
  int msg;
  uint8_t changed;
};

enum MazdaIcons {
  NONE = 0,
  STRAIGHT = 1,
  LEFT = 2,
  RIGHT = 3,
  SLIGHT_LEFT = 4,
  SLIGHT_RIGHT = 5,
  DESTINATION = 8,
  DESTINATION_LEFT = 33,
  DESTINATION_RIGHT = 34,
  SHARP_LEFT = 11,
  SHARP_RIGHT = 9,
  U_TURN_LEFT = 13,
  U_TURN_RIGHT = 10,
  FLAG = 12,
  FLAG_LEFT = 35,
  FLAG_RIGHT = 36,
  FORK_LEFT = 15,
  FORK_RIGHT = 14,
  MERGE_LEFT = 16,
  MERGE_RIGHT = 17,
  OFF_RAMP_LEFT = 7,
  OFF_RAMP_RIGHT = 30
};

enum MazdaDistanceUnits : uint8_t {
  METERS = 1,
  MILES = 2,
  KILOMETERS = 3,
  YARDS = 4,
  FEET = 5
};

class NavigationManager {
 public:
  NavigationManager(NavigationSignals::Pointer navSignals, const std::shared_ptr<DBus::Connection> &);
  ~NavigationManager();

 private:
  static void onNavigationStart();
  void onNavigationStop();
  void onNavigationTurn(int,
                        const std::string &,
                        aasdk::proto::enums::NavigationTurnSide_Enum,
                        aasdk::proto::enums::NavigationTurnEvent_Enum,
                        int);
  void onNavigationDistance([[maybe_unused]] int,
                            [[maybe_unused]] int, int, aasdk::proto::enums::NavigationDistanceUnit_Enum);
  static uint32_t roundabout(int, aasdk::proto::enums::NavigationTurnSide_Enum side);

//  HUDSettingsCLient *hudSettings_;
  std::shared_ptr<com_jci_vbs_navi_tmcProxy> tmcClient_;
  std::shared_ptr<com_jci_vbs_naviProxy> naviClient_;
  NaviData *navi_data;
  NavigationSignals::Pointer navSignals_;

  typedef std::array<MazdaIcons, 3> TurnIcon;
  typedef std::tuple<uint32_t, uint16_t, uint8_t, uint16_t, uint8_t, uint8_t> hudDisplayMsg;
  typedef std::tuple<std::string, uint8_t> guidancePointData;

  std::map<aasdk::proto::enums::NavigationTurnEvent_Enum, TurnIcon> AA2MAZ;

};