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

#include <array>
#include <cstdio>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include <algorithm>
#include <unordered_set>

#include "open_spiel/abseil-cpp/absl/flags/flag.h"
#include "open_spiel/abseil-cpp/absl/flags/parse.h"
#include "open_spiel/abseil-cpp/absl/strings/str_join.h"
#include "open_spiel/abseil-cpp/absl/time/clock.h"
#include "open_spiel/abseil-cpp/absl/time/time.h"
#include "open_spiel/spiel.h"
#include "open_spiel/spiel_utils.h"
#include "open_spiel/spiel_bots.h"

#include "open_spiel/games/gin_rummy.h"
#include "open_spiel/games/gin_rummy/gin_rummy_utils.h"
#include "open_spiel/games/gin_rummy/gin_rummy_bot.h"

ABSL_FLAG(std::string, player1, "gin_bot", "Who controls player1.");
ABSL_FLAG(std::string, player2, "random", "Who controls player2.");
ABSL_FLAG(int, num_games, 1, "How many games to play.");
ABSL_FLAG(uint_fast32_t, seed, 0, "Seed for random bot.");
ABSL_FLAG(bool, verbose, false, "Show gameplay.");
ABSL_FLAG(bool, show_legals, false, "Show legal moves.");

uint_fast32_t Seed() {
  uint_fast32_t seed = absl::GetFlag(FLAGS_seed);
  return seed != 0 ? seed : absl::ToUnixMicros(absl::Now());
}

std::unique_ptr<open_spiel::Bot> InitBot(
    std::string type, const open_spiel::Game& game, open_spiel::Player player) {
  if (type == "random") {
    return open_spiel::MakeUniformRandomBot(player, Seed());
  }
  if (type == "gin_bot") {
    return std::make_unique<open_spiel::gin_rummy::GinBot>(player);
  }
  open_spiel::SpielFatalError("Bad player type. Known types: gin_bot, random");
}

open_spiel::Action GetAction(const open_spiel::State& state,
                             std::string action_str) {
  for (open_spiel::Action action : state.LegalActions()) {
    if (action_str == state.ActionToString(state.CurrentPlayer(), action))
      return action;
  }
  return open_spiel::kInvalidAction;
}

std::pair<std::vector<double>, std::vector<std::string>> PlayGame(
    const open_spiel::Game& game,
    const std::vector<std::unique_ptr<open_spiel::Bot>>& bots,
    std::mt19937& rng,
    const std::vector<std::string>& initial_actions) {
  bool verbose = absl::GetFlag(FLAGS_verbose);
  bool show_legals = absl::GetFlag(FLAGS_show_legals);
  std::unique_ptr<open_spiel::State> state = game.NewInitialState();
  std::vector<std::string> history;
  std::vector<open_spiel::Action> action_history;

  bots[0]->Restart();
  bots[1]->Restart();

  for (const auto& action_str : initial_actions) {
    open_spiel::Action action = GetAction(*state, action_str);
    if (action == open_spiel::kInvalidAction)
      open_spiel::SpielFatalError(absl::StrCat("Invalid action: ", action_str));

    history.push_back(action_str);
    state->ApplyAction(action);
    if (verbose) {
      std::cout << "Forced action" << action_str << std::endl;
      std::cout << "Next state:\n" << state->ToString() << std::endl;
    }
  }

  while (!state->IsTerminal()) {
    open_spiel::Player player = state->CurrentPlayer();
    if (verbose) std::cout << "player turn: " << player << std::endl;

    open_spiel::Action action;

    if (show_legals) {
      std::vector<open_spiel::Action> actions = state->LegalActions(player);
      std::cout << "Legal moves for player " << player << ":" << std::endl;
      for (open_spiel::Action action : actions) {
        std::cout << "  " << state->ActionToString(player, action) << std::endl;
      }
    }

    if (state->IsChanceNode()) {
      // Chance node; sample one according to underlying distribution.
      open_spiel::ActionsAndProbs outcomes = state->ChanceOutcomes();
      action =
          open_spiel::SampleAction(
              outcomes, std::uniform_real_distribution<double>(0.0, 1.0)(rng))
              .first;
      if (verbose) {
        std::cout << "Sampled action: " << state->ActionToString(player, action)
                  << std::endl;
      }
    } else {
      // Decision node, ask the right bot to make its action
      action = bots[player]->Step(*state);
      if (verbose) {
        std::cout << "Chose action: " << state->ActionToString(player, action)
                  << std::endl;
      }
    }
    for (open_spiel::Player p = 0; p < bots.size(); ++p) {
      if (p != player) {
        bots[p]->InformAction(*state, player, action);
      }
    }
    history.push_back(state->ActionToString(player, action));
    action_history.push_back(action);
    std::vector<open_spiel::Action> legal_actions = state->LegalActions();
    if (std::find(legal_actions.begin(), legal_actions.end(), action) ==
        legal_actions.end()) {
      std::cout << " Game actions: " << absl::StrJoin(history, " ")
                << std::endl;
      std::cout << " Game actions: " << absl::StrJoin(action_history, " ")
                << std::endl;
    }
    state->ApplyAction(action);

    if (verbose)
      std::cout << "State: " << std::endl << state->ToString() << std::endl;
  }
  if (verbose) {
    std::cout << "Returns: " << absl::StrJoin(state->Returns(), ",")
              << " Game actions: " << absl::StrJoin(history, " ")
              << " Action history: " << absl::StrJoin(action_history, " ")
              << std::endl;
  }
  return {state->Returns(), history};
}

int main(int argc, char** argv) {
  std::vector<char*> positional_args = absl::ParseCommandLine(argc, argv);
  std::mt19937 rng(Seed());

  std::string game_name = "gin_rummy";
  std::cout << "game: " << game_name << std::endl;
  std::shared_ptr<const open_spiel::Game> game =
      open_spiel::LoadGame(game_name);

  std::vector<std::unique_ptr<open_spiel::Bot>> bots;
  bots.push_back(InitBot(absl::GetFlag(FLAGS_player1), *game, 0));
  bots.push_back(InitBot(absl::GetFlag(FLAGS_player2), *game, 1));

  std::vector<std::string> initial_actions;
  for (int i = 1; i < positional_args.size(); ++i) {
    initial_actions.push_back(positional_args[i]);
  }

  std::map<std::string, int> histories;
  std::vector<double> overall_returns(2, 0);
  std::vector<int> overall_wins(2, 0);
  int num_games = absl::GetFlag(FLAGS_num_games);
  absl::Time start = absl::Now();
  for (int game_num = 0; game_num < num_games; ++game_num) {
    if (game_num % 1000 == 0)
      std::cout << "Game num: " << game_num << std::endl;
    auto[returns, history] = PlayGame(*game, bots, rng, initial_actions);
    histories[absl::StrJoin(history, " ")] += 1;
    for (int i = 0; i < returns.size(); ++i) {
      double v = returns[i];
      overall_returns[i] += v;
      if (v > 0) {
        overall_wins[i] += 1;
      }
    }
  }
  absl::Time end = absl::Now();
  double seconds = absl::ToDoubleSeconds(end - start);

  std::cout << "Number of games played: " << num_games << std::endl;
  std::cout << "Number of distinct games played: " << histories.size()
            << std::endl;
  std::cout << "Overall wins: " << absl::StrJoin(overall_wins, ",")
            << std::endl;
  std::cout << "Overall returns: " << absl::StrJoin(overall_returns, ",")
            << std::endl;
  std::cout << "Seconds: " << seconds << std::endl;

  return 0;
}
