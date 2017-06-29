#include "GUIManager.hpp"

#include <sstream>
#include <iostream>
#include "Game.hpp"

int		turn;

GUIManager::GUIManager()
		:sf::RenderWindow(sf::VideoMode(screen_width, screen_height), "Gomoku", sf::Style::Titlebar | sf::Style::Close),
		_w(screen_width),
		_h(screen_height),
		_colorBG(173, 216, 230),
		_colorLine(0, 0, 128)
{

}

SpriteManager GUIManager::_textures = SpriteManager();

template<class T>
void		centerOnPos(T& target, float x, float y)
{
	sf::FloatRect textRect = target.getLocalBounds();
	target.setOrigin(textRect.left + textRect.width/2.0f,
					 textRect.top  + textRect.height/2.0f);
	target.setPosition(x, y);
}

GUIManager::~GUIManager() { this->close(); }

GUIManager::MenuButton		GUIManager::getMenuButton()
{
	auto mousePos = getMouseScreenRatio();

	int y = mousePos.y * 4;

	return (static_cast<MenuButton >(y));
}

bool	GUIManager::getMouseBoardPos(BoardPos& pos)
{
	pos = BoardPos(
			(sf::Mouse::getPosition(*this).x - screen_margin_x - board_offset_x + cell_width / 2) / cell_width,
			(sf::Mouse::getPosition(*this).y - screen_margin_y - board_offset_y + cell_width / 2) / cell_height);
	if (pos.x >= 0 && pos.x < BOARD_WIDTH)
		if (pos.y >= 0 && pos.y < BOARD_HEIGHT)
			return true;
	return false;
}

sf::Vector2f	GUIManager::getMouseScreenRatio()
{
	sf::Vector2i valueInt = sf::Mouse::getPosition(*this);
	sf::Vector2f value = sf::Vector2f(valueInt.x, valueInt.y);
	value.x /= _w;
	value.y /= _h;
	return value;
}

void	GUIManager::drawBoard(Game& g, Options options, const std::string message)
{
	BoardSquare 		c;
	sf::Sprite			highlight(_textures.highlight);
	sf::Sprite			background(_textures.board);
	sf::Sprite			sprite_black(_textures.stone_black);
	sf::Sprite			sprite_white(_textures.stone_white);
	sf::Sprite			sprite_preview_black(_textures.stone_preview_black);
	sf::Sprite			sprite_preview_white(_textures.stone_preview_white);
	sf::Sprite			sprite_preview_taboo(_textures.stone_preview_taboo);
	sf::Sprite			sprite_preview_taboo_mouse(_textures.stone_preview_taboo);
	sf::Sprite			sprite_suggestion(_textures.stone_suggestion);
	sf::Text			text_victory;

	Board &b = *g.getState();
	bool isPlayerNext = g.isPlayerNext();

	BoardPos mousePos;
	getMouseBoardPos(mousePos);

	background.setPosition(0, 0);

	if (b.isFlaggedFinal())
		background.setColor(sf::Color(123, 255, 255));
	else
		background.setColor(sf::Color(255, 255, 255));

	this->draw(background);
	std::string aff = "Turn : ";
	aff += std::to_string(turn);
	sf::Text	test(aff, _textures.font, 30);
	centerOnPos(test, _w / 2, 11);
	this->draw(test);

	MoveScore bestPriority = b.getBestPriority();

	for (BoardPos pos = BoardPos(); pos != BoardPos::boardEnd(); ++pos)
	{
		c = b.getCase(pos);
		int p = b.getPriority(pos);
		bool isHighlighted = g.hasPosChanged(pos);

		sf::Sprite* sprite = nullptr;
		sf::Text* text = nullptr;
		switch (c)
		{
			case BoardSquare::empty:
				if (p == -1)
				{
					if (pos == mousePos && isPlayerNext)
						sprite = &sprite_preview_taboo_mouse;
					else
						sprite = &sprite_preview_taboo;
				}
				else
				{
					if (p > 0)
					{
						text_victory = sf::Text(std::to_string(p), _textures.font, 20);
						text = &text_victory;
						text->setFillColor(sf::Color(0, 0, 0));
					}
					if (pos == mousePos && !message.size() && isPlayerNext)
					{
						if(g.getTurn() == PlayerColor::blackPlayer)
							sprite = &sprite_preview_black;
						else
							sprite =&sprite_preview_white;
					}
					else if (isPlayerNext && p == bestPriority.score && options.showTips) // pos == bestPriority)
					{
						if(g.getTurn() == PlayerColor::blackPlayer)
							sprite = &sprite_preview_black;
						else
							sprite =&sprite_preview_white;
					}

				}
				break ;
			case BoardSquare::white:
				sprite = &sprite_white;
				break ;
			case BoardSquare::black:
				sprite = &sprite_black;
				break ;
		}
		if (isHighlighted)
		{
			highlight.setScale(cell_width / 100., cell_width / 100.);
			centerOnPos(highlight,
						screen_margin_x + board_offset_x + pos.x * cell_width,
						screen_margin_y + board_offset_y + pos.y * cell_height);
			this->draw(highlight);
		}
		if (sprite != nullptr)
		{
			sprite->setScale(cell_width / 64., cell_width / 64.);
			centerOnPos(*sprite,
						screen_margin_x + board_offset_x + pos.x * cell_width,
						screen_margin_y + board_offset_y + pos.y * cell_height);
			this->draw(*sprite);
		}
		if (text != nullptr && options.showPriority)
		{
			text->setFillColor(sf::Color::White);
			sf::Vector2f textPos = sf::Vector2f(
					screen_margin_x + board_offset_x + pos.x * cell_width,
					screen_margin_y + board_offset_y + pos.y * cell_height);

			sf::Vector2f size = sf::Vector2f(40, 20);
			sf::RectangleShape shape = sf::RectangleShape(size);
			shape.setFillColor(sf::Color(50, 30, 10, 220));
			shape.setOutlineColor(sf::Color(50, 30, 10, 255));
			shape.setOutlineThickness(1);
			shape.setPosition(textPos.x - 20, textPos.y - 10);

			centerOnPos(*text, textPos.x, textPos.y);
			this->draw(shape);
			this->draw(*text);
		}
	}

	if (message.size())
	{
		sf::RectangleShape			wonPopup(sf::Vector2f(_w * 0.8,200));
		sf::Text					wonText(message, _textures.font, 50);

		wonPopup.setFillColor(sf::Color(50, 30, 10, 220));
		wonPopup.setOutlineColor(sf::Color(50, 30, 10, 255));
		wonPopup.setOutlineThickness(4);
		centerOnPos(wonPopup, _w/2, _h/2);
		centerOnPos(wonText, _w/2, _h/2);
		draw(wonPopup);
		draw(wonText);
	}

	sprite_black.setScale(cell_width / 64., cell_width / 64.);
	int count = b.getCapturedBlack();
	if (count > score_cell_count)
		count = score_cell_count;
	for (int i = 0; i < count; i++)
	{
		int posx = score_offset_x;
		int posy = (screen_height * (i + 0.5) / score_cell_count);
		centerOnPos(sprite_black, posx, posy);
		draw(sprite_black);
	}

	sprite_white.setScale(cell_width / 64., cell_width / 64.);
	count = b.getCapturedWhite();
	if (count > score_cell_count)
		count = score_cell_count;
	for (int i = 0; i < count; i++)
	{
		int posx = screen_width - score_offset_x;
		int posy = (screen_height * (i + 0.5) / score_cell_count);
		centerOnPos(sprite_white, posx, posy);
		draw(sprite_white);
	}

	display();
}

