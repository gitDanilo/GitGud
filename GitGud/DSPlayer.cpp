#include "DSPlayer.h"

PBYTE DSPlayer::ReadMultiLvlPtr(PBYTE BaseAddr, DWORD dwID)
{
	int i, j = MAX_OFFSET_COUNT - 1;
	PBYTE Pointer = BaseAddr;

	if (this->pProcMem->ReadProcMem(Pointer, Pointer) == false)
		return NULL;

	for (i = 0; i < MAX_OFFSET_COUNT; ++i)
	{
		Pointer += OffsetList[dwID][i];

		if (OffsetList[dwID][i + 1] == MAXDWORD || i == j)
			break;

		if (this->pProcMem->ReadProcMem(Pointer, Pointer) == false)
			return NULL;
	}

	return Pointer;
}

BOOL DSPlayer::ReadDataStruct(PLAYER_STRUCT &PlayerData)
{
	BYTE byte;
	PBYTE Addr;

	// Invisible
	Addr = ReadMultiLvlPtr(this->PointerData.BaseA, PlayerOffsetID::invisible);
	if (Addr == 0)
		return FALSE;

	if (pProcMem->ReadProcMem(Addr, byte) == false)
		return FALSE;

	PlayerData.bInvisible = !byte;

	// Super Armor
	Addr = ReadMultiLvlPtr(this->PointerData.BaseA, PlayerOffsetID::super_armor);
	if (Addr == 0)
		return FALSE;

	if (pProcMem->ReadProcMem(Addr, byte) == false)
		return FALSE;

	PlayerData.bSuperArmor = GETBIT(byte, 0);

	// No Damage
	Addr = ReadMultiLvlPtr(this->PointerData.BaseA, PlayerOffsetID::player_basic_status);
	if (Addr == 0)
		return FALSE;

	if (pProcMem->ReadProcMem(Addr, byte) == false)
		return FALSE;

	PlayerData.bNoDamage = GETBIT(byte, NODAMAGE_BOFFSET);

	// No FP Cost
	PlayerData.bNoFPCost = GETBIT(byte, NOFPCOST_BOFFSET);

	// No Stamina Consume
	PlayerData.bNoStaminaConsume = GETBIT(byte, NOSTAMINA_BOFFSET);

	// No Arrows Consume
	Addr = 0;
	if (pProcMem->ReadProcMem(this->PointerData.BaseB + 0x4, Addr) == false)
		return FALSE;

	if (pProcMem->ReadProcMem(Addr, byte) == false)
		return FALSE;

	PlayerData.bNoArrowsConsume = byte;

	// Backflip Animation
	if (pProcMem->ReadProcMem(this->PointerData.BaseC, byte) == false)
		return FALSE;

	PlayerData.bBackflipAnimation = (byte == 0x90 ? TRUE : FALSE);

	return TRUE;
}

DSPlayer::DSPlayer(Memory* pProcMem, const POINTER_STRUCT &PointerData)
{
	this->pProcMem = pProcMem;
	this->bIsPlayerDataValid = false;
	this->bIsPlayerDataBackupValid = false;
	CopyMemory(&this->PointerData, &PointerData, sizeof(POINTER_STRUCT));
}

DSPlayer::~DSPlayer()
{

}

BOOL DSPlayer::AddDataStruct(const TEMP_STRUCT &TempData)
{
	if (TempData.dwFileID != FileID::player)
		return FALSE;

	if (CopyMemory(&this->PlayerData, TempData.ptrData, TempData.dwSize) == false)
		return FALSE;

	if (ReadDataStruct(this->PlayerDataBackup) == FALSE)
		return FALSE;

	this->bIsPlayerDataValid = true;
	this->bIsPlayerDataBackupValid = true;

	return TRUE;
}

void DSPlayer::InvalidateAllData()
{
	this->bIsPlayerDataValid = false;
	this->bIsPlayerDataBackupValid = false;
}

