#pragma once

#include <iostream>
#include <vector>
#include <Windows.h>

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

void setColor(Color color)
{
	SetConsoleTextAttribute(hConsole, color);
}

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
