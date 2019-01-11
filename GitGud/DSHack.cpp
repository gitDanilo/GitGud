#include "DSHack.h"

bool DSHack::ToggleGravity(BYTE* BaseAddr)
{
	BYTE byte;
	Memory* pProcMem = Memory::GetInstance();

	if (pProcMem->ReadProcMem(BaseAddr, BaseAddr) == false)
		return false;

	BaseAddr += 0x80;

	if (pProcMem->ReadProcMem(BaseAddr, BaseAddr) == false)
		return false;

	BaseAddr += 0x1A08;

	if (pProcMem->ReadProcMem(BaseAddr, byte) == false)
		return false;

	if (GETBIT(byte, 6))
		CLBIT(byte, 6);
	else
		SETBIT(byte, 6);

	if (pProcMem->WriteProcMem(BaseAddr, byte) == false)
		return false;

	return true;
}

bool DSHack::ToggleAI(BYTE* BaseAddr)
{
	BYTE byte;
	Memory* pProcMem = Memory::GetInstance();

	BaseAddr = BaseAddr + 0x9 + 0x4;

	if (pProcMem->ReadProcMem(BaseAddr, byte) == false)
		return false;

	byte = (byte ? 0 : 1);

	if (pProcMem->WriteProcMem(BaseAddr, byte) == false)
		return false;

	return true;
}

bool DSHack::IncrementZPos(BYTE* BaseAddr, float fValue)
{
	float fBaseValue = 0;
	Memory* pProcMem = Memory::GetInstance();

	if (pProcMem->ReadProcMem(BaseAddr, BaseAddr) == false)
		return false;

	BaseAddr += 0x40;

	if (pProcMem->ReadProcMem(BaseAddr, BaseAddr) == false)
		return false;

	BaseAddr += 0x28;

	if (pProcMem->ReadProcMem(BaseAddr, BaseAddr) == false)
		return false;

	BaseAddr += 0x84;

	if (pProcMem->ReadProcMem(BaseAddr, fBaseValue) == false)
		return false;

	fBaseValue += fValue;

	if (pProcMem->WriteProcMem(BaseAddr, fBaseValue) == false)
		return false;

	return true;
}
