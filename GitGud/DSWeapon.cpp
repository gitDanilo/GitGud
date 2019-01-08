#include "DSWeapon.h"

BOOL DSWeapon::ReadDataStruct(WEAPON_STRUCT &WeaponData)
{
	BYTE byte;

	if (pProcMem->ReadProcMem(WeaponData.qwBaseAddr + ATTACK_PWR_OFFSET, reinterpret_cast<PBYTE>(&WeaponData.wBasePhysical), sizeof(WORD) * 4) == false)
		return FALSE;

	if (pProcMem->ReadProcMem(WeaponData.qwBaseAddr + ATTACK_PWR_OFFSET2, WeaponData.wBaseDark) == false)
		return FALSE;

	if (pProcMem->ReadProcMem(WeaponData.qwBaseAddr + ADDITIONAL_EFF_OFFSET, reinterpret_cast<PBYTE>(&WeaponData.iEffect1), sizeof(INT) * 3) == false)
		return FALSE;

	if (pProcMem->ReadProcMem(WeaponData.qwBaseAddr + ATTRIBUTE_BONUS_OFFSET, reinterpret_cast<PBYTE>(&WeaponData.fBonusStrength), sizeof(FLOAT) * 4) == false)
		return FALSE;

	if (pProcMem->ReadProcMem(WeaponData.qwBaseAddr + ATTRIBUTE_REQ_OFFSET, reinterpret_cast<PBYTE>(&WeaponData.bReqStrength), sizeof(BYTE) * 4) == false)
		return FALSE;

	if (pProcMem->ReadProcMem(WeaponData.qwBaseAddr + CANBEBUFFED_OFFSET, byte) == true)
		WeaponData.bCanBeBuffed = GETBIT(byte, CANBEBUFFED_BOFFSET);
	else
		return FALSE;

	if (pProcMem->ReadProcMem(WeaponData.qwBaseAddr + BONUSLUCK_OFFSET, byte) == true)
	{
		WeaponData.bBonusLuck = GETBIT(byte, BONUSLUCK_BOFFSET);
		WeaponData.bDisableRepair = GETBIT(byte, DISABLEREPAIR_BOFFSET);
	}
	else
		return FALSE;

	return TRUE;
}

DSWeapon::DSWeapon(Memory* pProcMem)
{
	this->pProcMem = pProcMem;
}

DSWeapon::~DSWeapon()
{

}

BOOL DSWeapon::AddDataStruct(const TEMP_STRUCT &TempData)
{
	PWEAPON_STRUCT PtrWpn;

	if (TempData.dwFileID != FileID::weapon)
		return FALSE;

	PtrWpn = reinterpret_cast<PWEAPON_STRUCT>(TempData.ptrData);
	WeaponDataList.push_back(*PtrWpn);
	WeaponDataBackupList.push_back(*PtrWpn);

	if (ReadDataStruct(WeaponDataBackupList.back()) == FALSE)
	{
		InvalidateAllData();
		return FALSE;
	}

	return TRUE;
}

void DSWeapon::InvalidateAllData()
{
	WeaponDataList.clear();
	WeaponDataBackupList.clear();
}

