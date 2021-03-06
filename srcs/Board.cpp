#include "Board.hpp"
#include <random>
#include <algorithm>

inline Score Board::fillScore()
{
	Score score = 0;
	for (int y = 0; y < BOARD_HEIGHT; y++)
	{
		for (int x = 0; x < BOARD_WIDTH; x++)
		{
			BoardSquare color = _data[y][x];
			if (color != BoardSquare::empty)
			{
				Score squareScore = 0;
				for (int dirX = -1; dirX <= 1; dirX++)
				{
					for (int dirY = -1; dirY <= 1; dirY++)
					{
						if (dirX || dirY)
						{
							const int maxX = CLAMP(x + 5 * dirX, -1, BOARD_WIDTH);
							const int maxY = CLAMP(y + 5 * dirY, -1, BOARD_HEIGHT);

							int value = 1;
							int emptyCount = 0;

							int _x = x + dirX;
							int _y = y + dirY;

							while ((dirX == 0 || _x != maxX) && (dirY == 0 || _y != maxY))
							{
								BoardSquare square = _data[_y][_x];
								if (square == empty)
								{
									squareScore += value;
									emptyCount++;
								}
								else if (square == color)
									value <<= 3;
								else
									break;
								_x += dirX, _y+= dirY;
							}
							value >>= 2;
							squareScore += value * emptyCount;
						}
					}
				}
				score += (color == BoardSquare::white) ? squareScore : -squareScore;
			}
		}
	}
	return score;
}

inline Score Board::getScore(bool considerCapture)
{
	Score score = fillScore();

	if (considerCapture)
	{
		score += 1 << _capturedBlacks / 2;
		score -= 1 << _capturedWhites / 2;
	}
	if (_victoryFlag == whitePlayer)
		score += pinfinity / 2;
	else if (_victoryFlag == blackPlayer)
		score -= pinfinity / 2;
	return score;
}

inline bool Board::isAlignedStoneDir(int x, int y, int dirX, int dirY, BoardSquare color, int size) const
{
	int ix = x + (size - 1) * -dirX;
	int iy = y + (size - 1) * -dirY;
	int mx = x + size * dirX;
	int my = y + size * dirY;

	while (ix < 0 || iy < 0 || ix >= BOARD_WIDTH || iy >= BOARD_HEIGHT)
		ix += dirX, iy += dirY;
	while (mx < -1 || my < -1 || mx > BOARD_WIDTH || my > BOARD_HEIGHT)
		mx -= dirX, my -= dirY;

	int count = 0;

	while ((dirX == 0 || ix != mx) && (dirY == 0 || iy != my))
	{
		if (_data[iy][ix] != color)
			count = 0;
		else if (++count == size)
			return true;
		ix += dirX, iy += dirY;
	}
	return false;
}

inline bool Board::isAlignedStonePos(int x, int y, int size) const
{
	BoardSquare c = _data[y][x];
	if (c == empty) return false;

	if (isAlignedStoneDir(x, y, 1, 0, c, size)) return true;
	if (isAlignedStoneDir(x, y, 1, 1, c, size)) return true;
	if (isAlignedStoneDir(x, y, 0, 1, c, size)) return true;
	if (isAlignedStoneDir(x, y, -1, 1, c, size)) return true;
	return false;
}

inline bool Board::isAlignedStone(int size) const
{
	for (int y = 0 ; y < BOARD_HEIGHT; ++y)
		for (int x = 0 ; x < BOARD_WIDTH; ++x)
			if (_data[y][x] != empty)
			if (isAlignedStonePos(x, y, size))
				return true;
	return false;
}

inline VictoryState  Board::calculateVictory(BoardPos pos, const Options& options)
{
	if (options.captureWin)
	{
		if (_capturedWhites >= captureVictoryPoints)
			return VictoryState(blackPlayer, VictoryType::captured);
		if (_capturedBlacks >= captureVictoryPoints)
			return VictoryState(whitePlayer, VictoryType::captured);
	}
	if (_victoryFlag == -_turn)
	{
		if (isAlignedStonePos(_alignmentPos.x, _alignmentPos.y, 5))
			return VictoryState(_victoryFlag, aligned);
		else
			_victoryFlag = nullPlayer;
	}

	if (isAlignedStonePos(pos.x, pos.y, 5))
	{
		if (options.capture)
		{
			_victoryFlag = _turn;
			_alignmentPos = pos;
		}
		else
			return VictoryState(_turn, aligned);
	}

	if (_turnNum - _capturedBlacks - _capturedWhites == BOARD_HEIGHT * BOARD_WIDTH)
	{
		return VictoryState(staleMate);
	}
	return  VictoryState(novictory);
}

