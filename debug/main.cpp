
#include <array>
#include <vector>
#include <queue>
#include <cmath>
#include <iostream>
#include <Windows.h>
#include <random>
#include <fstream>
#include "lib/menu.h"
#include "lib/Cell.h"
#include "lib/utility.h"
#include "lib/controls.h"

class Point
{
private:
public:
	char x;
	char y;

	Point() { this->x = 0; this->y = 0; }

	Point(char x, char y)
	{
		this->x = x;
		this->y = y;
	}

	void add(char x, char y)
	{
		this->x += x;
		this->y += y;
	}
};

std::ostream& operator << (std::ostream& os, const Point& point)
{
	os << '{' << (int)point.x << ' ' << (int)point.y << '}';
	return os;
}

std::ostream& operator << (std::ostream& os, const std::array<Point, 4>& shape)
{
	for (std::array<Point, 4>::const_iterator i = shape.begin(); i != shape.end(); i++)
	{
		os << *i << '\t';
	}

	return os;
}

enum Direction : char
{
	LEFT	= 0,
	RIGHT	= 1,
	DOWN	= 2
};

enum LineState : char
{
	EMPTY		= 0,
	PARTIALLY	= 1,
	FILLED		= 2,
	ERR			= 3
};

const std::array<std::array<std::array<Point, 4>, 4>, 7> SHAPES = 
{ {
	{ {
		{ Point(0, 1), Point(1, 1), Point(2, 1), Point(3, 1) },
		{ Point(2, 0), Point(2, 1), Point(2, 2), Point(2, 3) },
		{ Point(0, 2), Point(1, 2), Point(2, 2), Point(3, 2) },
		{ Point(1, 0), Point(1, 1), Point(1, 2), Point(1, 3) }
	} },

	{ {
		{ Point(1, 1), Point(2, 1), Point(1, 2), Point(2, 2) }
	} },

	{ {
		{ Point(1, 0), Point(0, 1), Point(1, 1), Point(2, 1) },
		{ Point(1, 0), Point(1, 1), Point(2, 1), Point(1, 2) },
		{ Point(0, 1), Point(1, 1), Point(2, 1), Point(1, 2) },
		{ Point(1, 0), Point(0, 1), Point(1, 1), Point(1, 2) }
	} },

	{ {
		{ Point(0, 0), Point(1, 0), Point(1, 1), Point(2, 1) },
		{ Point(2, 0), Point(1, 1), Point(2, 1), Point(1, 2) },
		{ Point(0, 1), Point(1, 1), Point(1, 2), Point(2, 2) },
		{ Point(1, 0), Point(0, 1), Point(1, 1), Point(0, 2) }
	} },

	{ {
		{ Point(1, 0), Point(2, 0), Point(0, 1), Point(1, 1) },
		{ Point(1, 0), Point(1, 1), Point(2, 1), Point(2, 2) },
		{ Point(1, 1), Point(2, 1), Point(0, 2), Point(1, 2) },
		{ Point(0, 0), Point(0, 1), Point(1, 1), Point(1, 2) }
	} },

	{ {
		{ Point(0, 0), Point(0, 1), Point(1, 1), Point(2, 1) },
		{ Point(1, 0), Point(2, 0), Point(1, 1), Point(1, 2) },
		{ Point(0, 1), Point(1, 1), Point(2, 1), Point(2, 2) },
		{ Point(1, 0), Point(1, 1), Point(0, 2), Point(1, 2) }
	} },

	{ {
		{ Point(2, 0), Point(0, 1), Point(1, 1), Point(2, 1) },
		{ Point(1, 0), Point(1, 1), Point(1, 2), Point(2, 2) },
		{ Point(0, 1), Point(1, 1), Point(2, 1), Point(0, 2) },
		{ Point(0, 0), Point(1, 0), Point(1, 1), Point(1, 2) }
	} }
} };

class Tile
{
private:
	const std::array<std::array<Point, 4>, 4>* shapes;
	std::array<Point, 4> shape;
	short angle;
	Type _type;
	Color _color;
	char x;
	char y;