BOOL DSWeapon::OverwriteMemory(BOOL bWithBackup)
{
	BYTE byte;
	std::vector<WEAPON_STRUCT>* pWeaponDataList = bWithBackup ? &WeaponDataBackupList : &WeaponDataList;

	for (auto const &Weapon : *pWeaponDataList)
	{
		if (Weapon.FieldEnabled[WeaponID::base_physical])
		{
			if (pProcMem->WriteProcMem(Weapon.qwBaseAddr + ATTACK_PWR_OFFSET, Weapon.wBasePhysical) == false)
				return FALSE;
		}

		if (Weapon.FieldEnabled[WeaponID::base_magic])
		{
			if (pProcMem->WriteProcMem(Weapon.qwBaseAddr + ATTACK_PWR_OFFSET + 0x2, Weapon.wBaseMagic) == false)
				return FALSE;
		}

		if (Weapon.FieldEnabled[WeaponID::base_fire])
		{
			if (pProcMem->WriteProcMem(Weapon.qwBaseAddr + ATTACK_PWR_OFFSET + 0x4, Weapon.wBaseFire) == false)
				return FALSE;
		}

		if (Weapon.FieldEnabled[WeaponID::base_lightning])
		{
			if (pProcMem->WriteProcMem(Weapon.qwBaseAddr + ATTACK_PWR_OFFSET + 0x6, Weapon.wBaseLightning) == false)
				return FALSE;
		}

		if (Weapon.FieldEnabled[WeaponID::base_dark])
		{
			if (pProcMem->WriteProcMem(Weapon.qwBaseAddr + ATTACK_PWR_OFFSET2, Weapon.wBaseDark) == false)
				return FALSE;
		}

		if (Weapon.FieldEnabled[WeaponID::effect1])
		{
			if (pProcMem->WriteProcMem(Weapon.qwBaseAddr + ADDITIONAL_EFF_OFFSET, Weapon.iEffect1) == false)
				return FALSE;
		}

		if (Weapon.FieldEnabled[WeaponID::effect2])
		{
			if (pProcMem->WriteProcMem(Weapon.qwBaseAddr + ADDITIONAL_EFF_OFFSET + 0x4, Weapon.iEffect2) == false)
				return FALSE;
		}

		if (Weapon.FieldEnabled[WeaponID::effect3])
		{
			if (pProcMem->WriteProcMem(Weapon.qwBaseAddr + ADDITIONAL_EFF_OFFSET + 0x8, Weapon.iEffect3) == false)
				return FALSE;
		}

		if (Weapon.FieldEnabled[WeaponID::bonus_str])
		{
			if (pProcMem->WriteProcMem(Weapon.qwBaseAddr + ATTRIBUTE_BONUS_OFFSET, Weapon.fBonusStrength) == false)
				return FALSE;
		}

		if (Weapon.FieldEnabled[WeaponID::bonus_dex])
		{
			if (pProcMem->WriteProcMem(Weapon.qwBaseAddr + ATTRIBUTE_BONUS_OFFSET + 0x4, Weapon.fBonusDexterity) == false)
				return FALSE;
		}

		if (Weapon.FieldEnabled[WeaponID::bonus_int])
		{
			if (pProcMem->WriteProcMem(Weapon.qwBaseAddr + ATTRIBUTE_BONUS_OFFSET + 0x8, Weapon.fBonusIntelligence) == false)
				return FALSE;
		}

		if (Weapon.FieldEnabled[WeaponID::bonus_faith])
		{
			if (pProcMem->WriteProcMem(Weapon.qwBaseAddr + ATTRIBUTE_BONUS_OFFSET + 0xC, Weapon.fBonusFaith) == false)
				return FALSE;
		}

		if (Weapon.FieldEnabled[WeaponID::req_str])
		{
			if (pProcMem->WriteProcMem(Weapon.qwBaseAddr + ATTRIBUTE_REQ_OFFSET, Weapon.bReqStrength) == false)
				return FALSE;
		}

		if (Weapon.FieldEnabled[WeaponID::req_dex])
		{
			if (pProcMem->WriteProcMem(Weapon.qwBaseAddr + ATTRIBUTE_REQ_OFFSET + 0x4, Weapon.bReqDexterity) == false)
				return FALSE;
		}

		if (Weapon.FieldEnabled[WeaponID::req_int])
		{
			if (pProcMem->WriteProcMem(Weapon.qwBaseAddr + ATTRIBUTE_REQ_OFFSET + 0x8, Weapon.bReqIntelligence) == false)
				return FALSE;
		}

		if (Weapon.FieldEnabled[WeaponID::req_faith])
		{
			if (pProcMem->WriteProcMem(Weapon.qwBaseAddr + ATTRIBUTE_REQ_OFFSET + 0xC, Weapon.bReqFaith) == false)
				return FALSE;
		}

		if (Weapon.FieldEnabled[WeaponID::buff])
		{
			if (pProcMem->ReadProcMem(Weapon.qwBaseAddr + CANBEBUFFED_OFFSET, byte) == true)
			{
				if (Weapon.bCanBeBuffed)
					SETBIT(byte, CANBEBUFFED_BOFFSET);
				else
					CLBIT(byte, CANBEBUFFED_BOFFSET);

				if (pProcMem->WriteProcMem(Weapon.qwBaseAddr + CANBEBUFFED_OFFSET, byte) == false)
					return FALSE;
			}
			else
				return FALSE;
		}

		if (pProcMem->ReadProcMem(Weapon.qwBaseAddr + BONUSLUCK_OFFSET, byte) == true)
		{
			if (Weapon.FieldEnabled[WeaponID::luck])
			{
				if (Weapon.bBonusLuck)
					SETBIT(byte, BONUSLUCK_BOFFSET);
				else
					CLBIT(byte, BONUSLUCK_BOFFSET);
			}

			if (Weapon.FieldEnabled[WeaponID::repair])
			{
				if (Weapon.bDisableRepair)
					SETBIT(byte, DISABLEREPAIR_BOFFSET);
				else
					CLBIT(byte, DISABLEREPAIR_BOFFSET);
			}

			if (pProcMem->WriteProcMem(Weapon.qwBaseAddr + BONUSLUCK_OFFSET, byte) == false)
				return FALSE;
		}
		else
			return FALSE;
	}

	return TRUE;
}
