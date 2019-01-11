#pragma once

#include "Memory.h"

namespace DSHack
{
	bool ToggleGravity(BYTE* BaseAddr);
	bool ToggleAI(BYTE* BaseAddr);
	bool IncrementZPos(BYTE* BaseAddr, float fValue);
}
