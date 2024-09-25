#include <array>
#include <vector>
#include <queue>
#include <cmath>
#include <iostream>
#include <Windows.h>
#include <random>
#include <fstream>
#include <string>
#include "../header/menu.h"
#include "../header/Cell.h"
#include "../header/utility.h"
#include "../header/controls.h"

class Point
{
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
	{ { // I
		{ Point(0, 1), Point(1, 1), Point(2, 1), Point(3, 1) },
		{ Point(2, 0), Point(2, 1), Point(2, 2), Point(2, 3) },
		{ Point(0, 2), Point(1, 2), Point(2, 2), Point(3, 2) },
		{ Point(1, 0), Point(1, 1), Point(1, 2), Point(1, 3) }
	} },

	{ { // O
		{Point(0, 0), Point(1, 0), Point(0, 1), Point(1, 1)}
		// { Point(1, 1), Point(2, 1), Point(1, 2), Point(2, 2) }
	} },

	{ { // T
		{ Point(1, 0), Point(0, 1), Point(1, 1), Point(2, 1) },
		{ Point(1, 0), Point(1, 1), Point(2, 1), Point(1, 2) },
		{ Point(0, 1), Point(1, 1), Point(2, 1), Point(1, 2) },
		{ Point(1, 0), Point(0, 1), Point(1, 1), Point(1, 2) }
	} },

	{ { // S
		{ Point(0, 0), Point(1, 0), Point(1, 1), Point(2, 1) },
		{ Point(2, 0), Point(1, 1), Point(2, 1), Point(1, 2) },
		{ Point(0, 1), Point(1, 1), Point(1, 2), Point(2, 2) },
		{ Point(1, 0), Point(0, 1), Point(1, 1), Point(0, 2) }
	} },

	{ { // Z
		{ Point(1, 0), Point(2, 0), Point(0, 1), Point(1, 1) },
		{ Point(1, 0), Point(1, 1), Point(2, 1), Point(2, 2) },
		{ Point(1, 1), Point(2, 1), Point(0, 2), Point(1, 2) },
		{ Point(0, 0), Point(0, 1), Point(1, 1), Point(1, 2) }
	} },

	{ { // J
		{ Point(0, 0), Point(0, 1), Point(1, 1), Point(2, 1) },
		{ Point(1, 0), Point(2, 0), Point(1, 1), Point(1, 2) },
		{ Point(0, 1), Point(1, 1), Point(2, 1), Point(2, 2) },
		{ Point(1, 0), Point(1, 1), Point(0, 2), Point(1, 2) }
	} },

	{ { // L
		{ Point(2, 0), Point(0, 1), Point(1, 1), Point(2, 1) },
		{ Point(1, 0), Point(1, 1), Point(1, 2), Point(2, 2) },
		{ Point(0, 1), Point(1, 1), Point(2, 1), Point(0, 2) },
		{ Point(0, 0), Point(1, 0), Point(1, 1), Point(1, 2) }
	} }
} };

const std::array<Point, 7> DIMENSIONS = { // Point.x & Point.y is width & height of corresponding Tile
	Point(4, 1), // I
	Point(2, 2), // O
	Point(3, 2), // T
	Point(3, 2), // S
	Point(3, 2), // Z
	Point(3, 2), // J
	Point(3, 2)  // L
};

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

		for (size_t i = 0; i < this->shape.size(); i++)
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

class Viewport
{
private:
	std::vector<std::vector<Cell>*>* _grid;
	int _x;
	int _y;
	char _width;
	char _height;

	void size(const Type& tiletype)
	{
		this->_width = DIMENSIONS.at(tiletype).x + 2;
		this->_height = DIMENSIONS.at(tiletype).y + 2;
	}

	void gridDelete() const
	{
		for (size_t i = 0; i < this->_grid->size(); i++)
		{
			delete this->_grid->at(i);
		}

		delete this->_grid;
	}

