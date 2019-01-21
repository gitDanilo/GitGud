#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <fstream>
#include "PatchData.h"
#include "ParamUtil.h"

enum class ReturnID
{
	success, end_of_file, read_error, syntax_error, invalid_field, invalid_data, invalid_id
};

typedef struct _FIELD
{
	char sName[24];
	DataT DataType;
	DWORD dwAddrOffset;
	BYTE BitOffset;
} FIELD, *PFIELD;

typedef struct _STRUCT_FIELD
{
	char sName[24];
	DataT DataType;
	DWORD dwStructOffset;
} STRUCT_FIELD, *PSTRUCT_FIELD;

typedef struct _FILE_INF
{
	char sName[12];
	PFIELD pFieldList;
	PSTRUCT_FIELD pStructFieldList;
	DWORD dwListSize;
} FILE_INF, *PFILE_INF;

typedef struct _RET_INF
{
	ReturnID RetID;
	DWORD dwLine;
} RET_INF, *PRET_INF;

#define PLAYER_FIELD_LIST_SIZE 10
static STRUCT_FIELD PlayerFieldList[PLAYER_FIELD_LIST_SIZE] =
{
	{"player_mod"        , DataT::no_type, MAXDWORD},

	{"invisible"         , DataT::bool_8, offsetof(PLAYER_STRUCT, bInvisible)        },
	{"super_armor"       , DataT::bool_8, offsetof(PLAYER_STRUCT, bSuperArmor)       },
	{"no_damage"         , DataT::bool_8, offsetof(PLAYER_STRUCT, bNoDamage)         },
	{"no_fp_cost"        , DataT::bool_8, offsetof(PLAYER_STRUCT, bNoFPCost)         },
	{"no_stamina_cost"   , DataT::bool_8, offsetof(PLAYER_STRUCT, bNoStaminaConsume) },
	{"no_goods_consume"  , DataT::bool_8, offsetof(PLAYER_STRUCT, bNoGoodsConsume)   },
	{"no_arrows_consume" , DataT::bool_8, offsetof(PLAYER_STRUCT, bNoArrowsConsume)  },
	{"backflip_animation", DataT::bool_8, offsetof(PLAYER_STRUCT, bBackflipAnimation)},
	{"edit_npc_stats"    , DataT::bool_8, offsetof(PLAYER_STRUCT, bEditNPCStat)      }
};

#define WEAPON_FIELD_LIST_SIZE 28
static FIELD WeaponFieldList[WEAPON_FIELD_LIST_SIZE] =
{
	{"weapon_mod"        , DataT::uint_32 , MAXDWORD  , 0},

	{"physical_base"     , DataT::uint_16 , 0x000000C4, 0},
	{"magic_base"        , DataT::uint_16 , 0x000000C6, 0},
	{"fire_base"         , DataT::uint_16 , 0x000000C8, 0},
	{"lightning_base"    , DataT::uint_16 , 0x000000CA, 0},
	{"dark_base"         , DataT::uint_16 , 0x00000188, 0},

	{"effect_on_hit_1"   , DataT::int_32  , 0x00000040, 0},
	{"effect_on_hit_2"   , DataT::int_32  , 0x00000044, 0},
	{"effect_on_hit_3"   , DataT::int_32  , 0x00000048, 0},

	{"effect_on_self_1"  , DataT::int_32  , 0x0000004C, 0},
	{"effect_on_self_2"  , DataT::int_32  , 0x00000050, 0},
	{"effect_on_self_3"  , DataT::int_32  , 0x00000054, 0},

	{"strength_bonus"    , DataT::float_32, 0x00000020, 0},
	{"dexterity_bonus"   , DataT::float_32, 0x00000024, 0},
	{"intelligence_bonus", DataT::float_32, 0x00000028, 0},
	{"faith_bonus"       , DataT::float_32, 0x0000002C, 0},
	{"luck_bonus"        , DataT::float_32, 0x00000198, 0},

	{"strength_req"      , DataT::uint_8  , 0x000000EE, 0},
	{"dexterity_req"     , DataT::uint_8  , 0x000000EF, 0},
	{"intelligence_req"  , DataT::uint_8  , 0x000000F0, 0},
	{"faith_req"         , DataT::uint_8  , 0x000000F1, 0},

	{"demon_dmg_rate"    , DataT::float_32, 0x0000009C, 0},
	{"undead_dmg_rate"   , DataT::float_32, 0x000000A0, 0},
	{"hollow_dmg_rate"   , DataT::float_32, 0x000000A4, 0},
	{"abyssal_dmg_rate"  , DataT::float_32, 0x000000A8, 0},

	{"luck_bonus"        , DataT::bit     , 0x00000103, 0},
	{"can_be_buffed"     , DataT::bit     , 0x00000102, 7},
	{"disable_repair"    , DataT::bit     , 0x00000103, 3}
};

