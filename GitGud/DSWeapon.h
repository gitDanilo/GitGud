#pragma once

#include "DSBase.h"

class DSWeapon: public DSBase
{
private:
	std::vector<WEAPON_STRUCT> WeaponDataList;
	std::vector<WEAPON_STRUCT> WeaponDataBackupList;
	BOOL ReadDataStruct(WEAPON_STRUCT &WeaponData);
public:
	DSWeapon(Memory* pProcMem);
	~DSWeapon();
	virtual BOOL AddDataStruct(const TEMP_STRUCT &TempData);
	virtual void InvalidateAllData();
	virtual BOOL OverwriteMemory(BOOL bWithBackup);
};