#pragma once

#include <wtypes.h>

typedef struct _BASE_STRUCT
{
	PBYTE BaseAddr;
	DWORD dwID;
} BASE_STRUCT, *PBASE_STRUCT;

////////////////
//// Player ////
////////////////
namespace PlayerID
{
	enum ID: DWORD
	{
		id,

		invisible,
		super_armor,
		no_dmg,
		no_fp,
		no_stam,
		no_good,
		no_arrow,
		backflip,

		_first = id,
		_last = backflip,
		_size = _last + 1
	};
}

typedef struct _PLAYER_STRUCT: BASE_STRUCT
{
	BOOL bInvisible;
	BOOL bSuperArmor;
#define NODAMAGE_BOFFSET 1
	BOOL bNoDamage;
#define NOFPCOST_BOFFSET 5
	BOOL bNoFPCost;
#define NOSTAMINA_BOFFSET 4
	BOOL bNoStaminaConsume;
#define NOGOODSCONSUME_BOFFSET 3
	BOOL bNoGoodsConsume;
	BOOL bNoArrowsConsume;
	BOOL bBackflipAnimation;

	bool FieldEnabled[PlayerID::_size];
} PLAYER_STRUCT, *PPLAYER_STRUCT;

////////////////
//// Weapon ////
////////////////
namespace WeaponID
{
	enum ID: DWORD
	{
		id,

		base_physical,
		base_magic,
		base_fire,
		base_lightning,
		base_dark,

		effect1,
		effect2,
		effect3,

		bonus_str,
		bonus_dex,
		bonus_int,
		bonus_faith,

		req_str,
		req_dex,
		req_int,
		req_faith,

		buff,
		luck,
		repair,

		_first = id,
		_last = repair,
		_size = _last + 1
	};
}

typedef struct _WEAPON_STRUCT: BASE_STRUCT
{
	// Attack power
#define ATTACK_PWR_OFFSET 0xC4
	WORD wBasePhysical;
	WORD wBaseMagic;
	WORD wBaseFire;
	WORD wBaseLightning;
#define ATTACK_PWR_OFFSET2 0x188
	WORD wBaseDark;

	// Additional effects
#define ADDITIONAL_EFF_OFFSET 0x40
	INT iEffect1;
	INT iEffect2;
	INT iEffect3;

	// Attribute bonus
#define ATTRIBUTE_BONUS_OFFSET 0x20
	FLOAT fBonusStrength;
	FLOAT fBonusDexterity;
	FLOAT fBonusIntelligence;
	FLOAT fBonusFaith;

	// Attribute requirement
#define ATTRIBUTE_REQ_OFFSET 0xEE
	BYTE bReqStrength;
	BYTE bReqDexterity;
	BYTE bReqIntelligence;
	BYTE bReqFaith;

#define CANBEBUFFED_OFFSET 0x102
#define CANBEBUFFED_BOFFSET 7
	BOOL bCanBeBuffed;

#define BONUSLUCK_OFFSET 0x103
#define BONUSLUCK_BOFFSET 0
	BOOL bBonusLuck;

#define DISABLEREPAIR_OFFSET 0x103
#define DISABLEREPAIR_BOFFSET 3
	BOOL bDisableRepair;

	bool FieldEnabled[WeaponID::_size];
} WEAPON_STRUCT, *PWEAPON_STRUCT;

////////////////
//// Effect ////
////////////////
namespace EffectID
{
	enum ID: DWORD
	{
		id,

		effect_duration,
		duration_inc,

		max_hp,
		max_fp,
		max_stam,

		slash_dmg_rate,
		blow_dmg_rate,
		thrust_dmg_rate,
		neutral_dmg_rate,
		magic_dmg_rate,
		fire_dmg_rate,
		lightning_dmg_rate,

		physical_pwr,
		magic_pwr,
		fire_pwr,
		lightning_pwr,

		change_hp,
		change_fp,

		fall_dmg_rate,

		att_slot,

		vigor_buff,
		att_buff,
		end_buff,
		vit_buff,
		str_buff,
		dex_buff,
		int_buff,
		faith_buff,
		luck_buff,

		poison_rate,
		frost_rate,

		effect_id,

		weapon_blood,

		_first = id,
		_last = weapon_blood,
		_size = _last + 1
	};
}

typedef struct _EFFECT_STRUCT: BASE_STRUCT
{
#define EFFECT_DURATION_OFFSET 0x8
	FLOAT fEffectDuration;
#define DURATION_INC_OFFSET 0xC
	FLOAT fDurationIncrease;

	// Player basic status
#define PLAYER_BASICS_OFFSET 0x10
	FLOAT fMaxHPRate;
	FLOAT fMaxFPRate;
	FLOAT fMaxStaminaRate;

	// Resistance reduction
#define RESISTANCE_RED_OFFSET 0x1C
	FLOAT fSlashDmgRate;
	FLOAT fBlowDmgRate;
	FLOAT fThrustDmgRate;
	FLOAT fNeutralDmgRate;
	FLOAT fMagicDmgRate;
	FLOAT fFireDmgRate;
	FLOAT fLightningDmgRate;

	// Attack power rate
#define ATTACK_PWR_OFFSET 0x48
	FLOAT fPhysicalPower;
	FLOAT fMagicPower;
	FLOAT fFirePower;
	FLOAT fLightningPower;

	// HP and FP per effect proc
#define CHANGE_RGN_OFFSET 0xA0
	INT iChangeHP;
#define CHANGE_RGN_OFFSET2 0xA8
	INT iChangeFP;

#define FALL_DMG_OFFSET 0xDC
	FLOAT fFallDmgRate;

	// Attunement slot
#define ATT_SLOT_OFFSET 0x143
	BYTE bAttunementSlot; // REPEAT DATA ON NEXT BYTE

	// Status buff
#define STATUS_BUFF_OFFSET 0x308
	BYTE bVigorBuff;
	BYTE bAttunementBuff;
	BYTE bEnduranceBuff;
	BYTE bVitalityBuff;
	BYTE bStrengthBuff;
	BYTE bDexterityBuff;
	BYTE bIntelligenceBuff;
	BYTE bFaithBuff;
	BYTE bLuckBuff;

	// Buildup rate
#define POISON_RATE_OFFSET 0xCC
	DWORD dwPoisonRate;
#define FROST_RATE_OFFSET 0x1AC
	DWORD dwFrostRate;

	// Effect
#define EFFECT_ID_OFFSET 0x128
	INT iEffectID;

#define WEAPON_BLOOD_OFFSET 0xD4
	DWORD dwWeaponBlood;

	bool FieldEnabled[EffectID::_size];
} EFFECT_STRUCT, *PEFFECT_STRUCT;