#define EFFECT_FIELD_LIST_SIZE 52
static FIELD EffectFieldList[EFFECT_FIELD_LIST_SIZE] =
{
	{"effect_mod"          , DataT::uint_32 , MAXDWORD  , 0},

	{"effect_duration"     , DataT::float_32, 0x00000008, 0},
	{"motion_interval"     , DataT::float_32, 0x0000000C, 0},

	{"max_hp_rate"         , DataT::float_32, 0x00000010, 0},
	{"max_fp_rate"         , DataT::float_32, 0x00000014, 0},
	{"max_stamina_rate"    , DataT::float_32, 0x00000018, 0},

	{"slash_dmg_rate"      , DataT::float_32, 0x0000001C, 0},
	{"blow_dmg_rate"       , DataT::float_32, 0x00000020, 0},
	{"thrust_dmg_rate"     , DataT::float_32, 0x00000024, 0},
	{"neutral_dmg_rate"    , DataT::float_32, 0x00000028, 0},
	{"magic_dmg_rate"      , DataT::float_32, 0x0000002C, 0},
	{"fire_dmg_rate"       , DataT::float_32, 0x00000030, 0},
	{"lightning_dmg_rate"  , DataT::float_32, 0x00000034, 0},
	// Power rate
	{"physical_power_rate" , DataT::float_32, 0x00000048, 0},
	{"magic_power_rate"    , DataT::float_32, 0x0000004C, 0},
	{"fire_power_rate"     , DataT::float_32, 0x00000050, 0},
	{"lightning_power_rate", DataT::float_32, 0x00000054, 0},
	// Power
	{"physical_power"      , DataT::uint_32 , 0x00000058, 0},
	{"magic_power"         , DataT::uint_32 , 0x0000005C, 0},
	{"fire_power"          , DataT::uint_32 , 0x00000060, 0},
	{"lightning_power"     , DataT::uint_32 , 0x00000064, 0},
	// Defense rate
	{"physical_def_rate"   , DataT::float_32, 0x00000068, 0},
	{"magic_def_rate"      , DataT::float_32, 0x0000006C, 0},
	{"fire_def_rate"       , DataT::float_32, 0x00000070, 0},
	{"lightning_def_rate"  , DataT::float_32, 0x00000074, 0},
	// Defense
	{"physical_def"        , DataT::uint_32 , 0x00000078, 0},
	{"magic_def"           , DataT::uint_32 , 0x0000007C, 0},
	{"fire_def"            , DataT::uint_32 , 0x00000080, 0},
	{"lightning_def"       , DataT::uint_32 , 0x00000084, 0},

	{"change_hp"           , DataT::int_32  , 0x000000A0, 0},
	{"change_fp"           , DataT::int_32  , 0x000000A8, 0},

	{"fall_dmg_rate"       , DataT::float_32, 0x000000DC, 0},

	{"magic_slot"          , DataT::uint_8  , 0x00000142, 0},
	{"miracle_slot"        , DataT::uint_8  , 0x00000143, 0},

	{"vigor_buff"          , DataT::uint_8  , 0x00000308, 0},
	{"attunement_buff"     , DataT::uint_8  , 0x00000309, 0},
	{"endurance_buff"      , DataT::uint_8  , 0x0000030A, 0},
	{"vitality_buff"       , DataT::uint_8  , 0x0000030B, 0},
	{"strenght_buff"       , DataT::uint_8  , 0x0000030C, 0},
	{"dexterity_buff"      , DataT::uint_8  , 0x0000030D, 0},
	{"intelligence_buff"   , DataT::uint_8  , 0x0000030E, 0},
	{"faith_buff"          , DataT::uint_8  , 0x0000030F, 0},
	{"luck_buff"           , DataT::uint_8  , 0x00000310, 0},

	{"poison_rate"         , DataT::uint_32 , 0x000000CC, 0},
	{"frost_rate"          , DataT::uint_32 , 0x000001AC, 0},

	{"replace_effect_id"   , DataT::int_32  , 0x00000124, 0},
	{"cycle_effect_id"     , DataT::int_32  , 0x00000128, 0},
	{"effect_on_hit"       , DataT::int_32  , 0x0000012C, 0},

	{"weapon_blood"        , DataT::uint_32 , 0x000000D4, 0},

	{"soul_rate"           , DataT::float_32, 0x000000E0, 0},
	{"drop_rate"           , DataT::float_32, 0x00000104, 0},

	{"fp_rate"             , DataT::float_32, 0x000002CC, 0}
};

