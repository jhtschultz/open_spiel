# Copyright 2022 DeepMind Technologies Limited
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Tabular Q-learning agent tests."""

from absl.testing import absltest
import numpy as np

from open_spiel.python import rl_environment
from open_spiel.python import rl_tools
from open_spiel.python.algorithms import tabular_qlearner
import pyspiel
from open_spiel.python.utils import stats

# Fixed seed to make test non stochastic.
SEED = 10000

# A simple two-action game encoded as an EFG game. Going left gets -1, going
# right gets a +1.
SIMPLE_EFG_DATA = """
  EFG 2 R "Simple single-agent problem" { "Player 1" } ""
  p "ROOT" 1 1 "ROOT" { "L" "R" } 0
    t "L" 1 "Outcome L" { -1.0 }
    t "R" 2 "Outcome R" { 1.0 }
"""


def blackjack_info_state_to_string(state):
  if state.is_terminal():
    return str(state)
  else:
    return (
        "Terminal? False\n"
        f"Dealer visible card: {state.dealers_visible_card()}\n"
        f"Player sum: {state.get_best_player_total(0)}\n"
    )


class QlearnerTest(absltest.TestCase):

  def test_simple_game(self):
    game = pyspiel.load_efg_game(SIMPLE_EFG_DATA)
    env = rl_environment.Environment(game=game)

    agent = tabular_qlearner.QLearner(0, game.num_distinct_actions())
    total_reward = 0

    for _ in range(100):
      total_eval_reward = 0
      for _ in range(1000):
        time_step = env.reset()
        while not time_step.last():
          agent_output = agent.step(time_step)
          time_step = env.step([agent_output.action])
          total_reward += time_step.rewards[0]
        agent.step(time_step)
      self.assertGreaterEqual(total_reward, 75)
      for _ in range(1000):
        time_step = env.reset()
        while not time_step.last():
          agent_output = agent.step(time_step, is_evaluation=True)
          time_step = env.step([agent_output.action])
          total_eval_reward += time_step.rewards[0]
      self.assertGreaterEqual(total_eval_reward, 250)

  @absltest.skip("Example when using different info state string.")
  def test_blackjack(self):
    game = pyspiel.load_game("blackjack")
    env = rl_environment.Environment(game=game, include_full_state=True)

    agent = tabular_qlearner.QLearner(
        0,
        game.num_distinct_actions(),
        info_state_to_string_override=blackjack_info_state_to_string,
        epsilon_schedule=rl_tools.ConstantSchedule(0.1),
    )

    train_return_swa = stats.SlidingWindowAccumulator(100000)
    eval_return_swa = stats.SlidingWindowAccumulator(100000)

    for i in range(1000):
      for _ in range(1000):
        episode_return = 0
        time_step = env.reset()
        while not time_step.last():
          agent_output = agent.step(time_step)
          time_step = env.step([agent_output.action])
          episode_return += time_step.rewards[0]
        agent.step(time_step)
        train_return_swa.add(episode_return)
      for _ in range(1000):
        episode_return = 0
        time_step = env.reset()
        while not time_step.last():
          agent_output = agent.step(time_step, is_evaluation=True)
          time_step = env.step([agent_output.action])
          episode_return += time_step.rewards[0]
        eval_return_swa.add(episode_return)
      print(
          f"epoch {i}, avg train return: {train_return_swa.mean()}, "
          + f"avg eval reward: {eval_return_swa.mean()}"
      )


if __name__ == "__main__":
  np.random.seed(SEED)
  absltest.main()
