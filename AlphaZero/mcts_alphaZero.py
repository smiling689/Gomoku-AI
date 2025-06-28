# -*- coding: utf-8 -*-
"""
Created on Fri Dec  7 22:05:17 2018

@author: initial
"""


import numpy as np
import copy
import math
import random

def softmax(x):
    probs = np.exp(x - np.max(x))
    probs /= np.sum(probs)
    return probs

class TreeNode(object):
    '''
    A node in the MCTS tree.
    Each node keeps track of its own value Q, prior probability P, and
    its visit-count-adjusted prior score u.
    '''

    def __init__(self, parent, prior_p):
        self._parent = parent
        self._children = {}  # a map from action to TreeNode
        self._n_visits = 0
        self._Q = 0
        self._u = 0
        self._P = prior_p # its the prior probability that action's taken to get this node

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
    '''
    An implementation of Monte Carlo Tree Search.
    '''
    # action_fc and evaluation_fc are not used in Pytorch version, just placeholders. see details in policy_value_net_pytorch.py
    def __init__(self, policy_value_fn,action_fc,evaluation_fc, is_selfplay,c_puct=5, n_playout=400):
        '''
        policy_value_fn: a function that takes in a board state and outputs
            a list of (action, probability) tuples and also a score in [-1, 1]
            (i.e. the expected value of the end game score from the current
            player's perspective) for the current player.
        c_puct: a number in (0, inf) that controls how quickly exploration
            converges to the maximum-value policy. A higher value means
            relying on the prior more.
        '''
        self._root = TreeNode(None, 1.0)
        # root node do not have parent ,and sure with prior probability 1

        self._policy_value_fn = policy_value_fn
        self._action_fc = action_fc
        self._evaluation_fc = evaluation_fc

        self._c_puct = c_puct
        # it's 5 in paper and don't change here,but maybe a better number exists in gomoku domain
        self._n_playout = n_playout # times of tree search
        self._is_selfplay = is_selfplay

    def _playout(self, state):
        """
        进行一次完整的选择->扩展->评估->反向传播
        """
        node = self._root
        # 1. Select
        while not node.is_leaf():
            action, node = node.select(self._c_puct)
            state.do_move(action)

        # 2. Expand & Evaluate
        # 调用神经网络获取先验概率和叶子节点的价值
        action_priors, leaf_value = self._policy_value_fn(state, self._action_fc, self._evaluation_fc)
        
        # 检查游戏是否实际已结束
        is_end, winner = state.game_end()
        if not is_end:
            node.expand(action_priors)
        else:
            if winner == -1:  # 平局
                leaf_value = 0.0
            else:
                leaf_value = 1.0 if winner == state.get_current_player() else -1.0
    
        # 反向传播
        while node is not None:
            node.update(-leaf_value)
            leaf_value = -leaf_value
            node = node._parent

    def update_with_move(self, last_move):
        """
        在棋盘上走一步后，复用搜索树。
        """
        if last_move in self._root._children:
            self._root = self._root._children[last_move]
            self._root._parent = None
        else:
            self._root = TreeNode(None, 1.0)
        
    def __str__(self):
        return "MCTS"

class MCTSPlayer(object):
    '''
    AI player based on MCTS
    '''
    def __init__(self, policy_value_function,action_fc,evaluation_fc,c_puct=5, n_playout=400, is_selfplay=0):
        '''
        init some parameters
        '''
        self._is_selfplay = is_selfplay
        self.policy_value_function = policy_value_function
        self.action_fc = action_fc
        self.evaluation_fc = evaluation_fc
        # self.first_n_moves = 12
        self.first_n_moves = 4
        # For the first n moves of each game, the temperature is set to τ = 1,
        # For the remainder of the game, an infinitesimal temperature is used, τ→ 0.
        # in paper n=30, here i choose 12 for 11x11, entirely by feel
        self.mcts = MCTS(policy_value_fn = policy_value_function,
                         action_fc = action_fc,
                         evaluation_fc = evaluation_fc,
                         is_selfplay = self._is_selfplay,
                         c_puct = c_puct,
                         n_playout = n_playout)

    def set_player_ind(self, p):
        '''
        set player index
        '''
        self.player = p

    def reset_player(self):
        '''
        reset player
        '''
        self.mcts.update_with_move(-1)

    def get_action(self,board,is_selfplay,print_probs_value):
        '''
        get an action by mcts
        do not discard all the tree and retain the useful part
        '''
        # sensible_moves = board.availables
        # # the pi vector returned by MCTS as in the alphaGo Zero paper
        # move, move_probs = None, np.zeros(board.width * board.height)
        # if len(sensible_moves) > 0:
            
        #     pass

        #     if print_probs_value and move_probs is not None:
        #         act_probs, value = self.policy_value_function(board,self.action_fc,self.evaluation_fc)
        #         print('-' * 10)
        #         print('value',value)
        #         # print the probability of each move
        #         probs = np.array(move_probs).reshape((board.width, board.height)).round(3)[::-1, :]
        #         for p in probs:
        #             for x in p:
        #                 print("{0:6}".format(x), end='')
        #             print('\r')

        #     return move,move_probs

        # else:
        #     print("WARNING: the board is full")

        self.mcts.update_with_move(board.last_move)
        
        sensible_moves = board.availables
        if len(sensible_moves) > 0:

            if is_selfplay:
            # 首次访问根节点时，先扩展
                if self.mcts._root.is_leaf():
                    action_priors, _ = self.mcts._policy_value_fn(board, self.mcts._action_fc, self.mcts._evaluation_fc)
                    self.mcts._root.expand(action_priors)
                
                # 添加噪声
                noise = np.random.dirichlet(0.3 * np.ones(len(self.mcts._root._children)))
                for i, node in enumerate(self.mcts._root._children.values()):
                    node._P = 0.75 * node._P + 0.25 * noise[i]

            # 运行 MCTS
            for n in range(self.mcts._n_playout):
                state_copy = copy.deepcopy(board)
                self.mcts._playout(state_copy)

            # 获取所有动作和对应的访问次数
            act_visits = [(act, node._n_visits)
                        for act, node in self.mcts._root._children.items()]
            acts, visits = zip(*act_visits)
            
            moves_played = board.width * board.height - len(board.availables)

            # 自我对弈时应当在前几步增加随机性
            if is_selfplay and moves_played < self.first_n_moves:
                temp = 1.0
            else:
                temp = 1e-3

            act_probs = softmax(1.0 / temp * np.log(np.array(visits) + 1e-10))
            
            # 从概率分布中抽样选择一个动作
            move = np.random.choice(acts, p=act_probs)

            # 更新
            self.mcts.update_with_move(move)

            if print_probs_value:
                pass

            # 创建一个完整的概率向量，用于训练
            move_probs = np.zeros(board.width * board.height)
            move_probs[list(acts)] = act_probs
            return move, move_probs
        else:
            print("WARNING: the board is full")

    def __str__(self):
        return "Alpha {}".format(self.player)


