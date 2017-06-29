#include "TextureManager.hpp"


SpriteManager::SpriteManager()
{
	if (!board.loadFromFile("./textures/board.png"))
	{
		throw std::logic_error("Could not load board texture");
	}
	if (!stone_black.loadFromFile("./textures/stone_black.png"))
	{
		throw std::logic_error("Could not load black stone texture");
	}
	if (!stone_white.loadFromFile("./textures/stone_white.png"))
	{
		throw std::logic_error("Could not load white stone texture");
	}
	if (!stone_suggestion.loadFromFile("./textures/stone_suggestion.png"))
	{
		throw std::logic_error("Could not load suggestion stone texture");
	}
	if (!stone_preview_black.loadFromFile("./textures/stone_preview_black.png"))
	{
		throw std::logic_error("Could not load preview black stone texture");
	}
	if (!stone_preview_white.loadFromFile("./textures/stone_preview_white.png"))
	{
		throw std::logic_error("Could not load preview black stone texture");
	}
	if (!stone_preview_taboo.loadFromFile("./textures/stone_preview_taboo.png"))
	{
		throw std::logic_error("Could not load preview taboo stone texture");
	}
	if (!font.loadFromFile("./textures/font.otf"))
	{
		throw std::logic_error("Could not load preview taboo stone texture");
	}
	if (!highlight.loadFromFile("./textures/highlight.png"))
	{
		throw std::logic_error("Could not load preview highlight texture");
	}
	stone_black.setSmooth(true);
	stone_white.setSmooth(true);
	stone_suggestion.setSmooth(true);
	stone_preview_white.setSmooth(true);
	stone_preview_black.setSmooth(true);
	stone_preview_taboo.setSmooth(true);
	highlight.setSmooth(true);
}
