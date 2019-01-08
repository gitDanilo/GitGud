#include "DSEffect.h"

BOOL DSEffect::ReadDataStruct(EFFECT_STRUCT &EffectData)
{
	if (pProcMem->ReadProcMem(EffectData.qwBaseAddr + EFFECT_DURATION_OFFSET, EffectData.fEffectDuration) == false)
		return FALSE;

	if (pProcMem->ReadProcMem(EffectData.qwBaseAddr + DURATION_INC_OFFSET, EffectData.fDurationIncrease) == false)
		return FALSE;

	if (pProcMem->ReadProcMem(EffectData.qwBaseAddr + PLAYER_BASICS_OFFSET, reinterpret_cast<PBYTE>(&EffectData.fMaxHPRate), sizeof(FLOAT) * 3) == false)
		return FALSE;

	if (pProcMem->ReadProcMem(EffectData.qwBaseAddr + RESISTANCE_RED_OFFSET, reinterpret_cast<PBYTE>(&EffectData.fSlashDmgRate), sizeof(FLOAT) * 7) == false)
		return FALSE;

	if (pProcMem->ReadProcMem(EffectData.qwBaseAddr + ATTACK_PWR_OFFSET, reinterpret_cast<PBYTE>(&EffectData.fPhysicalPower), sizeof(FLOAT) * 4) == false)
		return FALSE;

	if (pProcMem->ReadProcMem(EffectData.qwBaseAddr + CHANGE_RGN_OFFSET, EffectData.iChangeHP) == false)
		return FALSE;

	if (pProcMem->ReadProcMem(EffectData.qwBaseAddr + CHANGE_RGN_OFFSET2, EffectData.iChangeFP) == false)
		return FALSE;

	if (pProcMem->ReadProcMem(EffectData.qwBaseAddr + ATT_SLOT_OFFSET, EffectData.bAttunementSlot) == false)
		return FALSE;

	if (pProcMem->ReadProcMem(EffectData.qwBaseAddr + ATT_SLOT_OFFSET + 1, EffectData.bAttunementSlot) == false)
		return FALSE;

	if (pProcMem->ReadProcMem(EffectData.qwBaseAddr + STATUS_BUFF_OFFSET, reinterpret_cast<PBYTE>(&EffectData.bVigorBuff), sizeof(BYTE) * 9) == false)
		return FALSE;

	if (pProcMem->ReadProcMem(EffectData.qwBaseAddr + POISON_RATE_OFFSET, EffectData.dwPoisonRate) == false)
		return FALSE;

	if (pProcMem->ReadProcMem(EffectData.qwBaseAddr + FROST_RATE_OFFSET, EffectData.dwFrostRate) == false)
		return FALSE;

	if (pProcMem->ReadProcMem(EffectData.qwBaseAddr + EFFECT_ID_OFFSET, EffectData.iEffectID) == false)
		return FALSE;

	if (pProcMem->ReadProcMem(EffectData.qwBaseAddr + WEAPON_BLOOD_OFFSET, EffectData.dwWeaponBlood) == false)
		return FALSE;

	return TRUE;
}

DSEffect::DSEffect(Memory* pProcMem)
{
	this->pProcMem = pProcMem;
}

DSEffect::~DSEffect()
{

}

BOOL DSEffect::AddDataStruct(const TEMP_STRUCT &TempData)
{
	return 0;
}

void DSEffect::InvalidateAllData()
{
	EffectDataList.clear();
	EffectDataBackupList.clear();
}