inline VictoryState Board::getVictory()
{
	return (_victoryState);
}

inline size_t Board::getChildren(MoveScore* buffer, size_t count)
{
	MoveScore* bufferEnd = buffer;

	for (int y = 0; y < BOARD_HEIGHT; y++)
	{
		for (int x = 0; x < BOARD_WIDTH; x++)
		{
			if (_data[y][x] == empty)
			{
				int score = _priority[y][x];
				if (score > 0)
				{
					*bufferEnd = MoveScore(score, BoardPos(x, y));
					bufferEnd++;
				}
			}
		}
	}

	struct Sorter
	{
		Sorter(){};
		bool operator () (const MoveScore & a, const MoveScore & b)
		{
			return a.score > b.score;
		}
	};

	std::sort(buffer, bufferEnd, Sorter());

	size_t posCount = std::distance(buffer, bufferEnd);

	if (posCount < count)
		return posCount;
	return count;
}


inline bool Board::playCaptureDir(int x, int y, int dirX, int dirY, BoardSquare good) {

	BoardSquare bad = (good == BoardSquare::white ? BoardSquare::black : BoardSquare::white);

	if (x + 3*dirX < 0 || x + 3*dirX >= BOARD_WIDTH
		|| y + 3*dirY < 0 || y + 3*dirY >= BOARD_HEIGHT)
		return (false);
	return (_data[y][x] == good
			&& _data[y + dirY*1][x + dirX*1] == bad
			&& _data[y + dirY*2][x + dirX*2] == bad
			&& _data[y + dirY*3][x + dirX*3] == good);
}

inline int Board::playCapture(int x, int y) {
	BoardSquare c = _data[y][x];

	int capCount = 0;
	for (int dirX = -1 ; dirX <= 1; ++dirX)
		for (int dirY = -1 ; dirY <= 1; ++dirY)
			if (dirX || dirY) {
				if (playCaptureDir(x, y, dirX, dirY, c)) {
					_data[y + dirY * 1][x + dirX * 1] = BoardSquare::empty;
					_data[y + dirY * 2][x + dirX * 2] = BoardSquare::empty;
					++capCount;
				}
			}
	return capCount;
}

inline bool Board::checkFreeThree(int x, int y, int dirX, int dirY, BoardSquare enemy)
{
	int ix = x + 4 * -dirX;
	int iy = y + 4 * -dirY;
	int mx = x + 5 * dirX;
	int my = y + 5 * dirY;

	while (ix < 0 || iy < 0 || ix >= BOARD_WIDTH || iy >= BOARD_HEIGHT)
		ix += dirX, iy += dirY;
	while (mx < -1 || my < -1 || mx > BOARD_WIDTH || my > BOARD_HEIGHT)
		mx -= dirX, my -= dirY;

	BoardSquare buffer[6];
	int bufferIndex = 0;
	bool didLoop = false;

	while ((!dirX || ix != mx) && (!dirY || iy != my))
	{
		BoardSquare tmp = _data[iy][ix];
		buffer[bufferIndex] = tmp;

		++bufferIndex;

		if (bufferIndex == 6)
		{
			bufferIndex = 0;
			didLoop = true;
		}

		if (didLoop)
		{
			if (tmp == empty && buffer[bufferIndex] == empty)
			{
				int emptyCount = 0;
				bool foundEnemy = false;
				for (BoardSquare square:buffer)
				{
					if (square == enemy)
					{
						foundEnemy = true;
						break;
					}
					if (square == BoardSquare::empty)
						emptyCount++;
				}
				if (!foundEnemy && emptyCount == 4)
					return true;
			}
		}
		ix += dirX, iy += dirY;
	}
	return false;
}

constexpr int forbiddenZone = 4;
constexpr int middleX = (BOARD_WIDTH / 2);
constexpr int middleY = (BOARD_HEIGHT / 2);

inline void Board::fillTaboo(bool doubleThree, PlayerColor player)
{
	BoardSquare enemy = (player == blackPlayer)? white : black;
	if (doubleThree)
	{
		for (int y = 0; y < BOARD_HEIGHT; y++)
		{
			for (int x = 0; x < BOARD_WIDTH; x++)
			{
				if (_data[y][x] == BoardSquare::empty)
				{
					int count = 0;
					if (checkFreeThree(x, y, 1, 0, enemy)) count++;
					if (checkFreeThree(x, y, 1, 1, enemy)) count++;
					if (count >= 2) {_priority[y][x] = -1; continue;}
					if (checkFreeThree(x, y, 0, 1, enemy)) count++;
					if (count >= 2) {_priority[y][x] = -1; continue;}
					if (checkFreeThree(x, y, -1, 1, enemy)) count++;
					if (count >= 2) {_priority[y][x] = -1; continue;}
				}
			}
		}
	}
}


