#pragma once

#include "DSBase.h"

class DSEffect: public DSBase
{
private:
	std::vector<EFFECT_STRUCT> EffectDataList;
	std::vector<EFFECT_STRUCT> EffectDataBackupList;
	BOOL ReadDataStruct(EFFECT_STRUCT &EffectData);
public:
	DSEffect(Memory* pProcMem);
	~DSEffect();
	virtual BOOL AddDataStruct(const TEMP_STRUCT &TempData);
	virtual void InvalidateAllData();
	virtual BOOL OverwriteMemory(BOOL bWithBackup);
};