	short fixAngle(const short angle) const
	{
		short tempAngle = this->angle;
		tempAngle += angle;
		tempAngle %= angle;

		if (tempAngle < 0)
		{
			tempAngle += 360;
		}

		return tempAngle;
	}

	void movePoints(std::array<Point, 4>& shape) const
	{
		for (std::array<Point, 4>::iterator i = shape.begin(); i != shape.end(); i++)
		{
			i->add(this->x, this->y);
		}
	}

	void movePoints(std::array<Point, 4>& shape, char x, char y) const
	{
		for (std::array<Point, 4>::iterator i = shape.begin(); i != shape.end(); i++)
		{
			i->add(x, y);
		}
	}

	void movePoints(std::array<Point, 4>* shape, char x, char y) const
	{
		for (std::array<Point, 4>::iterator i = shape->begin(); i != shape->end(); i++)
		{
			i->add(x, y);
		}
	}

public:
	Tile() { this->shapes = nullptr; this->angle = 0; this->_type = Type::TILE_I; this->_color = Color::CYAN; this->x = 0; this->y = 0; }

	Tile(char x, char y, const Type& _type)
	{
		this->x		= x;
		this->y		= y;
		this->angle = 0;
		this->_type = _type;
		this->_color = explicitCast(_type);

		if (_type == Type::TILE_O) { this->shapes = nullptr; }
		else { this->shapes = &SHAPES[_type]; }
		this->shape = SHAPES[_type][0];
	}

	void rotate(short angle)
	{
		if (this->_type == TILE_O)
		{
			return;
		}

		this->angle += angle;
		this->angle %= 360;

		if (this->angle < 0)
		{
			this->angle += 360;
		}

		this->shape = this->shapes->at(abs(this->angle / 90));
	}

	void rotated(short angle, std::array<Point, 4>* position) const
	{
		*position = this->shape;

		if (this->_type != TILE_O)
		{
			short tempAngle = (this->angle + angle) % 360;
			
			if (tempAngle < 0)
			{
				tempAngle += 360;
			}

			*position = this->shapes->at(abs(tempAngle / 90));
		}
		else
		{
			*position = this->shape;
		}

		for (std::array<Point, 4>::iterator i = position->begin(); i != position->end(); i++)
		{
			i->add(this->x, this->y);
		}
	}

	void move(Direction direction)
	{
		switch (direction)
		{
		case LEFT:
			this->x--;
			break;
		case RIGHT:
			this->x++;
			break;
		case DOWN:
			this->y++;
			break;
		default:
			break;
		}
	}
	
	void moved(Direction direction, std::array<Point, 4>* position) const
	{
		*position = this->shape;

		switch (direction)
		{
		case LEFT:
			this->movePoints(position, this->x - 1, this->y);
			break;
		case RIGHT:
			this->movePoints(position, this->x + 1, this->y);
			break;
		case DOWN:
			this->movePoints(position, this->x, this->y + 1);
			break;
		default:
			break;
		}
	}

	const std::array<Point, 4> points() const
	{
		std::array<Point, 4> temp = this->shape;

		for (char i = 0; i < this->shape.size(); i++)
		{
			temp[i].add(this->x, this->y);
		}

		return temp;
	}

	Type type() const
	{
		return this->_type;
	}

	Color color() const
	{
		return this->_color;
	}

	void temp() const // debug prints to cout
	{
		for (std::array<Point, 4>::const_iterator i = this->shape.begin(); i != this->shape.end(); i++)
		{
			std::cout << *i << ' ';
		}
		std::cout << '\n';
	}
};

class Map
{
private:
	std::vector<std::vector<Cell>*>* grid;
	std::array<Point, 4> Points;
	std::array<Point, 4> validityCheck;
	Tile* tile;
	char _width;
	char _height;
	char _frameHeight;
	char _framePosition;
	const char _maxTileHeight = 2;
	bool isRunning;
	std::ofstream logFile; // debug

	char fixFramePosition(char newFramePosition) const
	{
		if (newFramePosition <= 0)
		{
			return 0;
		}
		else if (newFramePosition <= this->_frameHeight)
		{
			return newFramePosition;
		}
		else
		{
			return this->_frameHeight;
		}
	}

