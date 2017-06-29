#pragma once

enum PlayerColor
{
	blackPlayer = -1,
	nullPlayer  = 0,
	whitePlayer = 1,
};

PlayerColor operator-(PlayerColor& rhs);
