# -*- coding: utf-8 -*-
import numpy as np
import copy
import math
import random
from operator import itemgetter
from collections import defaultdict

"""
下面简单引入一下 Board/game_board.py 中的 Board 类，其中的一些接口会在下面的函数中用到，以便我自己查看。

1. 初始化棋盘
    接口: board = Board(width=15, height=15, n_in_row=5)
    功能: 创建一个棋盘对象。你可以指定棋盘的宽度、高度和胜利所需的连子数。
    如何使用: 在你的主程序中，你需要先创建 Board 的一个实例。
    接口: board.init_board()
    功能: 重置棋盘到初始状态。它会清空棋子 (states)，重置可用位置 (availables) 和当前玩家 (current_player)。
    如何使用: 在每一次MCTS模拟开始前，你需要在一个棋盘的副本上调用这个方法，以确保模拟是从一个干净的初始状态开始的。
        不过，更高效的做法是直接深拷贝（copy.deepcopy）当前的棋盘状态，这样就不需要 init_board 了。init_board 主要用于开始一盘全新的游戏。

2. 获取棋盘状态
    接口: board.availables
    功能: 这是一个属性（列表），包含了当前所有可以落子的位置。每个位置用一个整数表示。
    如何使用:
        在**扩展(Expansion)**阶段，你需要遍历这个列表来创建新的子节点。
        在**模拟(Simulation)**阶段，你需要从这个列表中随机选择一个位置来下“随机棋”。
    示例: random_move = random.choice(board.availables)
    接口: board.get_current_player()
    功能: 返回当前轮到哪位玩家下棋（玩家1或玩家2）。
    如何使用: 在**反向传播(Backpropagation)**阶段，你需要知道模拟结果（胜利者）是否就是当前节点对应的玩家，从而决定是给予正奖励（+1）还是负奖励（-1）。
    示例: player = board.get_current_player()

3. 操作棋盘
    接口: board.do_move(move)
    功能: 在棋盘上执行一步棋。它会更新棋盘上的棋子、从 availables 列表中移除该位置，并自动切换到下一位玩家。
    如何使用: 在MCTS的**选择(Selection)和模拟(Simulation)**步骤中，当你决定走一步棋时，就调用这个方法来推进棋局。
    示例: board_copy.do_move(some_action)

4. 判断游戏结局
    接口: board.game_end()
    功能: 判断游戏是否结束。
    返回值: 一个元组 (is_end, winner)。
        is_end (布尔值): True 表示游戏结束，False 表示尚未结束。
        winner (整数): 如果有胜利者，则返回胜利的玩家（1或2）。如果是平局，返回-1。如果游戏未结束，也返回-1。
    如何使用: 在**模拟(Simulation)**阶段，每走一步棋后都要调用此方法，以检查游戏是否结束。这也是你模拟循环的终止条件。
    示例:
        # 在模拟循环中
        is_end, winner = board_copy.game_end()
        if is_end:
            # 根据winner计算奖励值
            break # 结束模拟

可以忽略的接口
    current_state(): 这个方法是为AlphaZero的神经网络准备的，它返回一个复杂的多层特征平面，用于输入给神经网络。
        对于纯MCTS，你完全不需要这个方法。MCTS的决策仅依赖于模拟胜率，而非复杂的特征表示。
    move_to_location() 和 location_to_move(): 这两个是工具函数，用于整数位置和二维坐标之间的转换。
        你的MCTS算法可以直接使用整数位置，所以通常用不到它们，除非你想在调试时打印出坐标方便查看。
"""


def policy_value_fn(board):
    """
    a function that takes in a state and outputs a list of (action, probability)
    tuples and a score for the state
    """
    # return uniform probabilities and 0 score for pure MCTS
    action_probs = np.ones(len(board.availables)) / len(board.availables)
    return zip(board.availables, action_probs), 0


class TreeNode(object):
    """
    A node in the MCTS tree. Each node keeps track of its own value Q,
    prior probability P, and its visit-count-adjusted prior score u.
    """

    def __init__(self, parent, prior_p):
        self._parent = parent  # 对父节点的引用，根节点为none
        self._children = {}  # a map from action to TreeNode，是一个字典
        self._n_visits = 0  # 访问次数
        self._Q = 0  # 平均回报
        self._u = 0  # UCT，用公式算
        self._P = prior_p  # 先验概率。在纯MCTS中，当一个节点被扩展时，它的所有子节点的P可以初始化为均匀分布。

    # 完全按照 UCT 公式返回，c_puct 是 UCT 公式中的 c
    def get_u(self, c_puct):
        self._u = self._Q + c_puct * self._P * (self._parent._n_visits**0.5) / (
            1 + self._n_visits
        )
        return self._u

    # 扩展，对于所有的子节点，如果不存在，则创建一个节点
    def expand(self, action_prior):
        for act, prob in action_prior:
            if act not in self._children:
                self._children[act] = TreeNode(self, prob)

    # 返回一个元组 (action, next_node)
    # 根据 UCT 值返回最好的点
    def select(self, c_puct):
        max_u = -math.inf
        best_child = None
        best_act = None
        for action , child in self._children.items():
            u = child.get_u(c_puct)
            if u > max_u:
                best_child = child
                best_act = action
                max_u = u
        return best_act , best_child

    # 访问次数增加1，Q值更新
    def update(self, leaf_val):
        self._n_visits += 1
        self._Q += (leaf_val - self._Q) / self._n_visits

    # 判断是否为叶子结点
    def is_leaf(self):
        return self._children == {}


