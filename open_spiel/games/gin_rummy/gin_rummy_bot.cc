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

#include <algorithm>
#include <vector>

#include "open_spiel/spiel.h"
#include "open_spiel/spiel_utils.h"
#include "open_spiel/spiel_bots.h"

#include "open_spiel/games/gin_rummy.h"
#include "open_spiel/games/gin_rummy/gin_rummy_utils.h"
#include "open_spiel/games/gin_rummy/gin_rummy_bot.h"

namespace open_spiel {
namespace gin_rummy {

void GinBot::Restart() {
  knocked_ = false;
  next_actions_ = {};
}

std::vector<int> GinBot::ArrangeHandIntoMelds(const std::vector<int> &hand_) {
  std::vector<int> rv;
  std::vector<int> hand = hand_;
  if (gin_rummy::MinDeadwood(hand) == 0) {
    int discard = BestDiscard(hand);
    hand.erase(remove(hand.begin(), hand.end(), discard), hand.end());
  }
  std::vector<std::vector<int>> optimal_melds =
      gin_rummy::BestMeldGroup(hand);
  for (auto meld : optimal_melds) {
    rv.push_back(gin_rummy::meld_to_int.at(meld));
  }
  return rv;
}

std::vector<int> GinBot::FindOptimalDeadwood(const std::vector<int> &hand_) {
  std::vector<int> hand = hand_;
  std::vector<int> deadwood = hand;
  std::vector<std::vector<int>> optimal_melds =
      gin_rummy::BestMeldGroup(hand);

  for (auto meld : optimal_melds) {
    for (auto card : meld) {
      deadwood.erase(remove(deadwood.begin(), deadwood.end(), card),
                     deadwood.end());
    }
  }
  return deadwood;
}

bool GinBot::IsInOptimalMeld(const int upcard, const std::vector<int> &hand_) {
  std::vector<int> hand = hand_;
  hand.push_back(upcard);
  std::vector<int> optimal_deadwood = FindOptimalDeadwood(hand);
  std::vector<int>::iterator it;
  if (find(optimal_deadwood.begin(), optimal_deadwood.end(), upcard) !=
      optimal_deadwood.end()) {
    return false;
  } else {
    return true;
  }
}

int GinBot::BestDiscard(const std::vector<int> &hand) {
  std::vector<int> deadwood = FindOptimalDeadwood(hand);
  if (deadwood.size() > 0) {
    std::sort(deadwood.begin(), deadwood.end(), gin_rummy::CompareRanks);
    return deadwood.back();
  } else {
    // 11 card gin, must be careful to throw a card that preserves gin
    for (int i = 0; i < hand.size(); ++i) {
      std::vector<int> hand_ = hand;
      hand_.erase(hand_.begin() + i);
      if (gin_rummy::MinDeadwood(hand_) == 0)
        return hand[i];
    }
    SpielFatalError("11 card gin error");
  }
}

Action GinBot::Step(const State& state) {
  std::vector<double> observation;
  state.ObservationTensor(player_id_, &observation);

  std::vector<int> hand;
  std::vector<int> layed_melds;
  std::vector<int> discard_pile;
  int upcard = -1;
  int knock_card = -1;
  int stock_size = 0;

  int offset = 0;
  SPIEL_CHECK_TRUE(observation[player_id_] == 1);
  offset += 2;

  for (int i = 0; i < 10; ++i) {
    if (observation[offset + i] == 1) {
      knock_card += 1;
    }
  }
  offset += 10;

  for (int i = 0; i < gin_rummy::kNumCards; ++i) {
    if (observation[offset + i] == 1) {
      hand.push_back(i);
    }
  }
  offset += 52;

  for (int i = 0; i < gin_rummy::kNumCards; ++i) {
    if (observation[offset + i] == 1) {
      upcard = i;
    }
  }
  offset += 52;

  for (int i = 0; i < gin_rummy::kNumCards; ++i) {
    if (observation[offset + i] == 1) {
      discard_pile.push_back(i);
    }
  }
  offset += 52;

  for (int i = 0; i < 31; ++i) {
    if (observation[offset + i] == 1) {
      stock_size += 1;
    }
  }
  offset += 31;

  for (int i = 0; i < gin_rummy::kNumMeldActions; ++i) {
    if (observation[offset + i] == 1) {
      layed_melds.push_back(i);
      knocked_ = true;
    }
  }  // completed observation decoding

  auto legal_actions = state.LegalActions(player_id_);

  // next actions must be in legal order from back to front
  if (next_actions_.size() > 0) {
    Action action = next_actions_.back();
    if (std::find(legal_actions.begin(), legal_actions.end(), action) ==
        legal_actions.end()) {
      std::cout << "Legal Actions: " << legal_actions << std::endl;
      std::cout << "Next Actions: " << next_actions_ << std::endl;
    }
    SPIEL_CHECK_TRUE(std::find(legal_actions.begin(),
        legal_actions.end(), action) != legal_actions.end());
    next_actions_.pop_back();
    return action;
  }

  // when we knock we decide how to lay the hand all at once
  // save these melds as next actions
  if (knocked_) {
    if (layed_melds.size() > 0) {
      // our opponent knocked
      // must compute layoffs and then melds
      // TODO(jhtschultz) support layoffs
      next_actions_.push_back(gin_rummy::kPassAction);
      std::vector<int> melds_to_lay = ArrangeHandIntoMelds(hand);
      for (int meld_id : melds_to_lay) {
        next_actions_.push_back(gin_rummy::kMeldActionBase + meld_id);
      }
      next_actions_.push_back(gin_rummy::kPassAction);
    } else {
      next_actions_.push_back(gin_rummy::kPassAction);
      std::vector<int> melds_to_lay = ArrangeHandIntoMelds(hand);
      for (int meld_id : melds_to_lay) {
        next_actions_.push_back(gin_rummy::kMeldActionBase + meld_id);
      }
      int best_discard = BestDiscard(hand);
      next_actions_.push_back(best_discard);
    }
    Action action = next_actions_.back();
    SPIEL_CHECK_TRUE(std::find(legal_actions.begin(),
        legal_actions.end(), action) != legal_actions.end());
    next_actions_.pop_back();
    return action;
  }

  if (upcard == -1) {
    // MoveType kDiscard
    SPIEL_CHECK_EQ(hand.size(), 11);
    int deadwood = gin_rummy::MinDeadwood(hand);
    if (deadwood <= 10 && !knocked_) {
      knocked_ = true;
      return gin_rummy::kKnockAction;
    } else {
      int best_discard = BestDiscard(hand);
      if (best_discard >= 0) {
        return best_discard;
      } else {
        return legal_actions[0];
      }
    }
  } else {
    // MoveType kDraw
    if (stock_size == 2) return gin_rummy::kPassAction;
    if (IsInOptimalMeld(upcard, hand)) {
      return gin_rummy::kDrawUpcardAction;  // pick up upcard
    } else {
      return legal_actions.back();  // draw from stock or pass
    }
  }
  SpielFatalError("GinBot should not get here.");
}

}  // namespace gin_rummy
}  // namespace open_spiel
