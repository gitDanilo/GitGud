#pragma once

#include "DSBase.h"

static const BYTE AOB_ROLL[] =
{
	0x89, 0x81, 0xB4, 0x02, 0x00, 0x00
};

static const BYTE AOB_ROLL_PATCH[] =
{
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90
};

#define OFFSETLIST_SIZE 5
#define MAX_OFFSET_COUNT 5
static DWORD OffsetList[OFFSETLIST_SIZE][MAX_OFFSET_COUNT] =
{
	{0x00000080, 0x00001F90, 0x00000018, 0x00000008, 0x00002098}, // Invisible
	{0x00000080, 0x00001F90, 0x00000040, 0x00000010, 0xFFFFFFFF}, // Super Armor
	{0x00000080, 0x00001F90, 0x00000018, 0x000001C0, 0xFFFFFFFF}, // No Damage, No FP Cost, No Stamina Cost (Basic Status)
	{0x00000080, 0x00001EEA, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}, // No Goods Consume
	{0x00000080, 0x00000050, 0x000002B4, 0xFFFFFFFF, 0xFFFFFFFF}  // Backflip Animation
};

namespace PlayerOffsetID
{
	enum ID: DWORD
	{
		invisible,
		super_armor,
		player_basic_status,
		no_goods_consume,
		backflip_animation,
	};
}

typedef struct _POINTER_STRUCT
{
	PBYTE BaseA; // Player
	PBYTE BaseB; // No Arrows Consume
	PBYTE BaseC; // Roll
} POINTER_STRUCT, *PPOINTER_STRUCT;

class DSPlayer: public DSBase
{
private:
	bool bIsPlayerDataValid;
	PLAYER_STRUCT PlayerData = {};
	bool bIsPlayerDataBackupValid;
	PLAYER_STRUCT PlayerDataBackup = {};
	POINTER_STRUCT PointerData = {};
	PBYTE ReadMultiLvlPtr(PBYTE BaseAddr, DWORD dwID);
	BOOL ReadDataStruct(PLAYER_STRUCT &PlayerData);
public:
	DSPlayer(Memory* pProcMem, const POINTER_STRUCT &PointerData);
	~DSPlayer();
	virtual BOOL AddDataStruct(const TEMP_STRUCT &TempData);
	virtual void InvalidateAllData();
	virtual BOOL OverwriteMemory(BOOL bWithBackup);
};