class MCTS(object):
    # 初始化 MCTS 搜索树，根节点为 _root ，先验概率为1.0，接受一个策略价值函数
    def __init__(self, policy_value_fn, c_puct=5, n_playout=400):
        """
        policy_value_fn:一个策略-价值函数，输入棋盘状态,输出一个tuple:(每个可行动作,概率)和当前局面的分数。
            c_puct:控制探索与利用之间权衡的超参数，影响搜索时对先验概率的依赖程度。
            n_playout:每次决策时模拟（搜索）多少次。
        policy_value_fn: a function that takes in a board state and outputs
            a list of (action, probability) tuples and also a score in [-1, 1]
            (i.e. the expected value of the end game score from the current
            player's perspective) for the current player.
        c_puct: a number in (0, inf) that controls how quickly exploration
            converges to the maximum-value policy. A higher value means
            relying on the prior more.
        """
        self._root = TreeNode(parent=None, prior_p=1.0)
        # root node do not have parent ,and sure with prior probability 1
        self._policy = policy_value_fn
        self._c_puct = c_puct
        self._n_playout = n_playout  # times of tree search

    def __str__(self):
        return "MCTS"

    # 执行一次完整的MCTS四步流程：
    # 从根节点选择到叶子节点，扩展叶子节点，进行随机模拟，最后反向传播更新结果。
    # 需要一个state参数，它是当前棋盘状态的一个深拷贝，以避免修改原始棋盘。
    def _playout(self, state):
        node = self._root

        # 步骤 1: 选择
        while not node.is_leaf():
            # 找到最佳子节点
            action, node = node.select(self._c_puct)
            # 在棋盘副本上执行这个动作
            state.do_move(action) 

        # 步骤 2: 扩展
        # policy_value_fn 返回所有可用动作的均匀概率
        action_priors, _ = self._policy(state)
        # 检查游戏是否已经结束
        is_end, winner = state.game_end() 
        if not is_end:
            # 如果游戏没结束，扩展该叶子节点
            node.expand(action_priors)

        # 步骤 3: 模拟
        # 从当前叶子节点状态开始，随机走子直到游戏结束
        leaf_value = self._simulate_rollout(state)

        # 步骤 4: 反向传播
        # 每一层的贡献值需要变号
        # update函数会更新节点的访问次数 _n_visits 和平均回报 _Q
        while node is not None:
            # 用负的leaf_value
            node.update(-leaf_value)
            leaf_value = -leaf_value
            node = node._parent

    def _simulate_rollout(self, state):
        current_player = state.get_current_player()
        while True:
            is_end, winner = state.game_end()
            if is_end:
                break
            # 从可用位置中随机选择一个
            random_action = random.choice(state.availables)
            state.do_move(random_action)

        if winner == -1:  # 平局
            return 0
        # 如果胜利者是这次模拟开始时的玩家，返回1，否则返回-1
        return 1 if winner == current_player else -1

    def update_with_move(self, last_move):
        if last_move in self._root._children:
            # 如果上一步棋在子节点中，就将根节点更新为该子节点
            self._root = self._root._children[last_move]
            self._root._parent = None  # 新的根节点没有父节点
        else:
            # 重置为一个全新的根节点
            self._root = TreeNode(parent=None, prior_p=1.0)


class MCTSPlayer(object):
    """
    AI player based on MCTS
    """

    def __init__(self, c_puct=5, n_playout=400):
        """
        init a mcts class
        """
        self.mcts = MCTS(policy_value_fn, c_puct, n_playout)

    def set_player_ind(self, p):
        """
        set player index
        """
        self.player = p

    def reset_player(self):
        """
        reset player
        """
        self.mcts.update_with_move(-1)  # reset the node

    def get_action(self, board, is_selfplay=False, print_probs_value=0):
        """
        get an action by mcts
        do not discard all the tree and retain the useful part
        这是AI决策的入口函数。它调用MCTS类来运行指定次数的模拟,然后根据模拟结果选择最佳动作。应选择被访问次数最多的那个动作 。
        """
        self.mcts.update_with_move(board.last_move)
        move = None
        if len(board.availables) == 0:
            return move , None
        for n in range(self.mcts._n_playout):
            board_copy = copy.deepcopy(board)
            self.mcts._playout(board_copy)

        # 从根节点的子节点中选择最佳动作
        # 获取所有子节点的 (动作, 访问次数)
        child_visits = []
        for action, node in self.mcts._root._children.items():
            child_visits.append((action, node._n_visits))

        # 如果没有子节点被访问过，随机返回一个
        if not child_visits:
            return random.choice(board.availables), None

        # 找出访问次数最多的动作
        best_action, _ = max(child_visits, key=itemgetter(1))

        self.mcts.update_with_move(best_action)

        # 第二个返回值在纯MCTS中用不到
        return best_action, None

    def __str__(self):
        return "MCTS {}".format(self.player)