inline void Board::fillPriorityDir(int x, int y, int dirX, int dirY, BoardSquare color)
{
	const int maxX = CLAMP(x + 5 * dirX, -1, BOARD_WIDTH);
	const int maxY = CLAMP(y + 5 * dirY, -1, BOARD_HEIGHT);

	int value = 1;
	int count = 0;

	x += dirX, y+= dirY;
	while ((!dirX || x != maxX) && (!dirY || y != maxY))
	{
		BoardSquare square = _data[y][x];
		if (square == color)
		{
			value <<= 3;
		}
		else if (square == empty)
		{
			if (_priority[y][x] >= 0)
				_priority[y][x] += value;
		}
		else
		{
			count++;
			break;
		}
		x += dirX, y+= dirY;
		count++;
	}
	value >>= 2;
	while (count > 0)
	{
		count--;
		x -= dirX, y -= dirY;

		BoardSquare square = _data[y][x];
		if (square == empty && _priority[y][x] >= 0)
			_priority[y][x] += value;
	}
};

inline void Board::fillCapturePriorityDir(int x, int y, int dirX, int dirY, BoardSquare color)
{
	int endX = x + dirX * 3;
	int endY = y + dirY * 3;

	if (endX >= 0 && endX < BOARD_WIDTH && endY >= 0 && endY < BOARD_HEIGHT)
	if (_data[y + dirY * 1][x + dirX * 1] == color &&
		_data[y + dirY * 2][x + dirX * 2] == color &&
		_data[endY][endX] == empty)
	{
		_priority[endY][endX] += capturePriority;
	}
};

inline void Board::fillPriority(const Options& options)
{
	for (int y = 0; y < BOARD_HEIGHT; y++)
	{
		for (int x = 0; x < BOARD_WIDTH; x++)
		{
			BoardSquare color = _data[y][x];
			if (color != BoardSquare::empty)
			{
				BoardSquare enemyColor = (color == white) ? black : white;

				fillPriorityDir(x, y, -1, -1, color);
				fillPriorityDir(x, y, -1, 0, color);
				fillPriorityDir(x, y, -1, 1, color);
				fillPriorityDir(x, y, 0, -1, color);
				fillPriorityDir(x, y, 0, 1, color);
				fillPriorityDir(x, y, 1, -1, color);
				fillPriorityDir(x, y, 1, 0, color);
				fillPriorityDir(x, y, 1, 1, color);

				if (options.capture)
				{
					fillCapturePriorityDir(x, y, -1, -1, enemyColor);
					fillCapturePriorityDir(x, y, -1, 0, enemyColor);
					fillCapturePriorityDir(x, y, -1, 1, enemyColor);
					fillCapturePriorityDir(x, y, 0, -1, enemyColor);
					fillCapturePriorityDir(x, y, 0, 1, enemyColor);
					fillCapturePriorityDir(x, y, 1, -1, enemyColor);
					fillCapturePriorityDir(x, y, 1, 0, enemyColor);
					fillCapturePriorityDir(x, y, 1, 1, enemyColor);
				}
			}
		}
	}
}

inline Board::Board(PlayerColor player):
				_data(),
				_priority(),
				_capturedWhites(),
				_capturedBlacks(),
				_turnNum(),
				_turn(player),
				_victoryFlag(nullPlayer),
				_alignmentPos()
{
	_priority[9][9] = 1;
}

inline Board::Board(const Board& board)
{
	*this = board;
	bzero(_priority, sizeof(_priority));
}

inline Board::Board(const Board& board, BoardPos move, PlayerColor player, const Options& options) : Board(board)
{
	if (player == PlayerColor::whitePlayer)
		_data[move.y][move.x] = BoardSquare::white;
	else
		_data[move.y][move.x] = BoardSquare::black;

	if (options.capture)
	{
		int captures = playCapture(move.x, move.y);
		if (player == PlayerColor::whitePlayer)
			_capturedBlacks += 2 * captures;
		else
			_capturedWhites += 2 * captures;
	}
	_victoryState = calculateVictory(move, options);
	_turnNum = board._turnNum + 1;
	_turn = (PlayerColor) -(board._turn);
}

inline Board::~Board() { }

inline MoveScore Board::getBestPriority() const
{
	MoveScore best = MoveScore(-1, BoardPos());

	for (BoardPos current; current != BoardPos::boardEnd(); ++current)
	{
		int p = getPriority(current);
		if (p > best.score)
			best = MoveScore(p, current);
	}
	return (best);
}
