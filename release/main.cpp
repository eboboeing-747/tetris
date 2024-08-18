#include <array>
#include <vector>
#include <queue>
#include <iostream>
#include <Windows.h>
#include <random>

// https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
#define VK_A 0x41
#define VK_D 0x44
#define VK_E 0x45
#define VK_Q 0x51
#define VK_S 0x53
#define VK_W 0x57

#define KEY_PRESSED 0x8000
#define KEY_TOGGLED 0x0001

const clock_t KEY_TIMEGAP = 150;
const HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

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

enum Direction : char
{
	LEFT = 0,
	RIGHT = 1,
	DOWN = 2
};

enum LineState : char
{
	EMPTY = 0,
	PARTIALLY = 1,
	FILLED = 2,
	ERR = 3
};

enum Color : char
{
	CYAN = 11,
	YELLOW = 14,
	PURPLE = 5,
	GREEN = 2,
	BLUE = 9,
	RED = 12,
	ORANGE = 6,
	WHITE = 7
};

void setColor(Color color)
{
	SetConsoleTextAttribute(hConsole, color);
}

enum Type : char
{
	TILE_I = 0,
	TILE_O = 1,
	TILE_T = 2,
	TILE_S = 3,
	TILE_Z = 4,
	TILE_J = 5,
	TILE_L = 6,
};

Color explicitCast(Type type)
{
	switch (type)
	{
	case TILE_I:
		return Color::CYAN;
		break;
	case TILE_O:
		return Color::YELLOW;
		break;
	case TILE_T:
		return Color::PURPLE;
		break;
	case TILE_S:
		return Color::GREEN;
		break;
	case TILE_Z:
		return Color::BLUE;
		break;
	case TILE_J:
		return Color::RED;
		break;
	case TILE_L:
		return Color::ORANGE;
		break;
	default:
		return Color::WHITE;
		break;
	}
}

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
	const std::array<std::array<Point, 4>, 4>* _shapes;
	std::array<Point, 4> _shape;
	short _angle;
	Type _type;
	Color _color;
	char x;
	char y;

	void movePoints(std::array<Point, 4>* shape, char x, char y) const
	{
		for (std::array<Point, 4>::iterator i = shape->begin(); i != shape->end(); i++)
		{
			i->add(x, y);
		}
	}

