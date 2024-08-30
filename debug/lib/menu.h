#include <iostream>
#include <string>
#include <vector>
#include "controls.h"
#include "utility.h"

static Key UP;
static Key DOWN;
static Key BACK;
static Key EXECUTE;
static Key D_BACK(VK_ESCAPE); // D stands for default
static Key D_EXECUTE(VK_RETURN); // D stands for default

class Option
{
private:
	std::string _name;
	void (*_command)();
	int _spaceFront;
	int _spaceBack;
	bool _isFocus;

public:
	Option(const std::string& _name, void (*_command)())
	{
		this->_name = _name;
		this->_command = _command;
		this->_isFocus = false;
		this->_spaceFront = 0;
		this->_spaceBack = 0;
	}

	void calcSpace(int width)
	{
		size_t size = this->_name.size();
		this->_spaceFront = (int)((width - size) / 2);

		if (size % 2 != 0)
		{
			this->_spaceBack = this->_spaceFront + 1;
		}
		else
		{
			this->_spaceBack = this->_spaceFront;
		}
	}
	
	void draw() const
	{
		if (this->_isFocus)
		{
			std::cout << '>';
		}
		else
		{
			std::cout << '|';
		}

		for (int i = 0; i < this->_spaceFront; i++)
		{
			std::cout << ' ';
		}
		std::cout << this->_name;
		for (int i = 0; i < this->_spaceBack; i++)
		{
			std::cout << ' ';
		}

		std::cout << "|\n";
	}

	size_t length() const
	{
		return this->_name.size();
	}

	bool isFocus() const
	{
		return this->_isFocus;
	}

	void setFocus(bool _isFocus)
	{
		this->_isFocus = _isFocus;
	}

	void execute()
	{
		this->_command();
	}
};

class Tab
{
private:
	std::string _name;
	std::vector<Option> _options;
	int _focus;
	int _x;
	int _y;
	int _width;
	int _spaceFront;
	const char deselected = '-';
	const char selected = '=';

	int fixFocus(int focus)
	{
		if (focus < 0)
		{
			return (int)this->_options.size() - 1;
		}
		else if (focus >= this->_options.size())
		{
			return 0;
		}

		return focus;
	}

	size_t width()
	{
		size_t maxWidth = 0;

		for (const Option& option : this->_options)
		{
			if (option.length() > maxWidth)
			{
				maxWidth = option.length();
			}
		}

		return maxWidth;
	}
	
	void renderLine(const size_t& currentIter)
	{
		// Option* current = &this->_options.at(currentIter);

		std::cout << '+';
		for (int i = 0; i < this->_width; i++)
		{
			std::cout << '-';
		}
		std::cout << "+\n";

		if (currentIter < this->_options.size())
		{
			this->_options.at(currentIter).draw();
		}
	}

public:
	Tab(const std::string& _name, std::initializer_list<Option> _options, const int& _x, const int& _y)
	{
		this->_name = _name;
		this->_options = _options;
		this->_focus = 0;
		this->_options.at(0).setFocus(true);
		this->_x = _x;
		this->_y = _y;
		this->_width = (int)this->width() + 2;

		if (this->_width % 2 != 0)
		{
			this->_width++;
		}

		this->_spaceFront = (this->_width - (int)this->_name.size() + 2) / 2;

		for (Option& option : this->_options)
		{
			option.calcSpace(this->_width);
		}
	}

	void listenInput()
	{
		bool isFocus = true;

		while (isFocus)
		{
			if (UP.isPressed())
			{
				this->_options.at(this->_focus).setFocus(false);
				this->_focus = this->fixFocus(this->_focus - 1);
				this->_options.at(this->_focus).setFocus(true);
			}
			else if (DOWN.isPressed())
			{
				this->_options.at(this->_focus).setFocus(false);
				this->_focus = this->fixFocus(this->_focus + 1);
				this->_options.at(this->_focus).setFocus(true);
			}
			else if (BACK.isPressed() || D_BACK.isPressed())
			{
				isFocus = false;
			}
			else if (EXECUTE.isPressed() || D_EXECUTE.isPressed())
			{
				this->_options.at(this->_focus).execute();
			}

			this->show();
		}

		this->hide();
	}

	void show()
	{
		gotoxy(this->_x, this->_y);

		for (int i = 0; i < this->_spaceFront; i++)
		{
			std::cout << ' ';
		}

		std::cout << this->_name << '\n';

		for (size_t i = 0; i < this->_options.size() + 1; i++)
		{
			this->renderLine(i);
		}
	}

	void hide()
	{
		for (size_t height = 0; height < this->_options.size() * 2; height++)
		{
			for (size_t width = 0; width < this->_width + 2; width++)
			{
				std::cout << ' ';
			}
			std::cout << '\n';
		}
	}
};
