#include "Game.hpp"
#include "ThreadPool.hpp"
#include "Board.hpp"
#include <boost/bind.hpp>

using namespace std;

const double timeMargin = 0.005;
const int initialWidth = 40;
const int deepWidth = 20;
const int threadCount = 8;

Game::Game(const Options& options) :
		_options(options),
		_timeLimit(options.slowMode ? 10 : 0.5),
		_timeTaken(),
		_constDepth(7 + options.slowMode)
{
	_turn = PlayerColor::blackPlayer;
	_depth = _constDepth;
	std::cout << "depth: " << _depth << std::endl;
	_state = new Board(_turn);
	_state->fillTaboo(_options.doubleThree, _turn);
	_previousState = nullptr;
	_pool = new Pool(threadCount);

	std::random_device rd;
	_randomDevice = std::mt19937(rd());
}

Game::~Game()
{
	delete _pool;
	delete _state;
	delete _previousState;
}

Score Game::negamax(Board& node, int negDepth, Score alpha, Score beta, PlayerColor player)
{
	MoveScore children[BOARD_HEIGHT * BOARD_WIDTH];

	node.fillPriority(_options);
	int count = node.getChildren(children, deepWidth);

	if (!count)
		throw std::logic_error("GetChildren returned an empty array");

	size_t i;
	Score bestScore = ninfinity - 100;
	for (i = 0; i < (unsigned long)count; i++)
	{
		if (alpha <= beta && !isOverdue())
		{
			BoardPos pos = children[i].pos;
			Board *board = new Board(node, pos, player, _options);
			Score score;
			if (board->getVictory().type)
			{
				score = (pinfinity + negDepth) * (board->getVictory().victor * player);
			}
			else if (negDepth <= 1)
			{
				score = player * board->getScore(_options.captureWin);
			}
			else
			{
				score = -negamax(*board, negDepth - 1, -beta, -alpha, -player);
			}
			bestScore = std::max(bestScore, score);
			alpha = std::max(alpha, score);
			delete board;
		}
		else
		{
			break;
		}
	}

	return bestScore;
}

MoveScore Game::negamax_thread(ThreadData data)
{
	Score score;

	Board* board = data.node.board;
	BoardPos pos = data.node.move;

	if (isOverdue())
	{
		delete board;
		return MoveScore(ninfinity, pos);
	}
	if (data.alpha->load() > pinfinity)
	{
		delete board;
		return MoveScore(ninfinity, pos);
	}

	std::atomic<Score>* alpha = data.alpha;

	if (board->getVictory().type)
	{
		score = (pinfinity + _depth) * (board->getVictory().victor * data.player);
	}
	else
	{
		score = -negamax(*board, _depth - 1, ninfinity, -alpha->load(), -data.player);
	}

	alpha->store(std::max(alpha->load(), score));

	delete board;
	return (MoveScore(score, pos));
}

BoardPos Game::start_negamax(Board *node, PlayerColor player)
{
	std::atomic<Score> alpha(ninfinity);
	MoveScore children[BOARD_HEIGHT * BOARD_WIDTH];

	int count = node->getChildren(children, initialWidth);

	if (!count)
		throw std::logic_error("GetChildren returned an empty array");

	std::vector<ThreadData> threadData(count);

	for (size_t i = 0; i < (unsigned long)count; i++)
	{
		threadData[i] =	ThreadData(
				ChildBoard(
						new Board(*node, children[i].pos, player, _options),
						children[i].pos),
				&alpha,
				player);
	}


	std::function<MoveScore(ThreadData)> function = boost::bind(&Game::negamax_thread, this, _1);
#ifdef NON_THREADED
	std::vector<MoveScore> result;
	result.resize(threadData.size());
	for (size_t i = 0; i < threadData.size(); i++)
	{
		result[i] = function(threadData[i]);
	}
#else
	std::vector<MoveScore> result = _pool->run(function, threadData);
#endif

	MoveScore bestMove(ninfinity - 100);
	std::vector<MoveScore>	choice;
	for (size_t i = 0; i < result.size(); i++)
	{
		MoveScore move = result[i];

		if (move.score > bestMove.score)
		{
			choice.clear();
			bestMove = move;
			choice.push_back(bestMove);
		}
		else if (move.score == bestMove.score && move.score > ninfinity)
		{
			bestMove = move;
			choice.push_back(bestMove);
		}
	}

	std::uniform_int_distribution<int> uni(0, choice.size() - 1);
	return choice[uni(_randomDevice)].pos;
}


bool Game::isOverdue() const
{
	using namespace std;
	auto current = std::chrono::high_resolution_clock::now();

	double difference = std::chrono::duration_cast<std::chrono::milliseconds>(current - _start).count();

	return difference > (_timeLimit - timeMargin) * 1000;
}

double Game::getTimeDiff() const
{
	auto current = std::chrono::high_resolution_clock::now();
	double difference = double(std::chrono::duration_cast<std::chrono::milliseconds>(current - _start).count()) / 1000;
	return difference;
}

BoardPos Game::getNextMove()
{
	_start = std::chrono::high_resolution_clock::now();

	BoardPos pos = start_negamax(_state, _turn);

	_timeTaken = getTimeDiff();

	return pos;
}

Score Game::getCurrentScore() const
{
	return _state->getScore(_options.captureWin);
}

bool Game::play(BoardPos pos)
{
	if (_state->getCase(pos) != BoardSquare::empty)
		return false;
	if (_state->getPriority(pos) < 0)
		return false;

	delete _previousState;
	_previousState = _state;
	_state = new Board(*_previousState, pos, _turn, _options);

	_turn = -_turn;
	_state->fillTaboo(_options.doubleThree, _turn);
	_state->fillPriority(_options);

	return _state->getVictory().type;
}

bool Game::hasPosChanged(BoardPos pos) const
{
	if (_previousState == nullptr)
		return false;
	return _previousState->getCase(pos) != _state->getCase(pos);
}

double Game::getTimeTaken() const
{
	return _timeTaken;
}

bool Game::play()
{
	return play(getNextMove());
}

bool Game::isPlayerNext() const
{
	return
		(!_options.isBlackAI && getTurn() == blackPlayer) ||
		(!_options.isWhiteAI && getTurn() == whitePlayer);
}

Board *Game::getState()
{
	return (_state);
}

PlayerColor Game::getTurn() const
{
	return  (_turn);
}
