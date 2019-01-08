#pragma once

#include <vector>
#include "Memory.h"
#include "DSStruct.h"


typedef struct _TEMP_STRUCT
{
	PBYTE ptrData;
	DWORD dwSize;
	DWORD dwFileID;

	_TEMP_STRUCT()
	{
		this->ptrData = 0;
		this->dwSize = 0;
		this->dwFileID = MAXDWORD;
	};
	~_TEMP_STRUCT()
	{
		delete[] ptrData;
	};
	bool init(DWORD dwFileID)
	{
		switch (dwFileID)
		{
			case FileID::player:
				this->dwSize = sizeof(PLAYER_STRUCT);
				break;
			case FileID::weapon:
				this->dwSize = sizeof(WEAPON_STRUCT);
				break;
			case FileID::effect:
				this->dwSize = sizeof(EFFECT_STRUCT);
				break;
			default:
				return false;
		}
		this->dwFileID = dwFileID;
		this->ptrData = new BYTE[dwSize];
		ZeroMemory(this->ptrData, dwSize);
		return true;
	};
} TEMP_STRUCT, *PTEMP_STRUCT;

class DSBase
{
protected:
	Memory* pProcMem;
public:
	virtual ~DSBase() {};
	virtual BOOL AddDataStruct(const TEMP_STRUCT &TempData) = 0;
	virtual void InvalidateAllData() = 0;
	virtual BOOL OverwriteMemory(BOOL bWithBackup) = 0;
};
