#pragma once

#include <vector>
#include "Memory.h"
#include "PatchData.h"

typedef struct _PARAM_OBJECT
{
	DWORD dwID;
	DWORD dwOffset;
	BYTE* Address;
} PARAM_OBJECT, *PPARAM_OBJECT;

typedef struct _PARAM_CLASS
{
	char sParamName[32];
	BYTE* ParamAddr;
	PPARAM_OBJECT ObjList;
	WORD wObjCount;
} PARAM_CLASS, *PPARAM_CLASS;

typedef struct _PATCH_INF
{
	std::vector<PatchData> PatchList;
	std::vector<PatchData> PatchBackupList;
} PATCH_INF, *PPATCH_INF;

typedef struct _PLAYER_STRUCT
{
	bool bInvisible;
	bool bSuperArmor;
#define NODAMAGE_BIT_OFFSET 1
	bool bNoDamage;
#define NOFPCOST_BIT_OFFSET 5
	bool bNoFPCost;
#define NOSTAMINA_BIT_OFFSET 4
	bool bNoStaminaConsume;
#define NOGOODSCONSUME_BIT_OFFSET 3
	bool bNoGoodsConsume;
	bool bNoArrowsConsume;
	bool bBackflipAnimation;
	bool bNPCRedEyeVFX;
	bool bNPCAggroMod;
	bool bEditNPCStat;
} PLAYER_STRUCT, *PPLAYER_STRUCT;

static const BYTE AOB_ROLL[] =
{
	0x89, 0x81, 0xB4, 0x02, 0x00, 0x00
};

static const BYTE AOB_ROLL_PATCH[] =
{
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90
};

#define OFFSET_LIST_SIZE 5
#define MAX_OFFSET_COUNT 5
static DWORD OffsetList[OFFSET_LIST_SIZE][MAX_OFFSET_COUNT] =
{
	{0x00000080, 0x00001F90, 0x00000018, 0x00000008, 0x00002098}, // Invisible
	{0x00000080, 0x00001F90, 0x00000040, 0x00000010, 0xFFFFFFFF}, // Super Armor
	{0x00000080, 0x00001F90, 0x00000018, 0x000001C0, 0xFFFFFFFF}, // No Damage, No FP Cost, No Stamina Cost (Basic Status)
	{0x00000080, 0x00001EEA, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}, // No Goods Consume
	{0x00000080, 0x00000050, 0x000002B4, 0xFFFFFFFF, 0xFFFFFFFF}  // Backflip Animation
};

namespace OffsetID
{
	enum ID: DWORD
	{
		invisible,
		super_armor,
		player_basic_status,
		no_goods_consume,
		backflip_animation
	};
}

#define EFFECT_ID_OFFSET 0x128
#define NPCEFFECT_LIST_SIZE 20
static DWORD NPCEffectList[NPCEFFECT_LIST_SIZE] =
{
	7400, 7410, 7420, 7430, 7440, 7445, 7460, 7470, 7480, 7490, 7492, 7493, 7495, 7520, 7530, 7535, 7540, 7550, 7560, 7570
};

namespace ParamUtil
{
	bool WC2MB(LPWCH wcString, int wcCount, LPCH mbString, int mbMaxSize);
	BYTE* ReadMultiLvlPtr(BYTE* BaseAddr, DWORD dwID);
	bool PatchBin(PatchData &BinPatch);
	bool PatchMemory(const PLAYER_STRUCT &PlayerData, BYTE* BaseA, BYTE* BaseB, BYTE* BaseC, const PARAM_CLASS &EffectParam);
	bool PatchMemory(std::vector<PatchData> &PatchList);
	bool PatchMemoryWithBackup(PATCH_INF &PatchInf);
	BYTE* GetIDAddr(const PARAM_CLASS &ParamClass, DWORD dwID);
	WORD LoadObjList(PARAM_CLASS &ParamClass);
	bool LoadParamList(const BYTE* ParamPatch, PPARAM_CLASS ParamList, DWORD dwParamListSize);
	void DestroyParamList(PPARAM_CLASS ParamList, DWORD dwParamListSize);
}
