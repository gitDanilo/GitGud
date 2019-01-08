#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <fstream>
#include "PatchData.h"

#define DSM_PLAYER_FILE "player.dsm"
#define DSM_WEAPON_FILE "weapon.dsm"
#define DSM_EFFECT_FILE "effect.dsm"

enum class FileID
{
	player, weapon, effect
};

enum class ReturnID
{
	success, end_of_file, read_error, syntax_error, invalid_field, invalid_data
};

typedef struct _FIELD
{
	char sName[24];
	DataT DataType;
	DWORD dwAddrOffset;
	BYTE BitOffset;
} FIELD, *PFIELD;

typedef struct _RET_INF
{
	ReturnID RetID;
	DWORD dwLine;
} RET_INF, *PRET_INF;

#define PLAYER_FIELD_LIST_SIZE 9
static FIELD PlayerFieldList[PLAYER_FIELD_LIST_SIZE] =
{
	{"player_mod"        , DataT::no_type, MAXDWORD, 0},

	{"invisible"         , DataT::bool_8 , MAXDWORD, 0},
	{"super_armor"       , DataT::bool_8 , MAXDWORD, 0},
	{"no_damage"         , DataT::bool_8 , MAXDWORD, 0},
	{"no_fp_cost"        , DataT::bool_8 , MAXDWORD, 0},
	{"no_stamina_cost"   , DataT::bool_8 , MAXDWORD, 0},
	{"no_goods_consume"  , DataT::bool_8 , MAXDWORD, 0},
	{"no_arrows_consume" , DataT::bool_8 , MAXDWORD, 0},
	{"backflip_animation", DataT::bool_8 , MAXDWORD, 0}
};

#define WEAPON_FIELD_LIST_SIZE 20
static FIELD WeaponFieldList[WEAPON_FIELD_LIST_SIZE] =
{
	{"weapon_mod"        , DataT::uint_32 , MAXDWORD  , 0},

	{"physical_base"     , DataT::uint_16 , 0x000000C4, 0},
	{"magic_base"        , DataT::uint_16 , 0x000000C6, 0},
	{"fire_base"         , DataT::uint_16 , 0x000000CA, 0},
	{"lightning_base"    , DataT::uint_16 , 0x000000CC, 0},
	{"dark_base"         , DataT::uint_16 , 0x00000188, 0},

	{"effect_1"          , DataT::int_32  , 0x00000040, 0},
	{"effect_2"          , DataT::int_32  , 0x00000044, 0},
	{"effect_3"          , DataT::int_32  , 0x00000048, 0},

	{"strength_bonus"    , DataT::float_32, 0x00000020, 0},
	{"dexterity_bonus"   , DataT::float_32, 0x00000024, 0},
	{"intelligence_bonus", DataT::float_32, 0x00000028, 0},
	{"faith_bonus"       , DataT::float_32, 0x0000002C, 0},

	{"strength_req"      , DataT::uint_8  , 0x000000EE, 0},
	{"dexterity_req"     , DataT::uint_8  , 0x000000EF, 0},
	{"intelligence_req"  , DataT::uint_8  , 0x000000F0, 0},
	{"faith_req"         , DataT::uint_8  , 0x000000F1, 0},

	{"luck_bonus"        , DataT::bit     , 0x00000103, 0},
	{"can_be_buffed"     , DataT::bit     , 0x00000102, 0},
	{"disable_repair"    , DataT::bit     , 0x00000103, 3}
};

#define EFFECT_FIELD_LIST_SIZE 34
static FIELD EffectFieldList[EFFECT_FIELD_LIST_SIZE] =
{
	{"effect_mod"        , DataT::uint_32 , MAXDWORD  , 0},

	{"effect_duration"   , DataT::float_32, 0x00000008, 0},
	{"duration_increase" , DataT::float_32, 0x0000000C, 0},

	{"max_hp_rate"       , DataT::float_32, 0x00000010, 0},
	{"max_fp_rate"       , DataT::float_32, 0x00000014, 0},
	{"max_stamina_rate"  , DataT::float_32, 0x00000018, 0},

	{"slash_dmg_rate"    , DataT::float_32, 0x0000001C, 0},
	{"blow_dmg_rate"     , DataT::float_32, 0x00000020, 0},
	{"thrust_dmg_rate"   , DataT::float_32, 0x00000024, 0},
	{"neutral_dmg_rate"  , DataT::float_32, 0x00000028, 0},
	{"magic_dmg_rate"    , DataT::float_32, 0x0000002C, 0},
	{"fire_dmg_rate"     , DataT::float_32, 0x00000030, 0},
	{"lightning_dmg_rate", DataT::float_32, 0x00000034, 0},

	{"physical_power"    , DataT::float_32, 0x00000048, 0},
	{"magic_power"       , DataT::float_32, 0x0000004C, 0},
	{"fire_power"        , DataT::float_32, 0x00000050, 0},
	{"lightning_power"   , DataT::float_32, 0x00000054, 0},

	{"change_hp"         , DataT::int_32  , 0x000000A0, 0},
	{"change_fp"         , DataT::int_32  , 0x000000A8, 0},

	{"fall_dmg_rate"     , DataT::float_32, 0x000000DC, 0},

	{"attunement_slot"   , DataT::uint_8  , 0x00000143, 0},

	{"vigor_buff"        , DataT::uint_8  , 0x00000308, 0},
	{"attunement_buff"   , DataT::uint_8  , 0x00000309, 0},
	{"endurance_buff"    , DataT::uint_8  , 0x0000030A, 0},
	{"vitality_buff"     , DataT::uint_8  , 0x0000030B, 0},
	{"strenght_buff"     , DataT::uint_8  , 0x0000030C, 0},
	{"dexterity_buff"    , DataT::uint_8  , 0x0000030D, 0},
	{"intelligence_buff" , DataT::uint_8  , 0x0000030E, 0},
	{"faith_buff"        , DataT::uint_8  , 0x0000030F, 0},
	{"luck_buff"         , DataT::uint_8  , 0x00000310, 0},

	{"poison_rate"       , DataT::uint_32 , 0x000000CC, 0},
	{"frost_rate"        , DataT::uint_32 , 0x000001AC, 0},

	{"effect_id"         , DataT::int_32  , 0x00000128, 0},

	{"weapon_blood"      , DataT::uint_32 , 0x000000D4, 0}
};

class DSMParser
{
private:
	std::ifstream inStream;
	FileID File;
	DWORD dwLineCount;
	bool bHeader;
	std::size_t lPos;
	RET_INF RetInf;

	std::string sLine;
	std::string sField;
	std::string sFieldData;

	bool ReadNextLine();
	bool GetNextField();
	RET_INF SetRetInf(ReturnID RetID);
	bool GetPatchData(const FIELD& Field, PatchData &Data);
public:
	DSMParser(FileID File);
	~DSMParser();
	RET_INF GetFirstDataStruct(PVOID ptrData);
	RET_INF GetNextDataStruct(PVOID ptrData);
};
