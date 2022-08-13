#pragma once

#include <vector>
#include <functional>
#include <autoapp/Managers/IManager.hpp>
#include <aasdk_proto/NavigationChannelData.pb.h>
#include <aasdk_proto/NavigationChannelMessageIdsEnum.pb.h>
#include <aasdk_proto/NavigationDistanceMessage.pb.h>
#include <aasdk_proto/NavigationDistanceUnitEnum.pb.h>
#include <aasdk_proto/NavigationImageOptionsData.pb.h>
#include <aasdk_proto/NavigationRequestMessage.pb.h>
#include <aasdk_proto/NavigationRequestStateEnum.pb.h>
#include <aasdk_proto/NavigationTurnEventEnum.pb.h>
#include <aasdk_proto/NavigationTurnMessage.pb.h>
#include <aasdk_proto/NavigationTurnSideEnum.pb.h>

class INavigationManager : public IManager {
 public:
  using Pointer = std::shared_ptr<INavigationManager>;
  using nightCallback = std::function<void(bool)>;

 private:
  std::vector<nightCallback> nightCallbacks;

 public:
  INavigationManager() = default;
  ~INavigationManager() override = default;

  virtual void NavigationDistance(int distance,
                                  int time,
                                  int displayDistance,
                                  aasdk::proto::enums::NavigationDistanceUnit_Enum distanceUnit) = 0;
  virtual void NavigationTurn(int turn_number,
                              std::string turn_name,
                              aasdk::proto::enums::NavigationTurnSide_Enum turn_side,
                              aasdk::proto::enums::NavigationTurnEvent_Enum turn_event,
                              int turn_angle) = 0;
  virtual void NavigationStart() = 0;
  virtual void NavigationStop() = 0;
};