#include "PlayerColor.hpp"

PlayerColor operator-(PlayerColor &rhs)
{
	return (static_cast<PlayerColor >(-static_cast<int>(rhs)));
}
