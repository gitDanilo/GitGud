#pragma once

#include <Windows.h>
#include <iostream>

class Color
{
private:
	HANDLE m_hConsole;
	WORD m_wAttr;
public:
	explicit Color(HANDLE hConsole, WORD wAttr): m_hConsole(hConsole), m_wAttr(wAttr)
	{
	}
	friend std::ostream& operator<<(std::ostream &o, const Color &c)
	{
		SetConsoleTextAttribute(c.m_hConsole, c.m_wAttr);
		return o;
	}
};

namespace clr
{
	static HANDLE hConsole;
#define DEF_COLOR_MANIPULATOR(color_name, attr)				\
		inline std::ostream& color_name(std::ostream &out)	\
		{													\
			SetConsoleTextAttribute(hConsole, attr);		\
			return out;										\
		}													\

	DEF_COLOR_MANIPULATOR(blue  , 0x9);
	DEF_COLOR_MANIPULATOR(green , 0xA);
	DEF_COLOR_MANIPULATOR(cyan  , 0xB);
	DEF_COLOR_MANIPULATOR(red   , 0xC);
	DEF_COLOR_MANIPULATOR(pink  , 0xD);
	DEF_COLOR_MANIPULATOR(yellow, 0xE);
	DEF_COLOR_MANIPULATOR(white , 0xF);
};
