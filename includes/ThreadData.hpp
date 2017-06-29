#pragma once

#include <atomic>
#include "Board.hpp"
#include "PlayerColor.hpp"

class Game;

struct ThreadData
{
	ThreadData(){};
	ThreadData(ChildBoard _node, std::atomic<Score>* _alpha, PlayerColor _player):
			node(_node),
			alpha(_alpha),
			player(_player)
	{}

	~ThreadData(){};
	ChildBoard node;
	std::atomic<Score>* alpha;
	PlayerColor player;
};
