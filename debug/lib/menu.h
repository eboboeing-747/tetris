#include <iostream>
#include <string>
#include <vector>
#include "controls.h"
#include "utility.h"

Key UP;
Key DOWN;
Key BACK;
Key EXECUTE;
const Key D_BACK(VK_ESCAPE); // D stands for default
const Key D_EXECUTE(VK_RETURN); // D stands for default

class Option
{
private:
	std::string _name;
	void (*_command)();
	int _spaceFront;
	int _spaceBack;
	bool isFocus;

public:
	Option(std::string& _name, void (*_command)())
	{
		this->_name = _name;
		this->_command = _command;
		this->isFocus = false;
		this->_spaceFront = 0;
		this->_spaceBack = 0;
	}

	void calcSpace(int width)
	{
		size_t size = this->_name.size();
		this->_spaceFront = size / 2;

		if (size % 2 != 0)
		{
			this->_spaceBack = this->_spaceFront + 1;
		}
		else
		{
			this->_spaceBack = this->_spaceFront;
		}
	}

	size_t length() const
	{
		return this->_name.size();
	}
};

class Tab
{
private:
	std::string _name;
	std::vector<Option> _options;
	int _x;
	int _y;
	int _width;

	size_t width()
	{
		size_t maxWidth = 0;

		for (const Option& option : this->_options)
		{
			if (option.length() < maxWidth)
			{
				maxWidth = option.length();
			}
		}

		return maxWidth;
	}

public:
	Tab(std::string& _name, std::initializer_list<Option> _options, int& _x, int& _y)
	{
		this->_name = _name;
		this->_options = _options;
		this->_x = _x;
		this->_y = _y;
		this->_width = this->width();

		for (Option& option : this->_options)
		{
			option.calcSpace(this->_width);
		}
	}
};