public:
	Tile()
	{
		this->_shapes = nullptr;
		this->_angle = 0;
		this->_type = Type::TILE_I;
		this->_color = Color::CYAN;
		this->x = 0;
		this->y = 0;
	}

	Tile(char x, char y, const Type& _type)
	{
		this->x = x;
		this->y = y;
		this->_angle = 0;
		this->_type = _type;
		this->_color = explicitCast(_type);

		if (_type == Type::TILE_O) { this->_shapes = nullptr; }
		else { this->_shapes = &SHAPES[_type]; }
		this->_shape = SHAPES[_type][0];
	}

	void rotate(short angle)
	{
		if (this->_type == TILE_O)
		{
			return;
		}

		this->_angle += angle;
		this->_angle %= 360;

		if (this->_angle < 0)
		{
			this->_angle += 360;
		}

		this->_shape = this->_shapes->at(abs(this->_angle / 90));
	}

	void rotated(short angle, std::array<Point, 4>* position) const
	{
		*position = this->_shape;

		if (this->_type != TILE_O)
		{
			short tempAngle = (this->_angle + angle) % 360;

			if (tempAngle < 0)
			{
				tempAngle += 360;
			}

			*position = this->_shapes->at(abs(tempAngle / 90));
		}
		else
		{
			*position = this->_shape;
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
		*position = this->_shape;

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
		std::array<Point, 4> temp = this->_shape;

		for (char i = 0; i < this->_shape.size(); i++)
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
};

struct Cell
{
public:
	bool isBlank;
	Color color;

	Cell()
	{
		this->isBlank = true;
		this->color = Color::WHITE;
	}

	Cell(bool isBlank, Color color)
	{
		this->isBlank = isBlank;
		this->color = color;
	}
};

std::ostream& operator << (std::ostream& os, const Cell& cell)
{
	switch (cell.isBlank)
	{
	case(true):
		return os << "  ";
		break;
	case(false):
		setColor(cell.color);
		return os << "[]";
		break;
	}

	return os << "??";
}

std::ostream& operator << (std::ostream& os, const std::vector<Cell>* cellLine)
{
	for (size_t i = 0; i < cellLine->size(); i++)
	{
		os << cellLine->at(i);
	}

	return os;
}

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

	Type randomTileType() const
	{
		std::uniform_int_distribution<short> dist(0, 6);
		std::random_device rd;
		return (Type)dist(rd);
	}

	void tempGameOverMessage() const
	{
		std::cout << "\n\n\n\n\n\n\n\n\n\n|     GAME  OVER";
	}

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

	bool isWithinRange(const Point& point) const // returns true if point is within borders, false otherwise
	{
		if ((point.x < 0) || (point.x >= this->_width)) { return false; }

		if ((point.y < 0) || (point.y >= this->_height)) { return false; }

		return true;
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

	void copyLine(size_t source, size_t target)
	{
		std::vector<Cell>* sourceLine = this->grid->at(source);
		std::vector<Cell>* targetLine = this->grid->at(target);

		for (size_t i = 0; i < this->_width; i++)
		{
			targetLine->at(i) = sourceLine->at(i);
		}
	}

	void eraseLine(size_t i)
	{
		for (Cell& cell : *this->grid->at(i))
		{
			cell.isBlank = true;
		}
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
			exit(0);
		}

		this->Points = this->tile->points();
	}

	void updateOnLock()
	{
		this->lock();
		char newFramePosition = this->fixFramePosition(this->compress() - this->_frameHeight / 2);
		this->_framePosition = newFramePosition;
	}

public:
	Map(char _width, char _height, char _frameHeight)
	{
		this->_width = _width;
		this->_height = _height;
		this->_frameHeight = _frameHeight;
		this->_framePosition = _frameHeight;

		this->grid = new std::vector<std::vector<Cell>*>(_height);
		for (size_t i = 0; i < this->grid->size(); i++)
		{
			this->grid->at(i) = new std::vector<Cell>(_width);
		}

		Type tileType = this->randomTileType();
		this->tile = this->summonTile(tileType);
		this->Points = this->tile->points();
	}

	~Map()
	{
		for (size_t i = 0; i < this->grid->size(); i++)
		{
			delete this->grid->at(i);
		}

		delete this->grid;
	}

	void rotate(short angle)
	{
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
		this->erase();
		this->Points = this->tile->points();

		for (char i = 0; i < this->Points.size(); i++)
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
			std::cout << "| " << this->_height - i << "   \n";
		}

		std::cout << "+--------------------+\n";
	}
};

class Key
{
private:
	int VirtualKey;
	clock_t coolDown;

public:
	Key(int VirtualKey)
	{
		this->VirtualKey = VirtualKey;
		this->coolDown = clock();
	}

	bool isPressed()
	{
		if (!(GetKeyState(this->VirtualKey) & KEY_PRESSED))
		{
			return false;
		}

		if (clock() - this->coolDown > KEY_TIMEGAP)
		{
			this->coolDown = clock();
			return true;
		}

		return false;
	}
};

Key MoveRight(VK_D);
Key MoveLeft(VK_A);
Key MoveDown(VK_S);
Key TurnClockwise(VK_E);
Key TurnCounterClockwise(VK_Q);
Key Interrupt(VK_ESCAPE);

void gotoxy(short x, short y)
{
	COORD pos = { x, y };
	HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(output, pos);
}

void pause()
{
	std::cout << "\n\n\n\n\n\n\n\n\n\n|    GAME  PAUSED";

	while (true)
	{
		if (Interrupt.isPressed())
		{
			gotoxy(0, 0);
			return;
		}
	}
}

void showConsoleCursor(bool showFlag)
{
	CONSOLE_CURSOR_INFO     cursorInfo;

	GetConsoleCursorInfo(hConsole, &cursorInfo);
	cursorInfo.bVisible = showFlag; // set the cursor visibility
	SetConsoleCursorInfo(hConsole, &cursorInfo);
}

int main()
{
	Map map(10, 40, 20);

	double dropTick = clock();
	double renderTick = clock();
	const double TPS = 2; // 4
	const double FPS = 30;
	showConsoleCursor(false);

	while (true)
	{
		if (Interrupt.isPressed())
		{
			pause();
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