	void fixFramePosition()
	{
		for (size_t i = this->grid->size() - 1; i != 0; i--)
		{
			if (this->lineState(i) == LineState::EMPTY)
			{
				this->_framePosition = this->fixFramePosition((char)(i - this->_frameHeight / 2));
				return;
			}
		}

		this->_framePosition = 0;
	}

	Type randomTileType() const
	{
		std::uniform_int_distribution<short> dist(0, 6);
		std::random_device rd;
		return (Type)dist(rd);
	}

	bool isWithinRange(const Point& point) const // returns true if point is within borders, false otherwise
	{
		if ((point.x < 0) || (point.x >= this->_width)) { return false; }

		if ((point.y < 0) || (point.y >= this->_height)) { return false; }

		return true;
	}

	bool isOutOfRange(const Point& point) const // returns true if point is out of range, true otherwise
	{
		if ((point.x > 0) || (point.x <= this->_width)) { return true; }

		if ((point.y > 0) || (point.y <= this->_height)) { return true; }

		return false;
	}

	bool isValid(const std::array<Point, 4>& position) const
	{
		for (std::array<Point, 4>::const_iterator i = position.begin(); i != position.end(); i++)
		{
			if (!this->isWithinRange(*i)) { return false; } // if (x, y out of range) exit

			if (!this->grid->at(i->y)->at(i->x).isBlank) { return false; } // if (slot is busy) exit
		}

		return true;
	}

	bool isValid() const
	{
		for (std::array<Point, 4>::const_iterator i = this->validityCheck.begin(); i != this->validityCheck.end(); i++)
		{
			if (!this->isWithinRange(*i)) { return false; } // if (x, y out of range) exit

			if (!this->grid->at(i->y)->at(i->x).isBlank) { return false; } // if (slot is busy) exit
		}

		return true;
	}

	void copyLine(size_t source, size_t target)
	{
		std::vector<Cell>* sourceLine = this->grid->at(source);
		std::vector<Cell>* targetLine = this->grid->at(target);

		for (size_t i = 0; i < this->_width; i++)
		{
			targetLine->at(i) = sourceLine->at(i);
		}
	}
	
	LineState lineState(size_t i) const
	{
		std::vector<Cell>* line = this->grid->at(i);
		bool foundBlank = false;
		bool foundFilled = false;

		for (const Cell& cell : *line)
		{
			if (cell.isBlank)
			{
				foundBlank = true;
			}
			else
			{
				foundFilled = true;
			}
		}

		if (foundBlank && foundFilled)
		{
			return LineState::PARTIALLY;
		}
		else if (!foundBlank && foundFilled)
		{
			return LineState::FILLED;
		}
		else if (foundBlank && !foundFilled)
		{
			return LineState::EMPTY;
		}

		return LineState::ERR;
	}

	char compress()
	{
		std::queue<size_t> indexes;
		size_t begin = this->grid->size() - 1;
		size_t border = this->grid->size() - 1;

		for (size_t i = this->grid->size() - 1; i != 0; i--)
		{
			if (this->lineState(i) == LineState::PARTIALLY)
			{
				indexes.push(i);
				// this->logFile << "[" << clock() << "][log][PARTIALLY] { " << i << " }\n";
			}
			else if (this->lineState(i) == LineState::EMPTY)
			{
				// this->logFile << "[" << clock() << "][log][EMPTY] { " << i << " }\n";
				border = i;
				break;
			}
		}

		while (!indexes.empty())
		{
			if (begin != indexes.front())
			{
				// this->grid->at(begin) = this->grid->at(indexes.front());
				this->copyLine(indexes.front(), begin);
				// this->logFile << "[" << clock() << "][log] { " << indexes.front() << " -> " << begin << " }\n";
			}
			indexes.pop();
			begin--;
		}

		for (; begin >= border; begin--)
		{
			this->eraseLine(begin);
		}

		// this->logFile << "[" << clock() << "][log] { ";
		// 
		// for (size_t i = 0; i < this->grid->size(); i++)
		// {
		// 	this->logFile << (short)this->lineState(i) << ' ';
		// }
		// this->logFile << "}\n\n";
		// 
		// this->logFile << '\n';

		return (char)border;
	}

