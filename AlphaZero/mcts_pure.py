# -*- coding: utf-8 -*-
"""
Created on Fri Dec  7 21:19:11 2018

@author: initial-h
"""

import numpy as np
import copy
from operator import itemgetter
from collections import defaultdict

def policy_value_fn(board):
    '''
    a function that takes in a state and outputs a list of (action, probability)
    tuples and a score for the state
    '''
    # return uniform probabilities and 0 score for pure MCTS
    action_probs = np.ones(len(board.availables))/len(board.availables)
    return zip(board.availables, action_probs), 0

class TreeNode(object):
    '''
    A node in the MCTS tree. Each node keeps track of its own value Q,
    prior probability P, and its visit-count-adjusted prior score u.
    '''

    def __init__(self, parent, prior_p):
        self._parent = parent # 对父节点的引用，根节点为none
        self._children = {}  # a map from action to TreeNode
        self._n_visits = 0 # 访问次数
        self._Q = 0 # 平均回报
        self._u = 0 # UCT，用公式算
        self._P = prior_p # 先验概率。在纯MCTS中，当一个节点被扩展时，它的所有子节点的P可以初始化为均匀分布。

    def get_u(self , c_puct):
        self._u = self._Q + c_puct * self._P * (self._parent._n_visits ** 0.5) / (1 + self._n_visits)
        return self._u

    def expand(self , action_prior):
        for act , prob in action_prior:
            if act not in self._children:
                self._children[act] = TreeNode(self, prob)

    def select(self , c_puct):
        max_u = 0
        max_child = None
        for childs in self._children:
            if(childs[1].get_u(c_puct) > max_u):
                max_child = childs
        return max_child
    
    def update(self , leaf_val):
        self._n_visits += 1
        self._Q += (leaf_val - self._Q) / self._n_visits

    def is_leaf(self):
        return self._children == {}
    


class MCTS(object):
    '''
    A simple implementation of Monte Carlo Tree Search.
    '''
    def __init__(self, policy_value_fn, c_puct=5, n_playout=400):
        '''
        policy_value_fn: a function that takes in a board state and outputs
            a list of (action, probability) tuples and also a score in [-1, 1]
            (i.e. the expected value of the end game score from the current
            player's perspective) for the current player.
        c_puct: a number in (0, inf) that controls how quickly exploration
            converges to the maximum-value policy. A higher value means
            relying on the prior more.
        '''
        self._root = TreeNode(parent=None, prior_p=1.0)
        # root node do not have parent ,and sure with prior probability 1
        self._policy = policy_value_fn
        self._c_puct = c_puct
        self._n_playout = n_playout # times of tree search

    pass

    def __str__(self):
        return "MCTS"

class MCTSPlayer(object):
    '''
    AI player based on MCTS
    '''
    def __init__(self, c_puct=5, n_playout=400):
        '''
        init a mcts class
        '''
        self.mcts = MCTS(policy_value_fn, c_puct, n_playout)

    def set_player_ind(self, p):
        '''
        set player index
        '''
        self.player = p

    def reset_player(self):
        '''
        reset player
        '''
        self.mcts.update_with_move(-1) # reset the node

    def get_action(self, board,is_selfplay=False,print_probs_value=0):
        '''
        get an action by mcts
        do not discard all the tree and retain the useful part
        '''
        move = None

        pass

        return move, None

    def __str__(self):
        return "MCTS {}".format(self.player)








