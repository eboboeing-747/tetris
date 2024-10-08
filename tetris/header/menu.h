#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "controls.h"
#include "utility.h"

namespace Focus
{
	static Key UP;
	static Key DOWN;
	static Key BACK;
	static Key EXECUTE;
	static Key D_BACK(VK_ESCAPE); // D stands for default
	static Key D_EXECUTE(VK_RETURN); // D stands for default
}

class Tab;

class Option
{
private:
	std::string _name;
	Tab* _master;
	void (*_command)();
	int _spaceFront;
	int _spaceBack;
	bool _isFocus;
	bool _bindWithdraw;

public:
	Option(const std::string& _name, void (*_command)(), bool _bindWithdraw)
	{
		this->_name = _name;
		this->_command = _command;
		this->_isFocus = false;
		this->_spaceFront = 0;
		this->_spaceBack = 0;
		this->_bindWithdraw = _bindWithdraw;
		this->_master = nullptr;
	}

	void setMaster(Tab* _master)
	{
		this->_master = _master;
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

	void setName(const std::string& _name);

	void execute()
	{
		this->_command();
	}

	bool isBound() const
	{
		return this->_bindWithdraw;
	}
};

class Tab
{
private:
	std::string _name;
	std::vector<Option*> _options;
	void (*_onHide)();
	int _focus;
	int _x;
	int _y;
	int _width;
	int _height;
	int _spaceFront;
	static const char deselected = '-';
	static const char selected = '=';

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

	size_t calcWidth()
	{
		size_t maxWidth = 0;

		for (const Option* option : this->_options)
		{
			if (option->length() > maxWidth)
			{
				maxWidth = option->length();
			}
		}

		return maxWidth;
	}

	int calcHeight()
	{
		return this->_options.size() * 2 + 3;
	}

	void renderLine(const size_t& currentIter)
	{
		gotoxy(this->_x, currentIter * 2 + 1 + this->_y);

		std::cout << '+';
		for (int i = 0; i < this->_width; i++)
		{
			std::cout << '-';
		}
		std::cout << '+';

		if (currentIter < this->_options.size())
		{
			gotoxy(this->_x, currentIter * 2 + 2 + this->_y);
			this->_options.at(currentIter)->draw();
		}
	}

public:
	Tab(const std::string& _name, std::initializer_list<Option*> _options, const int& _x, const int& _y)
	{
		this->_name = _name;
		this->_options = _options;
		this->_focus = 0;
		this->_options.at(0)->setFocus(true);
		this->_x = _x;
		this->_y = _y;
		this->fixWidth();
		this->_onHide = nullptr;

		for (size_t i = 0; i < this->_options.size(); i++)
		{
			this->_options.at(i)->setMaster(this);
		}
	}

	Tab(const std::string& _name, std::initializer_list<Option*> _options, void (*_onHide)(), const int& _x, const int& _y)
	{
		this->_name = _name;
		this->_options = _options;
		this->_focus = 0;
		this->_options.at(0)->setFocus(true);
		this->_x = _x;
		this->_y = _y;
		this->fixWidth();
		this->_onHide = _onHide;

		for (size_t i = 0; i < this->_options.size(); i++)
		{
			this->_options.at(i)->setMaster(this);
		}
	}

	~Tab()
	{
		for (Option* option : this->_options)
		{
			delete option;
		}
	}

	void fixWidth()
	{
		this->_width = (int)this->calcWidth() + 2;
		this->_height = this->calcHeight();

		if (this->_width % 2 != 0)
		{
			this->_width++;
		}

		this->_spaceFront = (this->_width - (int)this->_name.size() + 2) / 2;

		for (Option* option : this->_options)
		{
			option->calcSpace(this->_width);
		}
	}

	void listenInput()
	{
		bool isFocus = true;

		this->show();

		while (isFocus)
		{
			if (Focus::UP.isPressed())
			{
				this->_options.at(this->_focus)->setFocus(false);
				this->_focus = this->fixFocus(this->_focus - 1);
				this->_options.at(this->_focus)->setFocus(true);
			}
			else if (Focus::DOWN.isPressed())
			{
				this->_options.at(this->_focus)->setFocus(false);
				this->_focus = this->fixFocus(this->_focus + 1);
				this->_options.at(this->_focus)->setFocus(true);
			}
			else if (Focus::BACK.isPressed() || Focus::D_BACK.isPressed())
			{
				isFocus = false;
			}
			else if (Focus::EXECUTE.isPressed() || Focus::D_EXECUTE.isPressed())
			{
				if (this->_options.at(this->_focus)->isBound())
				{
					isFocus = false;
				}

				this->_options.at(this->_focus)->execute();
			}
			else
			{
				continue;
			}

			this->show();
		}

		this->hide();
	}

	void show()
	{
		gotoxy(this->_x + this->_spaceFront, this->_y);

		std::cout << this->_name << '\n';

		for (size_t i = 0; i < this->_options.size() + 1; i++)
		{
			this->renderLine(i);
		}
	}

	Option* at(const size_t& iteration)
	{
		return this->_options.at(iteration);
	}

	size_t current() const
	{
		return this->_focus;
	}

	void setName(const std::string& name)
	{
		this->_options.at(this->_focus)->setName(name);
	}

	void hide()
	{
		if (this->_onHide != nullptr)
		{
			this->_onHide();
		}

		for (size_t height = 0; height < this->_options.size() * 2 + 2; height++)
		{
			gotoxy(this->_x, height);
			for (int width = 0; width < this->_width + 2; width++)
			{
				std::cout << ' ';
			}
			std::cout << '\n';
		}
	}

	int width() const
	{
		return this->_width;
	}

	int height() const
	{
		return this->_height;
	}
};

void Option::setName(const std::string& _name)
{
	this->_name = _name;
	this->_master->hide();
	this->_master->fixWidth();
	this->_master->show();
}
