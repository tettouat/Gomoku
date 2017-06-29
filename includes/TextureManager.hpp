#pragma once

#include <SFML/Graphics.hpp>

struct SpriteManager
{
	sf::Texture				board;
	sf::Texture 			stone_black;
	sf::Texture 			stone_white;
	sf::Texture 			stone_suggestion;
	sf::Texture				stone_preview_black;
	sf::Texture				stone_preview_white;
	sf::Texture 			stone_preview_taboo;
	sf::Texture 			highlight;
	sf::Font				font;

	SpriteManager();
};
