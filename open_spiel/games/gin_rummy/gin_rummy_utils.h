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

#ifndef OPEN_SPIEL_GAMES_GIN_RUMMY_UTILS_H_
#define OPEN_SPIEL_GAMES_GIN_RUMMY_UTILS_H_

#include <map>
#include <string>
#include <vector>

#include "open_spiel/abseil-cpp/absl/types/optional.h"

namespace open_spiel {
namespace gin_rummy {

inline constexpr int kNumSuits = 4;
inline constexpr int kNumRanks = 13;
inline constexpr int kNumCards = kNumSuits * kNumRanks;
inline constexpr int kMaxHandSize = 11;

using VecInt = std::vector<int>;
using VecVecInt = std::vector<std::vector<int>>;
using VecVecVecInt = std::vector<std::vector<std::vector<int>>>;

class Utils {
 public:
  Utils();

  static std::string CardString(absl::optional<int> card);
  static std::string HandToString(const VecInt &cards);
  
  static int CardInt(std::string card);
  
  static std::vector<std::string> CardIntsToCardStrings(const VecInt &cards);
  static VecInt CardStringsToCardInts(const std::vector<std::string> &cards);
  
  static int CardValue(int card_index);
  static int TotalCardValue(const VecInt &cards);
  static int TotalCardValue(const VecVecInt &meld_group);
  static int CardRank(const int card_index);
  static int CardSuit(const int card_index);
  
  static bool CompareRanks(int card_1, int card_2);
  static bool CompareSuits(int card_1, int card_2);
  
  // TODO
  static bool IsConsecutive(const VecInt &v);
  static bool IsRankMeld(const VecInt &cards);
  static bool IsSuitMeld(const VecInt &cards);
  
  static VecVecInt RankMelds(VecInt cards);
  static VecVecInt SuitMelds(VecInt cards);
  static VecVecInt AllMelds(const VecInt &cards);
  
  static bool VectorsIntersect(VecInt *v1, VecInt *v2);
  
  static VecVecInt NonOverlappingMelds(VecInt *meld, VecVecInt *melds);
  
  static void AllPaths(VecInt *meld, VecVecInt *all_melds, VecVecInt *path,
                VecVecVecInt *all_paths);
  
  static VecVecVecInt AllMeldGroups(const VecInt &cards);
  
  static VecVecInt BestMeldGroup(const VecInt &cards);
  
  static int MinDeadwood(VecInt hand, absl::optional<int> card);
  static int MinDeadwood(const VecInt &hand);
  
  static int RankMeldLayoff(const VecInt &meld);
  static VecInt SuitMeldLayoffs(const VecInt &meld);
  
  static VecInt LegalMelds(const VecInt &hand, int knock_card);
  static VecInt LegalDiscards(const VecInt &hand, int knock_card);
  
  static VecInt AllLayoffs(const VecInt &layed_melds, const VecInt &previous_layoffs);
  
  static int MeldToInt(VecInt meld);
  
  static std::map<VecInt, int> BuildMeldToIntMap();
  static std::map<int, VecInt> BuildIntToMeldMap();
  
  // TODO
  //static const std::map<int, VecInt> int_to_meld = BuildIntToMeldMap();
  //static const std::map<VecInt, int> meld_to_int = BuildMeldToIntMap();

  static std::map<int, VecInt> int_to_meld;
  static std::map<VecInt, int> meld_to_int;
};

}  // namespace gin_rummy
}  // namespace open_spiel

#endif  // OPEN_SPIEL_GAMES_GIN_RUMMY_UTILS_H_
