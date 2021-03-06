#include <iostream>
#include <unistd.h>
#include "GUIManager.hpp"
#include "Game.hpp"
#include "GUI.hpp"

using namespace std;

std::string getVictoryMessage(VictoryState v)
{
	std::string text;
	std::string player = (v.victor == blackPlayer) ? "Black" : "White";
	player += " player wins:\n";

	switch (v.type)
	{
		case aligned:
			text = player + "  Five stones aligned";
			break;
		case captured:
			text = player + "  Enough stones captured";
			break;
		case staleMate:
			text = "Stalemate";
		default:
			break;
	}
	return (text);
}

void game_page(GUIManager& win, Options &options)
{
	Game                g(options);
	bool                hasWon = false;
	bool 				turn_incr = true;
	std::string         text("");
	VictoryState        victory;
	BoardPos            pos;
	turn = 0;
	

	win.drawBoard(g, options, text);
	while (1)
	{
		bool shouldWait = true;
		win.clear();
		sf::Event   event;

		while (win.pollEvent(event))
		{	
			switch (event.type)
			{
				case sf::Event::Closed:
					exit(0);
				case sf::Event::KeyPressed:
					if (event.key.code == sf::Keyboard::Escape)
					{
						turn_incr = true;
						turn = 0;
						return;
					}
					break ;
				case sf::Event::MouseButtonPressed:
					if (hasWon)
						return;
					if (g.isPlayerNext() && win.getMouseBoardPos(pos) && !hasWon)
					{
						std::cout << "Player " << (g.getTurn() == whitePlayer ? "white:" : "black:") << std::endl;
						if (g.play(pos))
						{
							hasWon = true;
							victory = g.getState()->getVictory();
							text = getVictoryMessage(victory);
						}
						std::cout << "  Board score: " << g.getCurrentScore() << std::endl;
						win.drawBoard(g, options, text);
					}
					break ;
				default:
					break;
			}
		}

		if (g.isPlayerNext())
			win.drawBoard(g, options, text);

		if (g.getTurn() == PlayerColor::whitePlayer && options.isWhiteAI && !hasWon)
		{	
			if (g.play())
			{
				hasWon = true;
				victory = g.getState()->getVictory();
				text = getVictoryMessage(victory);
			}
			turn_incr = true;
			std::cout << "AI white:" << std::endl;
			std::cout << "  Board score: " << g.getCurrentScore() << std::endl;
			std::cout << "  Time taken: " << g.getTimeTaken() << std::endl;
			if (text != "") std::cout << text << std::endl;
			win.drawBoard(g, options, text);
			shouldWait = false;
		}

		if (g.getTurn() == PlayerColor::blackPlayer && options.isBlackAI && !hasWon)
		{
			if (g.play())
			{
				hasWon = true;
				victory = g.getState()->getVictory();
				text = getVictoryMessage(victory);
			}
			std::cout << "---------------------------" << std::endl;
			std::cout << "TURN " << ++turn << "!" << std::endl;
			std::cout << "AI black:" << std::endl;
			std::cout << "  Board score: " << g.getCurrentScore() << std::endl;
			std::cout << "  Time taken: " << g.getTimeTaken() << std::endl;
			if (text != "") std::cout << text << std::endl;
			win.drawBoard(g, options, text);
			shouldWait = false;
		}

		if (g.getTurn() == PlayerColor::blackPlayer && turn_incr)
		{
			turn_incr = false;
			std::cout << "---------------------------" << std::endl;
			std::cout << "TURN " << ++turn << "!" << std::endl;
		}

		if (g.getTurn() == PlayerColor::whitePlayer)
			turn_incr = true;

		if (shouldWait)
			usleep(200);
	}
}

GUIManager::MenuButton menu_page(GUIManager& win)
{
	while (1)
	{
		win.clear();
		sf::Event   event;
		while (win.pollEvent(event))
		{
			switch (event.type)
			{
				case sf::Event::Closed:
					exit(0);
				case sf::Event::KeyPressed:
					if (event.key.code == sf::Keyboard::Escape)
						exit(0);
					break ;
				case sf::Event::MouseButtonPressed:
					return win.getMenuButton();
				default:
					break;
			}
		}
		win.drawMenu();
		win.display();
		usleep(200);
	}
}

std::vector<std::pair<std::string, bool>> getOptionsData(Options &options)
{
	return std::vector<std::pair<std::string, bool>>({
							   std::pair<std::string, bool>("Show player suggestions", options.showTips),
							   std::pair<std::string, bool>("Show priority", options.showPriority),
							   std::pair<std::string, bool>("Increase deep search", options.slowMode),
							   std::pair<std::string, bool>("Block double free-threes", options.doubleThree),
							   std::pair<std::string, bool>("Allow captures", options.capture),
							   std::pair<std::string, bool>("Allow win by capture", options.captureWin),
					   });
}

void option_page(GUIManager& win, Options &options)
{
	int     pos;
	auto    optionsVector = getOptionsData(options);

	while (1)
	{
		win.clear();
		sf::Event   event;
		while (win.pollEvent(event))
		{
			switch (event.type)
			{
				case sf::Event::Closed:
					exit(0);
				case sf::Event::KeyPressed:
					if (event.key.code == sf::Keyboard::Escape)
						return ;
					break ;
				case sf::Event::MouseButtonPressed:
					pos = win.getMouseScreenRatio().y * optionsVector.size();

					switch (pos)
					{
						case 0: options.showTips = !options.showTips; break;
						case 1: options.showPriority = !options.showPriority; break;
						case 2: options.slowMode = !options.slowMode; break;
						case 3: options.doubleThree = !options.doubleThree; break;
						case 4: options.capture = !options.capture; break;
						case 5: options.captureWin = !options.captureWin; break;
					}
					break;
				default:
					break;
			}
		}
		optionsVector = getOptionsData(options);
		win.drawOptions(optionsVector);
		win.display();
		usleep(200);
	}
}

void GUI::start_loop()
{
	Options       options;
	GUIManager          win;

	srand(time(NULL));
	while (1)
	{
		switch (menu_page(win))
		{
			case GUIManager::ButtonAIVersusAI:
				options.isBlackAI = true;
				options.isWhiteAI = true;
				game_page(win, options);
				break;
			case GUIManager::ButtonPlayerVersusAI:
				options.isBlackAI = false;
				options.isWhiteAI = true;
				game_page(win, options);
				break;
			case GUIManager::ButtonPlayerVersusPlayer:
				options.isBlackAI = false;
				options.isWhiteAI = false;
				game_page(win, options);
				break;
			case GUIManager::ButtonOptions:
				option_page(win, options);
				break;
		}
	}
}

GUI::GUI(){};