	void eraseLine(size_t i)
	{
		for (Cell& cell : *this->grid->at(i))
		{
			cell.isBlank = true;
		}
	}
	
	Tile* summonTile(const Type& tileType)
	{
		this->fixFramePosition();
		Tile* tile;

		if (this->_framePosition - this->_maxTileHeight >= 0)
		{
			tile = new Tile(this->_width / 2, this->_framePosition - this->_maxTileHeight, tileType);
			return tile;
		}

		tile = new Tile(this->_width / 2, 0, tileType);
		std::array<Point, 4> points = tile->points();

		if (this->isValid(points))
		{
			return tile;
		}
		
		return nullptr;
	}

	void erase()
	{
		for (char i = 0; i < this->Points.size(); i++)
		{
			this->grid->at(Points[i].y)->at(Points[i].x).isBlank = true;
		}
	}

	void tempGameOverMessage()
	{
		const std::string GAME_OVER_MESSAGE("GAME  OVER");
		gotoxy((this->_width * 2 + 2 - GAME_OVER_MESSAGE.size()) / 2, (this->_frameHeight * 0.8 + 2) / 2);
		std::cout << GAME_OVER_MESSAGE;
		this->isRunning = false;
	}

	void lock()
	{
		for (std::array<Point, 4>::iterator i = this->Points.begin(); i != this->Points.end(); i++)
		{
			this->grid->at(i->y)->at(i->x).isBlank = false;
		}

		delete this->tile;
		Type tileType = this->randomTileType();
		this->tile = this->summonTile(tileType);

		if (this->tile == nullptr)
		{
			this->tempGameOverMessage();
		}
		else
		{
			this->Points = this->tile->points();
		}
	}

	void updateOnLock()
	{
		this->lock();
		if (!this->isRunning)
		{
			return;
		}
		char newFramePosition = this->fixFramePosition(this->compress() - this->_frameHeight / 2);
		this->_framePosition = newFramePosition;
		this->logFile << '\n';
	}

public:
	Map(char _width, char _height, char _frameHeight)
	{
		this->_width			= _width;
		this->_height			= _height;
		this->_frameHeight		= _frameHeight;
		this->_framePosition	= _frameHeight;
		this->isRunning			= true;

		this->grid = new std::vector<std::vector<Cell>*>(_height);
		for (size_t i = 0; i < this->grid->size(); i++)
		{
			this->grid->at(i) = new std::vector<Cell>(_width);
		}

		Type tileType	= this->randomTileType();
		this->tile		= this->summonTile(tileType);
		this->Points	= this->tile->points();

		this->logFile.open("log.txt"); // debug
	}

	~Map()
	{
		for (size_t i = 0; i < this->grid->size(); i++)
		{
			delete this->grid->at(i);
		}

		delete this->grid;
		this->logFile.close(); // debug
	}

	std::ostream& renderLine(std::ostream& os, char current)
	{
		os << this->grid->at(current);

		return os;
	}

	void rotate(short angle)
	{
		if (!this->isRunning)
		{
			return;
		}

		if (this->tile->type() == Type::TILE_O)
		{
			return;
		}

		this->tile->rotated(angle, &(this->validityCheck));
		this->erase();

		if (this->isValid())
		{
			this->tile->rotate(angle);
		}
	}

	void move(Direction direction)
	{
		if (!this->isRunning)
		{
			return;
		}

		this->tile->moved(direction, &this->validityCheck);
		this->erase();

		if (this->isValid())
		{
			this->tile->move(direction);
		}
		else
		{
			if (direction == Direction::DOWN)
			{
				this->updateOnLock();
			}
		}
	}