#define ATTACK_FIELD_LIST_SIZE 28
static FIELD AttackFieldList[ATTACK_FIELD_LIST_SIZE] =
{
	{"attack_mod"         , DataT::uint_32 , MAXDWORD  , 0},
	{"hit_1_radius"       , DataT::float_32, 0x00000000, 0},
	{"hit_2_radius"       , DataT::float_32, 0x00000004, 0},
	{"hit_3_radius"       , DataT::float_32, 0x00000008, 0},
	{"hit_4_radius"       , DataT::float_32, 0x0000000C, 0},
	{"knockback_dist"     , DataT::float_32, 0x00000010, 0},
	{"hit_stop_time"      , DataT::float_32, 0x00000014, 0},
	{"effect_id_1"        , DataT::int_32  , 0x00000018, 0},
	{"effect_id_2"        , DataT::int_32  , 0x0000001C, 0},
	{"effect_id_3"        , DataT::int_32  , 0x00000020, 0},
	{"effect_id_4"        , DataT::int_32  , 0x00000024, 0},
	{"effect_id_5"        , DataT::int_32  , 0x00000028, 0},

	{"atk_phys_mod"       , DataT::uint_16 , 0x0000003E, 0},
	{"atk_magic_mod"      , DataT::uint_16 , 0x00000040, 0},
	{"atk_fire_mod"       , DataT::uint_16 , 0x00000042, 0},
	{"atk_lightning_mod"  , DataT::uint_16 , 0x00000044, 0},
	{"atk_stamina_mod"    , DataT::uint_16 , 0x00000046, 0},
	{"guard_atk_rate_mod" , DataT::uint_16 , 0x00000048, 0},
	{"guard_break_mod"    , DataT::uint_16 , 0x0000004A, 0},
	{"atk_super_armor_mod", DataT::uint_16 , 0x0000004E, 0},

	{"atk_physical"       , DataT::uint_16 , 0x00000050, 0},
	{"atk_magic"          , DataT::uint_16 , 0x00000052, 0},
	{"atk_fire"           , DataT::uint_16 , 0x00000054, 0},
	{"atk_lightning"      , DataT::uint_16 , 0x00000056, 0},
	{"atk_stamina"        , DataT::uint_16 , 0x00000058, 0},

	{"guard_atk_rate"     , DataT::uint_16 , 0x0000005A, 0},
	{"guard_break"        , DataT::uint_16 , 0x0000005C, 0},
	{"atk_super_armor"    , DataT::uint_16 , 0x0000005E, 0}
};

#define MAGIC_FIELD_LIST_SIZE 7
static FIELD MagicFieldList[MAGIC_FIELD_LIST_SIZE] =
{
	{"magic_mod"   , DataT::uint_32, MAXDWORD  , 0},
	{"fp_cost"     , DataT::uint_16, 0x0000000C, 0},
	{"stamina_cost", DataT::uint_16, 0x0000000E, 0},
	{"slots_used"  , DataT::uint_8 , 0x0000001D, 0},
	{"req_int"     , DataT::uint_8 , 0x0000001E, 0},
	{"req_faith"   , DataT::uint_8 , 0x0000001F, 0},
	{"cast_anim_id", DataT::uint_8 , 0x00000025, 0}
};

