#ifndef BOARD_HPP
# define BOARD_HPP

class Board;

#pragma once

#include <vector>
#include <tuple>
#include "Constants.hpp"
#include "BoardPos.hpp"
#include "PlayerColor.hpp"
#include "Options.hpp"

enum BoardSquare
{
	empty = 0,
	black = 1,
	white = 2,
};

using BoardData = BoardSquare[BOARD_HEIGHT][BOARD_WIDTH];
using BoardScore = short[BOARD_HEIGHT][BOARD_WIDTH];

enum VictoryType
{
	novictory = 0,
	aligned = 1,
	captured = 2,
	staleMate = 3,
};

struct VictoryState
{
	PlayerColor victor;
	VictoryType type;

	VictoryState(PlayerColor _victor, VictoryType _type):victor(_victor),type(_type){};
	VictoryState(VictoryType _type):victor(nullPlayer),type(_type){};
	VictoryState():victor(nullPlayer),type(novictory){};
};

struct MoveScore
{
	Score score;
	BoardPos pos;

	MoveScore(Score _score, BoardPos _pos):score(_score), pos(_pos){}
	MoveScore(Score _score):score(_score),pos(){}
	MoveScore():score(),pos(){}
};

struct ChildBoard
{
	Board *board;
	BoardPos move;

	ChildBoard():board(),move(){};
	ChildBoard(Board *_board, BoardPos _move):board(_board),move(_move){};
};

class Board
{
	friend class AnalyzerBrainDead;
public:
	Board(PlayerColor player);
	Board(const Board& board);
	Board(const Board& board, BoardPos move, PlayerColor player, const Options& options);
	virtual ~Board();

	VictoryState	getVictory();
	size_t			getChildren(MoveScore* buffer, size_t count);

	void 			fillTaboo(bool doubleThree, PlayerColor player);
	void 			fillPriority(const Options& options);
	MoveScore		getBestPriority() const;
	bool			checkFreeThree(int x, int y, int dirX, int dirY, BoardSquare enemy);
	int 			playCapture(int x, int y);
	bool			isAlignedStone(int size) const;

	Score			fillScore();
	Score			getScore(bool considerCapture);

	BoardData*		getData() { return &_data; }
	BoardSquare		getCase(BoardPos pos) const { return (_data[pos.y][pos.x]); }
	BoardSquare&	getCase(BoardPos pos) { return (_data[pos.y][pos.x]); }
	BoardSquare		getCase(int x, int y) const {return (_data[y][x]);};
	BoardSquare&	getCase(int x, int y) {return (_data[y][x]);};
	int 			getCapturedBlack() const {return (_capturedBlacks);}
	int 			getCapturedWhite() const {return (_capturedWhites);}
	int 			getPriority(int x, int y) const {return _priority[y][x];}
	int 			getPriority(BoardPos pos) const {return _priority[pos.y][pos.x];}
	bool 			isFlaggedFinal() const { return _victoryFlag; };

private:
	BoardData		_data;
	BoardScore		_priority;
	int				_capturedWhites;
	int				_capturedBlacks;
	int 			_turnNum;
	PlayerColor 	_turn;
	PlayerColor 	_victoryFlag;
	BoardPos		_alignmentPos;
	VictoryState	_victoryState;

	VictoryState	calculateVictory(BoardPos pos, const Options& options);

	void 			fillPriorityDir(int x, int y, int dirX, int dirY, BoardSquare color);//, int bonus);
	void 			fillCapturePriorityDir(int x, int y, int dirX, int dirY, BoardSquare color);

	bool 			isAlignedStoneDir(int x, int y, int dirX, int dirY, BoardSquare good, int size) const;
	bool		 	isAlignedStonePos(int x, int y, int size) const;
	bool 			playCaptureDir(int x, int y, int dirX, int dirY, BoardSquare type);
};

#include "../srcs/Board.cpp"

#endif