	void update()
	{
		if (!this->isRunning)
		{
			return;
		}

		this->erase();
		this->Points = this->tile->points();

		for (char i = 0; i < this->Points.size(); i++)
		{
			// this->grid->at(Points[i].y)->at(Points[i].x).isBlank = false; // prev
			Cell* cell = &(this->grid->at(Points[i].y)->at(Points[i].x));
			cell->isBlank = false;
			cell->color = this->tile->color();
		}

		char tileFocus = this->fixFramePosition(this->Points[0].y - this->_frameHeight / 2);
		// this->logFile << "[log][tileFocus] { " << (int)tileFocus << " } [_framePosition] { " << (int)this->_framePosition << " }\n";
		if (tileFocus > this->_framePosition)
		{
			this->_framePosition = tileFocus;
		}
	}

	void temp()
	{
		setColor(Color::WHITE);
		std::cout << "+--------------------+\n";

		for (char i = this->_framePosition; i < this->_framePosition + this->_frameHeight; i++)
		{
			std::cout << '|' << this->grid->at(i);
			setColor(Color::WHITE);
			// std::cout<< "|\n";
			std::cout << "| " << this->_height - i << "   \n";
		}

		std::cout << "+--------------------+\n";
	}

	bool mainloop() const
	{
		return this->isRunning;
	}
};

Key MoveRight(VK_D);
Key MoveLeft(VK_A);
Key MoveDown(VK_S);
Key TurnClockwise(VK_E);
Key TurnCounterClockwise(VK_Q);
Key Interrupt(VK_ESCAPE);

static const int MAP_WIDTH = 10;
static const int MAP_HEIGHT = 40;
static const int FRAME_HEIGHT = 20;
static const std::string GAME_PAUSED_MESSAGE("GAME  PAUSED");

const double TPS = 2; // 4
const double FPS = 30;

void keybidns()
{
	gotoxy(0, 25);
	std::cout << "no keybinds tab so far";
}

Tab PauseMenu("GAME  PAUSED",
	{
		Option("back to game", []() { return; }, true ),
		Option("keybinds", keybidns, false),
		Option("quit", []() { exit(0); }, false )
	},
	3, 5); // MAP_WIDTH * 2 + 6, 0 

void pause()
{
	gotoxy((MAP_WIDTH * 2 + 2 - GAME_PAUSED_MESSAGE.size()) / 2, (FRAME_HEIGHT * 0.8 + 2) / 2);
	std::cout << GAME_PAUSED_MESSAGE;

	PauseMenu.listenInput();
}

void play()
{
	Map map(MAP_WIDTH, MAP_HEIGHT, FRAME_HEIGHT);
	double dropTick = clock();
	double renderTick = clock();

	gotoxy(0, 0);

	while (map.mainloop())
	{
		if (Focus::D_BACK.isPressed())
		{
			pause();
			gotoxy(0, 0);
		}

		if (MoveRight.isPressed())
		{
			map.move(Direction::RIGHT);
		}
		else if (MoveLeft.isPressed())
		{
			map.move(Direction::LEFT);
		}
		else if (MoveDown.isPressed())
		{
			map.move(Direction::DOWN);
		}
		else if (TurnClockwise.isPressed())
		{
			map.rotate(90);
		}
		else if (TurnCounterClockwise.isPressed())
		{
			map.rotate(-90);
		}

		if ((clock() - dropTick) / CLOCKS_PER_SEC >= 1.0 / TPS)
		{
			dropTick = clock();
			map.move(Direction::DOWN);
		}

		map.update();

		if ((clock() - renderTick) / CLOCKS_PER_SEC >= 1.0 / FPS)
		{
			renderTick = clock();
			map.temp();
			gotoxy(0, 0);
		}
	}
}

Tab MainMenu("MENU",
	{
		Option("play", play, false ),
		Option("keybinds", keybidns, false ),
		Option("quit", []() { exit(0); }, false )
	},
	5, 5);

int main()
{
	showConsoleCursor(false);

	Focus::UP.setVirtualKey(VK_W);
	Focus::DOWN.setVirtualKey(VK_S);
	Focus::BACK.setVirtualKey(VK_A);
	Focus::EXECUTE.setVirtualKey(VK_D);

	MainMenu.listenInput();
}
