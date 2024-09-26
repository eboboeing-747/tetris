#pragma once

#include <iostream>
#include <windows.h>
#include <string>
#include <array>

// https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
#define VK_A 0x41
#define VK_D 0x44
#define VK_E 0x45
#define VK_Q 0x51
#define VK_S 0x53
#define VK_W 0x57

#define KEY_PRESSED 0x8000
#define KEY_TOGGLED 0x0001

static clock_t KEY_TIMEGAP = 150;
static const std::array<std::string, 254> keyNames = { "LMB", "RMB", "?", "MMB", "mouse 4", "mouse 5", "?", "backspace", "tab", "?", "?", "clear", "enter", "?", "?", "shift", "ctrl", "alt", "pause", "caps lock", "?", "?", "?", "?", "?", "?", "?", "?", "esc", "?", "?", "space", "page up", "page down", "end", "home", "left", "up", "right", "down", "?", "print", "enter", "PrtSc", "insert", "delete", "help", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "?", "?", "?", "?", "?", "?", "?", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "Win", "Context", "FN?", "?", "Sleep", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "Mult", "Add", "Sep", "Sub", "Dec", "Div", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "F13", "F14", "F15", "F16", "F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24", "?", "?", "?", "?", "?", "?", "?", "?", "Num Lock", "Scroll", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "L Shift", "R Shift", "L CTRL", "R CTRL", "L ALT", "R ALT", "Brow Back", "Brow Frwd", "Brow Refr", "Brow Stop", "Brow Srch", "Brow Fav", "Brow Home", "Brow Mute", "Brow Down", "Brow Up", "Med Next Track", "Med Prev Track", "Med Stop", "Med Play Pause", "?", "?", "?", "?", "?", "?", ";", "+", ",", "-", ".", "/", "~", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "[", "|", "]", "'", "?", "?", "?", "?"};
														//1     2     3     4      5          6		    7		8			9	 10	  11	12		 13		14	  15	16		17		18		19		20			21	 22	  23	24	25							30																				 40																			   50												 60												   70												 80												   90															  100														  110															120																	 130														  140															150													160																												170																																		   180												 190											   200												 210											   220
class Key
{
private:
	int virtualKey;
	clock_t coolDown;

public:
	Key()
	{
		this->virtualKey = 0;
		this->coolDown = 0;
	}

	Key(int virtualKey)
	{
		this->virtualKey = virtualKey;
		this->coolDown = clock();
	}

	bool isPressed()
	{
		if (!(GetKeyState(this->virtualKey) & KEY_PRESSED))
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

	void rebindVirtualKey()
	{
		Sleep(KEY_TIMEGAP);

		while (true)
		{
			for (int virtualKey = 1; virtualKey < 254; virtualKey++)
			{
				if (GetKeyState(virtualKey) & KEY_PRESSED)
				{
					this->virtualKey = virtualKey;
					Sleep(KEY_TIMEGAP);
					return;
				}
			}
		}
	}

	void setVirtualKey(const int& virtualKey)
	{
		this->virtualKey = virtualKey;
	}

	int getVirtualKey() const
	{
		return this->virtualKey;
	}

	std::string getVirtualKeyName() const
	{
		return keyNames.at(this->virtualKey - 1);
	}
};