BOOL DSEffect::OverwriteMemory(BOOL bWithBackup)
{
	std::vector<EFFECT_STRUCT>* pEffectDataList = bWithBackup ? &EffectDataBackupList : &EffectDataList;

	for (auto const &Effect : *pEffectDataList)
	{
		if (Effect.FieldEnabled[EffectID::effect_duration])
		{
			if (pProcMem->WriteProcMem(Effect.BaseAddr + EFFECT_DURATION_OFFSET, Effect.fEffectDuration) == false)
				return FALSE;
		}

		if (Effect.FieldEnabled[EffectID::duration_inc])
		{
			if (pProcMem->WriteProcMem(Effect.BaseAddr + DURATION_INC_OFFSET, Effect.fDurationIncrease) == false)
				return FALSE;
		}

		if (Effect.FieldEnabled[EffectID::max_hp])
		{
			if (pProcMem->WriteProcMem(Effect.BaseAddr + PLAYER_BASICS_OFFSET, Effect.fMaxHPRate) == false)
				return FALSE;
		}

		if (Effect.FieldEnabled[EffectID::max_fp])
		{
			if (pProcMem->WriteProcMem(Effect.BaseAddr + PLAYER_BASICS_OFFSET + 0x4, Effect.fMaxFPRate) == false)
				return FALSE;
		}

		if (Effect.FieldEnabled[EffectID::max_stam])
		{
			if (pProcMem->WriteProcMem(Effect.BaseAddr + PLAYER_BASICS_OFFSET + 0x8, Effect.fMaxStaminaRate) == false)
				return FALSE;
		}

		//
		if (Effect.FieldEnabled[EffectID::slash_dmg_rate])
		{
			if (pProcMem->WriteProcMem(Effect.BaseAddr + RESISTANCE_RED_OFFSET, Effect.fSlashDmgRate) == false)
				return FALSE;
		}

		if (Effect.FieldEnabled[EffectID::blow_dmg_rate])
		{
			if (pProcMem->WriteProcMem(Effect.BaseAddr + RESISTANCE_RED_OFFSET + 0x4, Effect.fBlowDmgRate) == false)
				return FALSE;
		}

		if (Effect.FieldEnabled[EffectID::thrust_dmg_rate])
		{
			if (pProcMem->WriteProcMem(Effect.BaseAddr + RESISTANCE_RED_OFFSET + 0x8, Effect.fThrustDmgRate) == false)
				return FALSE;
		}

		if (Effect.FieldEnabled[EffectID::neutral_dmg_rate])
		{
			if (pProcMem->WriteProcMem(Effect.BaseAddr + RESISTANCE_RED_OFFSET + 0xC, Effect.fNeutralDmgRate) == false)
				return FALSE;
		}

		if (Effect.FieldEnabled[EffectID::magic_dmg_rate])
		{
			if (pProcMem->WriteProcMem(Effect.BaseAddr + RESISTANCE_RED_OFFSET + 0x10, Effect.fMagicDmgRate) == false)
				return FALSE;
		}

		if (Effect.FieldEnabled[EffectID::fire_dmg_rate])
		{
			if (pProcMem->WriteProcMem(Effect.BaseAddr + RESISTANCE_RED_OFFSET + 0x14, Effect.fFireDmgRate) == false)
				return FALSE;
		}

		if (Effect.FieldEnabled[EffectID::lightning_dmg_rate])
		{
			if (pProcMem->WriteProcMem(Effect.BaseAddr + RESISTANCE_RED_OFFSET + 0x18, Effect.fLightningDmgRate) == false)
				return FALSE;
		}
		// TODO
		if (pProcMem->WriteProcMem(Effect.BaseAddr + ATTACK_PWR_OFFSET, reinterpret_cast<PBYTE>(&Effect.fPhysicalPower), sizeof(FLOAT) * 4) == false)
			return FALSE;

		if (pProcMem->WriteProcMem(Effect.BaseAddr + CHANGE_RGN_OFFSET, Effect.iChangeHP) == false)
			return FALSE;

		if (pProcMem->WriteProcMem(Effect.BaseAddr + CHANGE_RGN_OFFSET2, Effect.iChangeFP) == false)
			return FALSE;

		if (pProcMem->WriteProcMem(Effect.BaseAddr + ATT_SLOT_OFFSET, Effect.bAttunementSlot) == false)
			return FALSE;

		if (pProcMem->WriteProcMem(Effect.BaseAddr + ATT_SLOT_OFFSET + 1, Effect.bAttunementSlot) == false)
			return FALSE;

		if (pProcMem->WriteProcMem(Effect.BaseAddr + STATUS_BUFF_OFFSET, reinterpret_cast<PBYTE>(&Effect.bVigorBuff), sizeof(BYTE) * 9) == false)
			return FALSE;

		if (pProcMem->WriteProcMem(Effect.BaseAddr + POISON_RATE_OFFSET, Effect.dwPoisonRate) == false)
			return FALSE;

		if (pProcMem->WriteProcMem(Effect.BaseAddr + FROST_RATE_OFFSET, Effect.dwFrostRate) == false)
			return FALSE;

		if (pProcMem->WriteProcMem(Effect.BaseAddr + EFFECT_ID_OFFSET, Effect.iEffectID) == false)
			return FALSE;

		if (pProcMem->WriteProcMem(Effect.BaseAddr + WEAPON_BLOOD_OFFSET, Effect.dwWeaponBlood) == false)
			return FALSE;
	}
	return TRUE;
}