#define BULLET_FIELD_LIST_SIZE 30
static FIELD BulletFieldList[BULLET_FIELD_LIST_SIZE] =
{
	{"bullet_mod"             , DataT::uint_32 , MAXDWORD  , 0},
	{"life"                   , DataT::float_32, 0x00000010, 0},
	{"dist"                   , DataT::float_32, 0x00000014, 0},
	{"shoot_interval"         , DataT::float_32, 0x00000018, 0},
	{"gravity_in_range"       , DataT::float_32, 0x0000001C, 0},
	{"gravity_out_range"      , DataT::float_32, 0x00000020, 0},
	{"homing_stop_range"      , DataT::float_32, 0x00000024, 0},
	{"init_vellocity"         , DataT::float_32, 0x00000028, 0},
	{"accel_in_range"         , DataT::float_32, 0x0000002C, 0},
	{"accel_out_range"        , DataT::float_32, 0x00000030, 0},
	{"max_vellocity"          , DataT::float_32, 0x00000034, 0},
	{"min_vellocity"          , DataT::float_32, 0x00000038, 0},
	{"accel_time"             , DataT::float_32, 0x0000003C, 0},
	{"homing_begin_dist"      , DataT::float_32, 0x00000040, 0},
	{"hit_radius"             , DataT::float_32, 0x00000044, 0},
	{"hit_radius_max"         , DataT::float_32, 0x00000048, 0},
	{"spread_time"            , DataT::float_32, 0x0000004C, 0},
	{"exp_delay"              , DataT::float_32, 0x00000050, 0},
	{"homing_offset_range"    , DataT::float_32, 0x00000054, 0},
	{"dmg_hit_record_lifetime", DataT::float_32, 0x00000058, 0},
	{"effect_id_for_shooter"  , DataT::int_32  , 0x00000060, 0},
	{"hit_bullet_id"          , DataT::int_32  , 0x00000068, 0},
	{"effect_id_1"            , DataT::int_32  , 0x0000006C, 0},
	{"effect_id_2"            , DataT::int_32  , 0x00000070, 0},
	{"effect_id_3"            , DataT::int_32  , 0x00000074, 0},
	{"effect_id_4"            , DataT::int_32  , 0x00000078, 0},
	{"effect_id_5"            , DataT::int_32  , 0x0000007C, 0},
	{"num_shoot"              , DataT::uint_16 , 0x00000080, 0},
	{"homing_angle"           , DataT::uint_16 , 0x00000082, 0},
	{"shoot_angle"            , DataT::uint_16 , 0x00000084, 0}
};

#define FILEINF_LIST_SIZE 6
static FILE_INF FileList[FILEINF_LIST_SIZE] =
{
	{"player.dsm", nullptr, PlayerFieldList, PLAYER_FIELD_LIST_SIZE},
	{"weapon.dsm", WeaponFieldList, nullptr, WEAPON_FIELD_LIST_SIZE},
	{"effect.dsm", EffectFieldList, nullptr, EFFECT_FIELD_LIST_SIZE},
	{"attack.dsm", AttackFieldList, nullptr, ATTACK_FIELD_LIST_SIZE},
	{"magic.dsm" , MagicFieldList , nullptr, MAGIC_FIELD_LIST_SIZE },
	{"bullet.dsm", BulletFieldList, nullptr, BULLET_FIELD_LIST_SIZE}
};

namespace FileID
{
	enum ID: DWORD
	{
		player, weapon, effect, attack, magic, bullet,

		_first = player,
		_last = bullet,
	};
}

class DSMParser
{
private:
	std::ifstream inStream;
	DWORD dwFileID;
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
	bool GetPatchData(const FIELD& Field, bool bHex, PatchData &Patch);
public:
	DSMParser(DWORD dwFileID);
	~DSMParser();
	void SetFile(DWORD dwFileID);
	RET_INF GetPatchDataList(const PARAM_CLASS &Param, std::vector<PatchData> &PatchDataList);
	RET_INF GetStruct(BYTE* pStruct);
};
