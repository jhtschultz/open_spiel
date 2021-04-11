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

//inline constexpr int kNumSuits = 4;
//inline constexpr int kNumRanks = 13;
//inline constexpr int kNumCards = kNumSuits * kNumRanks;
inline constexpr int kMaxHandSize = 11;

using VecInt = std::vector<int>;
using VecVecInt = std::vector<std::vector<int>>;
using VecVecVecInt = std::vector<std::vector<std::vector<int>>>;

// TODO changed logic in first if statement
struct SuitComparator {
  SuitComparator(const int num_ranks) : num_ranks_(num_ranks) {}
  int CardSuit(int card) { return card / num_ranks_; }
  bool operator()(int card_1, int card_2) {
    if (CardSuit(card_1) == CardSuit(card_2)) {
      return card_1 < card_2;
    }
    return CardSuit(card_1) < CardSuit(card_2);
  }
  int num_ranks_;
};

// TODO
struct RankComparator {
  RankComparator(const int num_ranks) : num_ranks_(num_ranks) {}
  int CardRank(int card) { return card % num_ranks_; }
  bool operator()(int card_1, int card_2) {
    if (CardRank(card_1) == CardRank(card_2)) {
      return card_1 < card_2;
    }
    return CardRank(card_1) < CardRank(card_2);
  }
  int num_ranks_;
};


class Utils {
 public:
  //Utils();
  // TODO check order of ranks and suits everywhere
  Utils(int num_ranks, int num_suits);

  const int num_suits_;
  const int num_ranks_;
  const int num_cards_;

  const SuitComparator suit_comp_;
  const RankComparator rank_comp_;
  

  std::string CardString(absl::optional<int> card) const;
  std::string HandToString(const VecInt &cards) const;
  
  int CardInt(std::string card) const;
  
  std::vector<std::string> CardIntsToCardStrings(const VecInt &cards) const;
  VecInt CardStringsToCardInts(const std::vector<std::string> &cards) const;
  
  int CardValue(int card_index) const;
  int TotalCardValue(const VecInt &cards) const;
  int TotalCardValue(const VecVecInt &meld_group) const;
  int CardRank(const int card_index) const;
  int CardSuit(const int card_index) const;
  
  // TODO
  //static bool CompareRanks(int card_1, int card_2);
  //static bool CompareSuits(int card_1, int card_2);
  
  // TODO
  bool IsConsecutive(const VecInt &v) const;
  bool IsRankMeld(const VecInt &cards) const;
  bool IsSuitMeld(const VecInt &cards) const;
  
  VecVecInt RankMelds(VecInt cards) const;
  VecVecInt SuitMelds(VecInt cards) const;
  VecVecInt AllMelds(const VecInt &cards) const;
  
  bool VectorsIntersect(VecInt *v1, VecInt *v2) const;
  
  VecVecInt NonOverlappingMelds(VecInt *meld, VecVecInt *melds) const;
  
  void AllPaths(VecInt *meld, VecVecInt *all_melds, VecVecInt *path,
         VecVecVecInt *all_paths) const;
  
  VecVecVecInt AllMeldGroups(const VecInt &cards) const;
  
  VecVecInt BestMeldGroup(const VecInt &cards) const;
  
  int MinDeadwood(VecInt hand, absl::optional<int> card) const;
  int MinDeadwood(const VecInt &hand) const;
  
  int RankMeldLayoff(const VecInt &meld) const;
  VecInt SuitMeldLayoffs(const VecInt &meld) const;
  
  VecInt LegalMelds(const VecInt &hand, int knock_card) const;
  VecInt LegalDiscards(const VecInt &hand, int knock_card) const;
  
  VecInt AllLayoffs(const VecInt &layed_melds, const VecInt &previous_layoffs) const;
  
  int MeldToInt(VecInt meld) const;
  
  std::map<VecInt, int> BuildMeldToIntMap() const;
  std::map<int, VecInt> BuildIntToMeldMap() const;
  
  // TODO
  //static const std::map<int, VecInt> int_to_meld = BuildIntToMeldMap();
  //static const std::map<VecInt, int> meld_to_int = BuildMeldToIntMap();

  const std::map<int, VecInt> int_to_meld;
  const std::map<VecInt, int> meld_to_int;
};

}  // namespace gin_rummy
}  // namespace open_spiel

#endif  // OPEN_SPIEL_GAMES_GIN_RUMMY_UTILS_H_
