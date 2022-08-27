#include <set>

#include "autoapp/Managers/INavigationManager.hpp"

class NavigationManager: public INavigationManager {
 public:
  explicit NavigationManager();
  ~NavigationManager() override = default;
  void start() override;
  void stop() override;
  void NavigationStart() override;
  void NavigationStop() override;
  void NavigationTurn(int turn_number,
                      std::string turn_name,
                      aasdk::proto::enums::NavigationTurnSide_Enum turn_side,
                      aasdk::proto::enums::NavigationTurnEvent_Enum turn_event,
                      int turn_angle) override;
  void NavigationDistance(int distance,
                          int time,
                          int displayDistance,
                          aasdk::proto::enums::NavigationDistanceUnit_Enum distanceUnit) override;
};