	void gridInit(const Type& tiletype)
	{
		if (this->_grid != nullptr)
		{
			this->gridDelete();
		}

		this->size(tiletype);

		this->_grid = new std::vector<std::vector<Cell>*>(this->_height);
		for (size_t i = 0; i < this->_grid->size(); i++)
		{
			this->_grid->at(i) = new std::vector<Cell>(this->_width);
		}
	}

	void draw() const
	{
		this->drawBorder(this->_x, this->_y);
		for (size_t i = 0; i < this->_grid->size(); i++)
		{
			gotoxy(this->_x, this->_y + i + 1);
			std::cout << '|' << this->_grid->at(i);
			setColor(Color::WHITE);
			std::cout << '|';
		}
		this->drawBorder(this->_x, this->_y + this->_height + 1);

		gotoxy(0, 0);
	}

	void hide() const
	{
		for (int i = 0; i < this->_height + 2; i++)
		{
			gotoxy(this->_x, this->_y + i);

			for (int line = 0; line < this->_width * 2 + 2; line++)
			{
				std::cout << ' ';
			}
		}

		gotoxy(0, 0);
	}

	void drawBorder(int x, int y) const
	{
		gotoxy(x, y);
		std::cout << '+';
		for (int i = 0; i < this->_width * 2; i++)
		{
			std::cout << '-';
		}
		std::cout << '+';
	}

public:
	Viewport(int _x, int _y)
	{
		this->_grid = nullptr;
		this->_x = _x;
		this->_y = _y;
		this->_width = 0;
		this->_height = 0;
	}

	~Viewport()
	{
		this->gridDelete();
	}

	void update(const Type& tiletype)
	{
		this->hide();
		this->gridInit(tiletype);

		const std::array<Point, 4>& tileshape = SHAPES.at(tiletype).at(0);
		int offset_y = 1;

		for (size_t i = 0; i < tileshape.size(); i++)
		{
			if (tiletype != Type::TILE_I)
			{
				offset_y = 1;
			}
			else
			{
				offset_y = 0;
			}

			Cell& cell = this->_grid->at(tileshape.at(i).y + offset_y)->at(tileshape.at(i).x + 1);

			cell.isBlank = false;
			cell.color = explicitCast(tiletype);
		}

		this->draw();
	}
};

class Map
{
private:
	Viewport* tab;
	std::vector<std::vector<Cell>*>* grid;
	std::array<Point, 4> Points;
	std::array<Point, 4> validityCheck;
	Tile* tile;
	Type nextTiletype;
	char _width;
	char _height;
	char _frameHeight;
	char _framePosition;
	const char _maxTileHeight = 2;
	bool isRunning;

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

