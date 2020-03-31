// Copyright 2019 DeepMind Technologies Ltd. All rights reserved.
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

#ifndef THIRD_PARTY_OPEN_SPIEL_GAMES_GIN_RUMMY_BOT_H_
#define THIRD_PARTY_OPEN_SPIEL_GAMES_GIN_RUMMY_BOT_H_

#include <algorithm>
#include <vector>

#include "open_spiel/spiel.h"
#include "open_spiel/spiel_utils.h"
#include "open_spiel/spiel_bots.h"

#include "open_spiel/games/gin_rummy.h"
#include "open_spiel/games/gin_rummy/gin_rummy_utils.h"

namespace open_spiel {
namespace gin_rummy {

class GinBot : public Bot {
 public:
  explicit GinBot(Player player_id) : player_id_(player_id) {}

  void Restart() override;
  Action Step(const State& state) override;
  std::vector<int> ArrangeHandIntoMelds(const std::vector<int> &hand_);
  std::vector<int> FindOptimalDeadwood(const std::vector<int> &hand_);
  bool IsInOptimalMeld(const int upcard, const std::vector<int> &hand_);
  int BestDiscard(const std::vector<int> &hand);

 private:
  const Player player_id_;
  bool knocked_ = false;
  std::vector<Action> next_actions_;
};

}  // namespace gin_rummy
}  // namespace open_spiel

#endif  // THIRD_PARTY_OPEN_SPIEL_GAMES_GIN_RUMMY_BOT_H_