BOOL DSPlayer::OverwriteMemory(BOOL bWithBackup)
{
	BYTE byte;
	int int_32;
	PBYTE Addr;
	PPLAYER_STRUCT pPlayerData = NULL;

	if (bWithBackup)
	{
		if (this->bIsPlayerDataBackupValid == false)
			return FALSE;
		pPlayerData = &PlayerDataBackup;
	}
	else
	{
		if (this->bIsPlayerDataValid == false)
			return FALSE;
		pPlayerData = &PlayerData;
	}

	// Invisible
	if (pPlayerData->FieldEnabled[PlayerID::invisible])
	{
		Addr = ReadMultiLvlPtr(this->PointerData.BaseA, PlayerOffsetID::invisible);
		if (Addr == 0)
			return FALSE;

		byte = pPlayerData->bInvisible ? 0 : 1;
		if (pProcMem->WriteProcMem(Addr, byte) == false)
			return FALSE;
	}

	// Super Armor
	if (pPlayerData->FieldEnabled[PlayerID::super_armor])
	{
		Addr = ReadMultiLvlPtr(this->PointerData.BaseA, PlayerOffsetID::super_armor);
		if (Addr == 0)
			return FALSE;

		if (pProcMem->ReadProcMem(Addr, byte) == false)
			return FALSE;

		if (pPlayerData->bSuperArmor)
			SETBIT(byte, 0);
		else
			CLBIT(byte, 0);

		if (pProcMem->WriteProcMem(Addr, byte) == false)
			return FALSE;
	}

	// No Damage
	Addr = ReadMultiLvlPtr(this->PointerData.BaseA, PlayerOffsetID::player_basic_status);
	if (Addr == 0)
		return FALSE;

	if (pProcMem->ReadProcMem(Addr, byte) == false)
		return FALSE;

	if (pPlayerData->FieldEnabled[PlayerID::no_dmg])
	{
		if (pPlayerData->bNoDamage)
			SETBIT(byte, NODAMAGE_BOFFSET);
		else
			CLBIT(byte, NODAMAGE_BOFFSET);
	}

	// No FP Cost
	if (pPlayerData->FieldEnabled[PlayerID::no_fp])
	{
		if (pPlayerData->bNoFPCost)
			SETBIT(byte, NOFPCOST_BOFFSET);
		else
			CLBIT(byte, NOFPCOST_BOFFSET);
	}

	// No Stamina Consume
	if (pPlayerData->FieldEnabled[PlayerID::no_stam])
	{
		if (pPlayerData->bNoStaminaConsume)
			SETBIT(byte, NOSTAMINA_BOFFSET);
		else
			CLBIT(byte, NOSTAMINA_BOFFSET);
	}

	if (pProcMem->WriteProcMem(Addr, byte) == false)
		return FALSE;

	// No Goods Consume
	if (pPlayerData->FieldEnabled[PlayerID::no_good])
	{
		Addr = ReadMultiLvlPtr(this->PointerData.BaseA, PlayerOffsetID::no_goods_consume);
		if (Addr == 0)
			return FALSE;

		if (pProcMem->ReadProcMem(Addr, byte) == false)
			return FALSE;

		if (pPlayerData->bNoGoodsConsume)
			SETBIT(byte, NOGOODSCONSUME_BOFFSET);
		else
			CLBIT(byte, NOGOODSCONSUME_BOFFSET);

		if (pProcMem->WriteProcMem(Addr, byte) == false)
			return FALSE;
	}

	// No Arrows Consume
	if (pPlayerData->FieldEnabled[PlayerID::no_arrow])
	{
		byte = pPlayerData->bNoArrowsConsume ? 1 : 0;
		if (pProcMem->WriteProcMem(this->PointerData.BaseB + 0x4, byte) == false)
			return FALSE;
	}

	// Backflip Animation
	if (pPlayerData->FieldEnabled[PlayerID::backflip])
	{
		if (pProcMem->WriteProcMem(this->PointerData.BaseC, const_cast<PBYTE>(pPlayerData->bBackflipAnimation ? AOB_ROLL_PATCH : AOB_ROLL), sizeof(AOB_ROLL)) == false)
			return FALSE;

		if (pPlayerData->bBackflipAnimation)
		{
			Addr = ReadMultiLvlPtr(this->PointerData.BaseA, PlayerOffsetID::backflip_animation);
			if (Addr == 0)
				return FALSE;

			int_32 = 0;
			if (pProcMem->WriteProcMem(Addr, int_32) == false)
				return FALSE;
		}
	}

	return TRUE;
}
