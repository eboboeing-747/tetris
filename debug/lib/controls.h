#include <iostream>
#include <windows.h>
#include <time.h>

// https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
#define VK_A 0x41
#define VK_D 0x44
#define VK_E 0x45
#define VK_Q 0x51
#define VK_S 0x53
#define VK_W 0x57

#define KEY_PRESSED 0x8000
#define KEY_TOGGLED 0x0001

clock_t KEY_TIMEGAP = 150;

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