void 		GUIManager::drawMenu()
{
	sf::RectangleShape	onerect(sf::Vector2f(_w, _h / 4));
	sf::RectangleShape	tworect(sf::Vector2f(_w, _h / 4));
	sf::RectangleShape	threerect(sf::Vector2f(_w, _h / 4));
	sf::RectangleShape	fourrect(sf::Vector2f(_w, _h / 4));


	sf::Text			onetext("Human vs IA", _textures.font, 100);
	sf::Text			twotext("Human vs Human", _textures.font, 100);
	sf::Text			threetext("AI vs AI", _textures.font, 100);
	sf::Text			fourtext("Settings", _textures.font, 100);

	onerect.setFillColor(sf::Color(200, 100, 100));
	tworect.setFillColor(sf::Color(100, 200, 100));
	threerect.setFillColor(sf::Color(100, 100, 200));
	fourrect.setFillColor(sf::Color(100, 100, 100));
	onerect.setPosition(0, 0);
	tworect.setPosition(0, _h / 4);
	threerect.setPosition(0, (_h / 4) * 2);
	fourrect.setPosition(0, (_h / 4) * 3);

	centerOnPos(onetext, _w / 2, (_h / 4) / 2);
	centerOnPos(twotext, _w / 2, ((_h / 4) * 2) - (_h / 4) / 2);
	centerOnPos(threetext, _w / 2, ((_h / 4) * 3) - (_h / 4) / 2);
	centerOnPos(fourtext, _w / 2, ((_h / 4) * 4) - (_h / 4) / 2);

	this->draw(onerect);
	this->draw(tworect);
	this->draw(threerect);
	this->draw(fourrect);
	this->draw(onetext);
	this->draw(twotext);
	this->draw(threetext);
	this->draw(fourtext);
}

void 		GUIManager::drawOptions(std::vector<std::pair<std::string, bool>> options)
{
	sf::RectangleShape	rect(sf::Vector2f(_w, _h / options.size()));
	sf::Text			text;

	int i = 0;
	for (auto pair:options)
	{
		if (pair.second)
			rect.setFillColor(sf::Color(100, 200, 100));
		else
			rect.setFillColor(sf::Color(200, 100, 100));
		text = sf::Text(pair.first, _textures.font, 70);

		auto pos = sf::Vector2f(_w / 2., (float)_h / options.size() * (i + 0.5));
		centerOnPos(rect, pos.x, pos.y);
		centerOnPos(text, pos.x, pos.y);
		this->draw(rect);
		this->draw(text);
		i++;
	}
}
