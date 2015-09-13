// Copyright 2014 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef AI_CONTROLLER_H_
#define AI_CONTROLLER_H_

#include "precompiled.h"

#include <vector>

#include "audio_config_generated.h"
#include "character_state_machine_def_generated.h"
#include "common.h"
#include "config_generated.h"
#include "controller.h"
#include "game_state.h"
#include "pie_noon_common_generated.h"
#include "timeline_generated.h"

namespace fpl {
namespace pie_noon {

// A computer-controlled player.  Basically the same as PlayerController,
// except that instead of generating logical inputs based on events,
// this generates inputs based on random numbers and the current game state.
class AiController : public Controller {
 public:
  AiController();

  // Give the AI everything it will need.
  void Initialize(GameState* gamestate_ptr, const Config* config,
                  int characterId);

  // Decide what the robot is doing this frame.
  virtual void AdvanceFrame(WorldTime delta_time);

 private:
  bool IsInDanger(CharacterId id) const;
  WorldTime block_timer_;  // How many milliseconds we need to block.

  GameState* gamestate_;  // Pointer to the gamestate object
  const Config* config_;  // Pointer to the config structure
  WorldTime time_to_next_action_;
};

}  // pie_noon
}  // fpl

#endif  // AI_CONTROLLER_H_
