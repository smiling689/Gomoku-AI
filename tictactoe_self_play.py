from Board.game_board import Board, Game
from AlphaZero.policy_value_net_torch import PolicyValueNet
from AlphaZero.mcts_alphaZero import MCTSPlayer
from collections import Counter


def self_play_draw_check(
    board_width=3, board_height=3, n_in_row=3, model_path=None, n_games=100
):
    # 创建棋盘和游戏环境
    board = Board(width=board_width, height=board_height, n_in_row=n_in_row)
    game = Game(board)

    # 加载AI模型
    policy_value_net = PolicyValueNet(
        board_width, board_height, block=1, init_model=model_path, cuda=False
    )
    mcts_player = MCTSPlayer(
        policy_value_function=policy_value_net.policy_value_fn_random,
        action_fc=policy_value_net.action_fc_test,
        evaluation_fc=policy_value_net.evaluation_fc2_test,
        c_puct=5,
        n_playout=25,
        is_selfplay=False,
    )

    draws = 0
    results = []

    for i in range(n_games):
        winner = game.start_play(
            mcts_player, mcts_player, start_player=i % 2, is_shown=0
        )
        if winner == -1:
            draws += 1
        results.append(winner)
    counter = Counter(results)
    print(f"总对局数: {n_games}")
    print(f"和局次数: {counter[-1]}")
    print(f"玩家1胜利次数: {counter[1]}")
    print(f"玩家2胜利次数: {counter[2]}")
    print(f"和局率: {counter[-1] / n_games:.2%}")
    return draws, results


if __name__ == "__main__":
    # 传入你的模型路径
    model_path = "model/3_3_3.model"
    self_play_draw_check(
        board_width=3, board_height=3, n_in_row=3, model_path=model_path, n_games=100
    )