	Type randomTileType()
	{
		std::uniform_int_distribution<short> dist(0, 6);
		std::random_device rd;
		Type currentTiletype = this->nextTiletype;
		this->nextTiletype = (Type)dist(rd);
		this->tab->update(nextTiletype);
		return currentTiletype;
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

		for (char i = 0; i < this->_width; i++)
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
			}
			else if (this->lineState(i) == LineState::EMPTY)
			{
				border = i;
				break;
			}
		}

		while (!indexes.empty())
		{
			if (begin != indexes.front())
			{
				this->copyLine(indexes.front(), begin);
			}
			indexes.pop();
			begin--;
		}

		for (; begin >= border; begin--)
		{
			this->eraseLine(begin);
		}

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
		for (size_t i = 0; i < this->Points.size(); i++)
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
			this->isRunning = false;
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

		this->tab		= new Viewport(this->_width * 2 + 6, 0);
		Type tileType	= this->randomTileType();
		this->tile		= this->summonTile(tileType);
		this->Points	= this->tile->points();
		this->tab->update(tileType);
	}

	~Map()
	{
		for (size_t i = 0; i < this->grid->size(); i++)
		{
			delete this->grid->at(i);
		}

		delete this->grid;
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

		for (size_t i = 0; i < this->Points.size(); i++)
		{
			Cell* cell = &(this->grid->at(Points[i].y)->at(Points[i].x));
			cell->isBlank = false;
			cell->color = this->tile->color();
		}

		char tileFocus = this->fixFramePosition(this->Points[0].y - this->_frameHeight / 2);
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
			std::cout << "| " << this->_height - i << " \n";
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

Tab* KeyBindsDisplay;
Tab* KeyBinds;

static void rebindMoveRight()
{
	MoveRight.rebindVirtualKey();
	const size_t position = KeyBinds->current();
	const std::string name = MoveRight.getVirtualKeyName();
	KeyBindsDisplay->at(position)->setName(name);
}

static void rebindMoveLeft()
{
	MoveLeft.rebindVirtualKey();
	const size_t position = KeyBinds->current();
	const std::string name = MoveLeft.getVirtualKeyName();
	KeyBindsDisplay->at(position)->setName(name);
}

static void rebindMoveDown()
{
	MoveDown.rebindVirtualKey();
	const size_t position = KeyBinds->current();
	const std::string name = MoveDown.getVirtualKeyName();
	KeyBindsDisplay->at(position)->setName(name);
}

static void rebindTurnClockwise()
{
	TurnClockwise.rebindVirtualKey();
	const size_t position = KeyBinds->current();
	const std::string name = TurnClockwise.getVirtualKeyName();
	KeyBindsDisplay->at(position)->setName(name);
}

static void rebindTurnCounterClockwise()
{
	TurnCounterClockwise.rebindVirtualKey();
	const size_t position = KeyBinds->current();
	const std::string name = TurnCounterClockwise.getVirtualKeyName();
	KeyBindsDisplay->at(position)->setName(name);
}

static const int MAP_WIDTH = 10;
static const int MAP_HEIGHT = 40;
static const int FRAME_HEIGHT = 20;
static const std::string GAME_PAUSED_MESSAGE("GAME  PAUSED");

const double TPS = 2; // 4
const double FPS = 30;

static void keybidns()
{
	KeyBindsDisplay->show();
	KeyBinds->listenInput();
}

static void onExit(const int& exitCode)
{
	delete KeyBindsDisplay;
	delete KeyBinds;

	exit(exitCode);
}

Tab PauseMenu("GAME  PAUSED",
	{
		new Option("back to game", []() { return; }, true ),
		new Option("keybinds", keybidns, false ),
		new Option("quit", []() { onExit(0); }, false )
	},
	3, 5); // MAP_WIDTH * 2 + 6, 0 

static void pause()
{
	showConsoleCursor(false);
	gotoxy((MAP_WIDTH * 2 + 2 - GAME_PAUSED_MESSAGE.size()) / 2, (FRAME_HEIGHT * 0.8 + 2) / 2);
	std::cout << GAME_PAUSED_MESSAGE;

	PauseMenu.listenInput();
}

static void play()
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
		new Option("play", play, false ),
		new Option("keybinds", keybidns, false ),
		new Option("quit", []() { onExit(0); }, false )
	},
	5, 5);

int main()
{
	KeyBindsDisplay = new Tab("",
	{
		new Option("", []() { return; }, true),
		new Option("d", []() { return; }, false),
		new Option("a", []() { return; }, false),
		new Option("s", []() { return; }, false),
		new Option("q", []() { return; }, false),
		new Option("e", []() { return; }, false),
	},
	57, 0);

	KeyBinds = new Tab("keybinds",
	{
		new Option("back", []() { KeyBindsDisplay->hide(); }, true),
		new Option("move right", rebindMoveRight, false),
		new Option("move left", rebindMoveLeft, false),
		new Option("move down", rebindMoveDown, false),
		new Option("rotate left", rebindTurnClockwise, false),
		new Option("rotate right", rebindTurnCounterClockwise, false)
	},
	[]() { KeyBindsDisplay->hide(); }, 42, 0);

	showConsoleCursor(false);
	
	Focus::UP.setVirtualKey(VK_W);
	Focus::DOWN.setVirtualKey(VK_S);
	Focus::BACK.setVirtualKey(VK_A);
	Focus::EXECUTE.setVirtualKey(VK_D);
	
	MainMenu.listenInput();

	onExit(0);
}
