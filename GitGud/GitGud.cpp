#include <iostream>
#include <iomanip>
#include <string>
#include "Color.h"
#include "DSHack.h"
#include "DSMParser.h"

///////////////////
//// Constants ////
///////////////////
#define FIVE_SECONDS     5000
#define DEFAULT_COLOR    7
#define VECTOR_START_CAP 48
#define PROCESS_NAME     "DarkSoulsIII.exe"

/////////////////
//// Structs ////
/////////////////
typedef struct _MODULE_INFO
{
	char sModuleName[32];
	BYTE* ModuleBaseAddr;
	DWORD dwModuleSize;
	BYTE* aobModule;
} MODULE_INFO, *PMODULE_INFO;

typedef struct _AOB_PATTERN
{
	DWORD dwModuleID;
	DWORD dwOffset;
	DWORD dwExtra;
	BYTE* aobPattern;
	DWORD dwPatternSize;
	BYTE* Address;
	DWORD dwAddrOffset;
} AOB_PATTERN, *PAOB_PATTERN;

////////////////////
//// Signatures ////
////////////////////
static const BYTE AOB_PARAM_PATCH[] =
{
	0x48, 0x8B, 0x0D, 0xCC, 0xCC, 0xCC, 0xCC, 0x48, 0x85, 0xC9, 0x74, 0x0B, 0x4C, 0x8B, 0xC0, 0x48, 0x8B, 0xD7
};

static const BYTE AOB_BASE_A[] = // Player
{
	0x48, 0x8B, 0x1D, 0xCC, 0xCC, 0xCC, 0x04, 0x48, 0x8B, 0xF9, 0x48, 0x85, 0xDB, 0xCC, 0xCC, 0x8B, 0x11, 0x85, 0xD2, 0xCC, 0xCC, 0x8D
};

static const BYTE AOB_BASE_B[] = // Arrow
{
	0x4C, 0x8D, 0x05, 0xCC, 0xCC, 0xCC, 0xCC, 0x48, 0x8D, 0x15, 0xCC, 0xCC, 0xCC, 0xCC, 0x48, 0x8B, 0xCB, 0xE8, 0xCC, 0xCC, 0xCC, 0xCC,
	0x48, 0x83, 0x3D, 0xCC, 0xCC, 0xCC, 0xCC, 0x00
};

static const BYTE AOB_BASE_C[] = // Roll
{
	0x89, 0x81, 0xB4, 0x02, 0x00, 0x00, 0x48, 0x85, 0xDB, 0x74, 0x20, 0x48
};

///////////////
//// Lists ////
///////////////
#define MODULELIST_SIZE 1
static MODULE_INFO ModuleList[MODULELIST_SIZE] =
{
	{"DarkSoulsIII.exe", 0, 0, nullptr}
};

namespace ModuleID
{
	enum ID : DWORD
	{
		base
	};
}

#define PATTERNLIST_SIZE 4
static AOB_PATTERN PatternList[PATTERNLIST_SIZE] =
{
	{ModuleID::base, 3, 7, const_cast<BYTE*>(AOB_PARAM_PATCH), sizeof(AOB_PARAM_PATCH), nullptr, 0},
	{ModuleID::base, 3, 7, const_cast<BYTE*>(AOB_BASE_A)     , sizeof(AOB_BASE_A)     , nullptr, 0},
	{ModuleID::base, 3, 7, const_cast<BYTE*>(AOB_BASE_B)     , sizeof(AOB_BASE_B)     , nullptr, 0},
	{ModuleID::base, 0, 0, const_cast<BYTE*>(AOB_BASE_C)     , sizeof(AOB_BASE_C)     , nullptr, 0}
};

namespace SigID
{
	enum ID : DWORD
	{
		param_patch,
		base_a,
		base_b,
		base_c
	};
}

#define PARAMLIST_SIZE 17
static PARAM_CLASS ParamList[PARAMLIST_SIZE] =
{
	{"EquipParamWeapon"    , nullptr, nullptr, 0},
	{"EquipParamProtector" , nullptr, nullptr, 0},
	{"EquipParamGoods"     , nullptr, nullptr, 0},
	{"EquipParamAccessory" , nullptr, nullptr, 0},
	{"SpEffectParam"       , nullptr, nullptr, 0},
	{"ThrowParam"          , nullptr, nullptr, 0},
	{"BehaviorParam_PC"    , nullptr, nullptr, 0},
	{"AtkParam_Pc"         , nullptr, nullptr, 0},
	{"SpEffectVfxParam"    , nullptr, nullptr, 0},
	{"ObjectParam"         , nullptr, nullptr, 0},
	{"BonfireWarpParam"    , nullptr, nullptr, 0},
	{"HitEffectSfxParam"   , nullptr, nullptr, 0},
	{"ShopLineupParam"     , nullptr, nullptr, 0},
	{"ItemLotParam"        , nullptr, nullptr, 0},
	{"SwordArtsParam"      , nullptr, nullptr, 0},
	{"Magic"               , nullptr, nullptr, 0},
	{"Bullet"              , nullptr, nullptr, 0}
};

namespace ParamID
{
	enum ID: DWORD
	{
		equip_param_weapon,
		equip_param_protector,
		equip_param_goods,
		equip_param_accessory,
		sp_effect_param,
		throw_param,
		behavior_param_pc,
		atk_param_pc,
		sp_effect_vfx_param,
		object_param,
		bonfire_warp_param,
		hit_effect_sfx_param,
		shop_lineup_param,
		item_lot_param,
		sword_arts_param,
		magic,
		bullet
	};
}

enum class ActionID
{
	patch_all, patch_player, toggle_gravity, toggle_ai, print_ds_struct, exit
};

enum class PatchID
{
	patch_weapon, patch_effect, patch_attack, patch_magic, patch_bullet
};

//////////////////////////
//// Global variables ////
//////////////////////////
CONSOLE_SCREEN_BUFFER_INFO DefaultConsoleInfo = {};

PLAYER_STRUCT PlayerData;
PLAYER_STRUCT PlayerDataBackup = {false, false, false, false, false, false, false, false};
PATCH_INF WeaponPatchInf;
PATCH_INF EffectPatchInf;
PATCH_INF AttackPatchInf;
PATCH_INF MagicPatchInf;
PATCH_INF BulletPatchInf;

Memory* pProcMem    = nullptr;
HHOOK hKeyboardHook = nullptr;
BYTE* ParamPatch    = nullptr;
BYTE* BaseA         = nullptr;
BYTE* BaseB         = nullptr;
BYTE* BaseC         = nullptr;

/////////////////////////////
//// Methods declaration ////
/////////////////////////////
BOOL WINAPI ConsoleEventHandler(DWORD dwEvent);
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);

void OnKBEvent(bool bShift, PatchID Patch);
void OnKBEvent2(bool bShift, ActionID Action);

bool ScanSignatures();
bool LoadModules();

bool GetStrData(const BYTE* Addr, DataT DataType, BYTE BitOffset, std::string &sData);

bool PrintDSStruct();
void PrintParamList();
void PrintParserError(const RET_INF &Inf);
void PrintInputOptions();

bool LoadPatchData(DWORD dwFileID);
void Cleanup();

void SuccessBeep();
void SuccessBeep2();
void FailBeep();

////////////////////////////
//// Methods definition ////
////////////////////////////
void SuccessBeep()
{
	Beep(600, 50);
	Beep(800, 50);
	Beep(1600, 100);
}

void SuccessBeep2()
{
	Beep(1600, 50);
	Beep(800, 50);
	Beep(600, 100);
}

void FailBeep()
{
	Beep(600, 50);
	Beep(600, 50);
}

BOOL WINAPI ConsoleEventHandler(DWORD dwEvent)
{
	switch (dwEvent)
	{
		case CTRL_C_EVENT:
		case CTRL_CLOSE_EVENT:
		case CTRL_LOGOFF_EVENT:
		case CTRL_SHUTDOWN_EVENT:
			std::cout << clr::white << "Exiting..." << std::endl;
			Cleanup();
		default:
			return FALSE;
	}
	return TRUE;
}

LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	PKBDLLHOOKSTRUCT ptrKBHOOK = reinterpret_cast<PKBDLLHOOKSTRUCT>(lParam);
	if (wParam == WM_KEYDOWN)
	{
		switch (ptrKBHOOK->vkCode)
		{
			case VK_F1:
			{
				OnKBEvent(GetAsyncKeyState(VK_SHIFT), PatchID::patch_weapon);
				break;
			}
			case VK_F2:
			{
				OnKBEvent(GetAsyncKeyState(VK_SHIFT), PatchID::patch_effect);
				break;
			}
			case VK_F3:
			{
				OnKBEvent(GetAsyncKeyState(VK_SHIFT), PatchID::patch_attack);
				break;
			}
			case VK_F4:
			{
				OnKBEvent(GetAsyncKeyState(VK_SHIFT), PatchID::patch_magic);
				break;
			}
			case VK_F5:
			{
				OnKBEvent(GetAsyncKeyState(VK_SHIFT), PatchID::patch_bullet);
				break;
			}
			case VK_F9:
			{
				OnKBEvent2(GetAsyncKeyState(VK_SHIFT), ActionID::patch_all);
				break;
			}
			case VK_F6:
			{
				OnKBEvent2(GetAsyncKeyState(VK_SHIFT), ActionID::patch_player);
				break;
			}
			case VK_F7:
			{
				OnKBEvent2(GetAsyncKeyState(VK_SHIFT), ActionID::toggle_ai);
				break;
			}
			case VK_F8:
			{
				OnKBEvent2(GetAsyncKeyState(VK_SHIFT), ActionID::toggle_gravity);
				break;
			}
			case VK_F10:
			{
				OnKBEvent2(GetAsyncKeyState(VK_SHIFT), ActionID::print_ds_struct);
				break;
			}
			case VK_F12:
			{
				OnKBEvent2(GetAsyncKeyState(VK_SHIFT), ActionID::exit);
				break;
			}
		}
	}
	return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

bool ScanSignatures()
{
	DWORD i, j;
	BYTE* AOBAddr = nullptr;

	for (i = 0; i < MODULELIST_SIZE; ++i)
	{
		std::cout << clr::white << "Scanning for signatures on module " << clr::yellow << ModuleList[i].sModuleName;
		std::cout << clr::white << '(' << clr::green << "0x" << std::hex << std::uppercase << reinterpret_cast<QWORD>(ModuleList[i].ModuleBaseAddr) << clr::white << ")..." << std::endl;

		ModuleList[i].aobModule = new BYTE[ModuleList[i].dwModuleSize];
		if (pProcMem->ReadProcMem(ModuleList[i].ModuleBaseAddr, ModuleList[i].aobModule, ModuleList[i].dwModuleSize) == false)
		{
			std::cout << clr::red << "\t|--> Error reading module's memory!" << std::endl;
			delete[] ModuleList[i].aobModule;
			ModuleList[i].aobModule = nullptr;
			return false;
		}
		for (j = 0; j < PATTERNLIST_SIZE; ++j)
		{
			if (PatternList[j].dwModuleID == i)
			{
				AOBAddr = pProcMem->AOBScan(ModuleList[i].aobModule, ModuleList[i].dwModuleSize, PatternList[j].aobPattern, PatternList[j].dwPatternSize);
				if (AOBAddr == nullptr)
				{
					std::cout << clr::red << "\t|--> AOB scan with pattern id " << j << " failed!" << std::endl;
					delete[] ModuleList[i].aobModule;
					ModuleList[i].aobModule = nullptr;
					return false;
				}
				PatternList[j].dwAddrOffset = AOB2DWORD(AOBAddr, PatternList[j].dwOffset) + PatternList[j].dwExtra;
				PatternList[j].Address = ModuleList[i].ModuleBaseAddr + (AOBAddr - ModuleList[i].aobModule);
				std::cout << clr::white << "\t|--> Signature " << clr::green << "0x" << std::hex << std::uppercase << reinterpret_cast<QWORD>(PatternList[j].Address) << clr::white << " found for pattern id " << j << '!' << std::endl;
			}
		}
		delete[] ModuleList[i].aobModule;
		ModuleList[i].aobModule = nullptr;
	}

	return true;
}

bool LoadModules()
{
	for (int i = 0; i < MODULELIST_SIZE; ++i)
	{
		if (pProcMem->IsProcessRunning() == false)
			return false;
		std::cout << clr::white << "Loading module " << clr::yellow << ModuleList[i].sModuleName << clr::white << " (" << (i + 1) << '/' << MODULELIST_SIZE << ')' << std::endl;
		if (pProcMem->LoadModule(ModuleList[i].sModuleName, ModuleList[i].ModuleBaseAddr, ModuleList[i].dwModuleSize) == false)
		{
			std::cout << clr::red << "Failed to load module. Retrying in 5 seconds..." << std::endl;
			--i;
			Sleep(FIVE_SECONDS);
		}
	}
	return true;
}

bool GetStrData(const BYTE* Addr, DataT DataType, BYTE BitOffset, std::string &sData)
{
	BYTE TmpData[4] = {};
	bool bTmp;
	try
	{
		switch (DataType)
		{
			case DataT::uint_8:
			{
				if (pProcMem->ReadProcMem(Addr, TmpData, sizeof(char)) == false)
					return false;
				sData = std::to_string(*reinterpret_cast<DWORD*>(TmpData));
				break;
			}
			case DataT::uint_16:
			{
				if (pProcMem->ReadProcMem(Addr, TmpData, sizeof(WORD)) == false)
					return false;
				sData = std::to_string(*reinterpret_cast<DWORD*>(TmpData));
				break;
			}
			case DataT::int_16:
			{
				if (pProcMem->ReadProcMem(Addr, TmpData, sizeof(short)) == false)
					return false;
				sData = std::to_string(*reinterpret_cast<long*>(TmpData));
				break;
			}
			case DataT::uint_32:
			{
				if (pProcMem->ReadProcMem(Addr, TmpData, sizeof(DWORD)) == false)
					return false;
				sData = std::to_string(*reinterpret_cast<DWORD*>(TmpData));
				break;
			}
			case DataT::int_32:
			{
				if (pProcMem->ReadProcMem(Addr, TmpData, sizeof(long)) == false)
					return false;
				sData = std::to_string(*reinterpret_cast<long*>(TmpData));
				break;
			}
			case DataT::float_32:
			{
				if (pProcMem->ReadProcMem(Addr, TmpData, sizeof(float)) == false)
					return false;
				sData = std::to_string(*reinterpret_cast<float*>(TmpData));
				break;
			}
			case DataT::bit:
			{
				if (pProcMem->ReadProcMem(Addr, TmpData, sizeof(bool)) == false)
					return false;
				bTmp = GETBIT(TmpData[0], BitOffset);
				sData = bTmp ? "true" : "false";
				break;
			}
			case DataT::bool_8:
			{
				if (pProcMem->ReadProcMem(Addr, TmpData, sizeof(bool)) == false)
					return false;
				break;
				bTmp = static_cast<bool>(TmpData[0]);
				sData = bTmp ? "true" : "false";
			}
			default:
			{
				return false;
			}
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}

void OnKBEvent(bool bShift, PatchID Patch)
{
	bool bFail = false;
	DWORD dwFileID;
	PPATCH_INF pPatchInf;

	switch (Patch)
	{
		case PatchID::patch_weapon:
			dwFileID = FileID::weapon;
			pPatchInf = &WeaponPatchInf;
			break;
		case PatchID::patch_effect:
			dwFileID = FileID::effect;
			pPatchInf = &EffectPatchInf;
			break;
		case PatchID::patch_attack:
			dwFileID = FileID::attack;
			pPatchInf = &AttackPatchInf;
			break;
		case PatchID::patch_magic:
			dwFileID = FileID::magic;
			pPatchInf = &MagicPatchInf;
			break;
		case PatchID::patch_bullet:
			dwFileID = FileID::bullet;
			pPatchInf = &BulletPatchInf;
			break;
		default:
			return;
	}

	if (bShift == false)
	{
		if (ParamUtil::PatchMemory(pPatchInf->PatchBackupList) && LoadPatchData(dwFileID) && ParamUtil::PatchMemoryWithBackup(*pPatchInf))
			SuccessBeep();
		else
		{
			FailBeep();
			bFail = true;
		}
	}
	else
	{
		if (!pPatchInf->PatchBackupList.empty() && ParamUtil::PatchMemory(pPatchInf->PatchBackupList))
			SuccessBeep2();
		else
		{
			FailBeep();
			bFail = true;
		}
	}
}

void OnKBEvent2(bool bShift, ActionID Action)
{
	bool bFail = false;

	switch (Action)
	{
		case ActionID::patch_all:
		{
			if (ParamUtil::PatchMemory(WeaponPatchInf.PatchBackupList) &&
				ParamUtil::PatchMemory(EffectPatchInf.PatchBackupList) &&
				ParamUtil::PatchMemory(AttackPatchInf.PatchBackupList) &&
				ParamUtil::PatchMemory(MagicPatchInf.PatchBackupList) &&
				ParamUtil::PatchMemory(BulletPatchInf.PatchBackupList) &&
				LoadPatchData(FileID::weapon) &&
				LoadPatchData(FileID::effect) &&
				LoadPatchData(FileID::attack) &&
				LoadPatchData(FileID::magic) &&
				LoadPatchData(FileID::bullet) &&
				ParamUtil::PatchMemoryWithBackup(WeaponPatchInf) &&
				ParamUtil::PatchMemoryWithBackup(EffectPatchInf) &&
				ParamUtil::PatchMemoryWithBackup(AttackPatchInf) &&
				ParamUtil::PatchMemoryWithBackup(MagicPatchInf) &&
				ParamUtil::PatchMemoryWithBackup(BulletPatchInf))
			{
				SuccessBeep();
			}
			else
			{
				FailBeep();
				bFail = true;
			}
			break;
		}
		case ActionID::patch_player:
		{
			if (bShift == false)
			{
				if (LoadPatchData(FileID::player) && ParamUtil::PatchMemory(PlayerData, BaseA, BaseB, BaseC, ParamList[ParamID::sp_effect_param]))
				{
					SuccessBeep();
				}
				else
				{
					FailBeep();
					bFail = true;
				}
			}
			else
			{
				if (ParamUtil::PatchMemory(PlayerDataBackup, BaseA, BaseB, BaseC, ParamList[ParamID::sp_effect_param]))
				{
					SuccessBeep2();
				}
				else
				{
					FailBeep();
					bFail = true;
				}
			}
			break;
		}
		case ActionID::print_ds_struct:
		{
			if (hKeyboardHook != nullptr)
				UnhookWindowsHookEx(hKeyboardHook);

			if (PrintDSStruct() == false)
				bFail = true;

			hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, nullptr, 0);
			break;
		}
		case ActionID::toggle_gravity:
		{
			if (bShift == false)
			{
				if (DSHack::ToggleGravity(BaseA))
					SuccessBeep();
				else
				{
					FailBeep();
					bFail = true;
				}
			}
			else
			{
				if (DSHack::IncrementZPos(BaseA, 1.25f))
					Beep(500, 25);
				else
				{
					FailBeep();
					bFail = true;
				}
			}
			break;
		}
		case ActionID::toggle_ai:
		{
			if (DSHack::ToggleAI(BaseB))
				SuccessBeep();
			else
			{
				FailBeep();
				bFail = true;
			}
			break;
		}
		case ActionID::exit:
		{
			if (bShift == false)
			{
				ParamUtil::PatchMemory(WeaponPatchInf.PatchBackupList);
				ParamUtil::PatchMemory(EffectPatchInf.PatchBackupList);
				ParamUtil::PatchMemory(AttackPatchInf.PatchBackupList);
				ParamUtil::PatchMemory(MagicPatchInf.PatchBackupList);
				ParamUtil::PatchMemory(BulletPatchInf.PatchBackupList);
				ParamUtil::PatchMemory(PlayerDataBackup, BaseA, BaseB, BaseC, ParamList[ParamID::sp_effect_param]);
				std::cout << clr::white << "All backup data restored." << std::endl;
			}
			std::cout << clr::white << "Exiting..." << std::endl;
			PostQuitMessage(0);
			break;
		}
	}

	if (bFail == true && pProcMem->IsProcessRunning() == false)
	{
		std::cout << clr::red << "Process is not running. Exiting..." << std::endl;
		PostQuitMessage(0);
	}
}

bool PrintDSStruct()
{
	DWORD dwStruct     = MAXDWORD;
	DWORD dwStructID   = 0;
	BYTE* Addr         = nullptr;
	PFIELD pFieldList  = nullptr;
	int i, iFieldCount = 0;
	size_t StrSize     = 0;
	size_t MaxStrSize  = 0;
	std::string sData;

	std::cout << clr::green << "1 " << clr::white << "- Weapon struct" << std::endl;
	std::cout << clr::green << "2 " << clr::white << "- Effect struct" << std::endl;
	std::cout << clr::green << "3 " << clr::white << "- Attack struct" << std::endl;
	std::cout << clr::green << "4 " << clr::white << "- Magic struct" << std::endl;
	std::cout << clr::green << "5 " << clr::white << "- Bullet struct" << std::endl;
	std::cout << clr::cyan << "Select a struct to read: " << clr::green;
	std::getline(std::cin, sData);

	try
	{
		dwStruct = std::stoul(sData, nullptr, 10);
	}
	catch (...)
	{
		std::cout << clr::red << "Invalid input." << std::endl << std::endl;
		return true;
	}

	switch (dwStruct)
	{
		case 1:
			dwStruct = ParamID::equip_param_weapon;
			pFieldList = WeaponFieldList;
			iFieldCount = WEAPON_FIELD_LIST_SIZE;
			break;
		case 2:
			dwStruct = ParamID::sp_effect_param;
			pFieldList = EffectFieldList;
			iFieldCount = EFFECT_FIELD_LIST_SIZE;
			break;
		case 3:
			dwStruct = ParamID::atk_param_pc;
			pFieldList = AttackFieldList;
			iFieldCount = ATTACK_FIELD_LIST_SIZE;
			break;
		case 4:
			dwStruct = ParamID::magic;
			pFieldList = MagicFieldList;
			iFieldCount = MAGIC_FIELD_LIST_SIZE;
			break;
		case 5:
			dwStruct = ParamID::bullet;
			pFieldList = BulletFieldList;
			iFieldCount = BULLET_FIELD_LIST_SIZE;
			break;
		default:
			std::cout << clr::red << "Invalid input." << std::endl << std::endl;
			return true;
	}

	std::cout << clr::cyan << "Struct ID: " << clr::green;
	sData.clear();
	std::getline(std::cin, sData);

	try
	{
		dwStructID = std::stoul(sData, nullptr, (sData.find("0x") == 0) ? 16 : 10);
	}
	catch (...)
	{
		std::cout << clr::red << "Invalid ID." << std::endl << std::endl;
		return true;
	}

	Addr = ParamUtil::GetIDAddr(ParamList[dwStruct], dwStructID);
	if (Addr == nullptr)
	{
		std::cout << clr::red << "Invalid ID." << std::endl << std::endl;
		return true;
	}

	MaxStrSize = std::strlen(pFieldList[1].sName);
	for (i = 2; i < iFieldCount; ++i)
	{
		StrSize = std::strlen(pFieldList[i].sName);
		if (StrSize > MaxStrSize)
			MaxStrSize = StrSize;
	}

	std::cout << std::endl << clr::white << '[' << clr::yellow << pFieldList[0].sName << clr::white << ':' << clr::green << sData << clr::white << ']' << std::endl;
	for (i = 1; i < iFieldCount; ++i)
	{
		if (GetStrData(Addr + pFieldList[i].dwAddrOffset, pFieldList[i].DataType, pFieldList[i].BitOffset, sData) == false)
			break;
		std::cout << ((i % 2 == 0) ? clr::pink : clr::cyan) << std::setw(MaxStrSize) << std::left << pFieldList[i].sName << clr::white << " = " << ((i % 2 == 0) ? clr::pink : clr::cyan) << sData << std::endl;
	}

	std::cout << std::endl;

	return true;
}

void PrintParamList()
{
	std::cout << clr::white << "Param list objects:" << std::endl;
	for (int i = 0; i < PARAMLIST_SIZE; ++i)
	{
		std::cout << clr::white << "\t|--> " << clr::green << "0x" << std::hex << std::uppercase << reinterpret_cast<QWORD>(ParamList[i].ParamAddr) << clr::white << ": " << clr::yellow << ParamList[i].sParamName << std::endl;
		std::cout << clr::white << "\t|    Object count: " << std::dec << ParamList[i].wObjCount << '.' << std::endl;
	}
}

void PrintParserError(const RET_INF &Inf)
{
	switch (Inf.RetID)
	{
		case ReturnID::end_of_file:
			std::cout << clr::red << "\t|--> Expecting a field on line: " << std::dec << Inf.dwLine << '.' << std::endl << std::endl;
			break;
		case ReturnID::read_error:
			std::cout << clr::red << "\t|--> Error reading the file." << std::endl << std::endl;
			break;
		case ReturnID::syntax_error:
			std::cout << clr::red << "\t|--> Syntax error on line: " << std::dec << Inf.dwLine << '.' << std::endl << std::endl;
			break;
		case ReturnID::invalid_field:
			std::cout << clr::red << "\t|--> Invalid field name on line: " << std::dec << Inf.dwLine << '.' << std::endl << std::endl;
			break;
		case ReturnID::invalid_data:
			std::cout << clr::red << "\t|--> Invalid data on line: " << std::dec << Inf.dwLine << '.' << std::endl << std::endl;
			break;
		case ReturnID::invalid_id:
			std::cout << clr::red << "\t|--> Invalid id on line: " << std::dec << Inf.dwLine << '.' << std::endl << std::endl;
			break;
		default:
			std::cout << clr::red << "\t|--> Unknown error." << std::endl << std::endl;
	}
}

void PrintInputOptions()
{
	std::cout << std::endl;
	std::cout << clr::cyan << "<    F1    >" << clr::white << " - Load modifications from "      << clr::cyan << FileList[FileID::weapon].sName << clr::white << '.' << std::endl;
	std::cout << clr::pink << "<SHIFT + F1>" << clr::white << " - Restore modifications from "   << clr::pink << FileList[FileID::weapon].sName << clr::white << '.' << std::endl;
	
	std::cout << clr::cyan << "<    F2    >" << clr::white << " - Load modifications from "      << clr::cyan << FileList[FileID::effect].sName << clr::white << '.' << std::endl;
	std::cout << clr::pink << "<SHIFT + F2>" << clr::white << " - Restore modifications from "   << clr::pink << FileList[FileID::effect].sName << clr::white << '.' << std::endl;
	
	std::cout << clr::cyan << "<    F3    >" << clr::white << " - Load modifications from "      << clr::cyan << FileList[FileID::attack].sName << clr::white << '.' << std::endl;
	std::cout << clr::pink << "<SHIFT + F3>" << clr::white << " - Restore modifications from "   << clr::pink << FileList[FileID::attack].sName << clr::white << '.' << std::endl;
																							     
	std::cout << clr::cyan << "<    F4    >" << clr::white << " - Load modifications from "      << clr::cyan << FileList[FileID::magic].sName << clr::white << '.' << std::endl;
	std::cout << clr::pink << "<SHIFT + F4>" << clr::white << " - Restore modifications from "   << clr::pink << FileList[FileID::magic].sName << clr::white << '.' << std::endl;
																							     
	std::cout << clr::cyan << "<    F5    >" << clr::white << " - Load modifications from "      << clr::cyan << FileList[FileID::bullet].sName << clr::white << '.' << std::endl;
	std::cout << clr::pink << "<SHIFT + F5>" << clr::white << " - Restore modifications from "   << clr::pink << FileList[FileID::bullet].sName << clr::white << '.' << std::endl;

	std::cout << clr::cyan << "<    F6    >" << clr::white << " - Load flags from "              << clr::cyan << FileList[FileID::player].sName << clr::white << '.' << std::endl;
	std::cout << clr::pink << "<SHIFT + F6>" << clr::white << " - Disable flags from "           << clr::pink << FileList[FileID::player].sName << clr::white << '.' << std::endl;
	
	std::cout << clr::cyan << "<    F7    >" << clr::white << " - Toggle AI."                        << std::endl;

	std::cout << clr::pink << "<    F8    >" << clr::white << " - Toggle gravity."                   << std::endl;
	std::cout << clr::cyan << "<SHIFT + F8>" << clr::white << " - Increment player Z pos."           << std::endl;
	
	std::cout << clr::pink << "<    F9    >" << clr::white << " - Load all modifications files."     << std::endl;

	std::cout << clr::cyan << "<   F10    >" << clr::white << " - Read struct data."                 << std::endl;
	
	std::cout << clr::pink << "<   F12    >" << clr::white << " - Restore all backup data and exit." << std::endl;
	std::cout << clr::cyan << "<SHIFT +F12>" << clr::white << " - Exit."                             << std::endl;
	
	std::cout << std::endl << clr::cyan << "Waiting for input..." << std::endl << std::endl;
}

bool LoadPatchData(DWORD dwFileID)
{
	DSMParser FileParser(dwFileID);
	RET_INF RetInf;

	switch (dwFileID)
	{
		// Struct
		case FileID::player:
			std::cout << clr::white << "Loading file " << clr::yellow << FileList[dwFileID].sName << clr::white << "..." << std::endl;
			RetInf = FileParser.GetStruct(reinterpret_cast<BYTE*>(&PlayerData));
			break;
		// Patch
		case FileID::weapon:
			std::cout << clr::white << "Loading file " << clr::yellow << FileList[dwFileID].sName << clr::white << "..." << std::endl;
			RetInf = FileParser.GetPatchDataList(ParamList[ParamID::equip_param_weapon], WeaponPatchInf.PatchList);
			break;
		case FileID::effect:
			std::cout << clr::white << "Loading file " << clr::yellow << FileList[dwFileID].sName << clr::white << "..." << std::endl;
			RetInf = FileParser.GetPatchDataList(ParamList[ParamID::sp_effect_param], EffectPatchInf.PatchList);
			break;
		case FileID::magic:
			std::cout << clr::white << "Loading file " << clr::yellow << FileList[dwFileID].sName << clr::white << "..." << std::endl;
			RetInf = FileParser.GetPatchDataList(ParamList[ParamID::magic], MagicPatchInf.PatchList);
			break;
		case FileID::attack:
			std::cout << clr::white << "Loading file " << clr::yellow << FileList[dwFileID].sName << clr::white << "..." << std::endl;
			RetInf = FileParser.GetPatchDataList(ParamList[ParamID::atk_param_pc], AttackPatchInf.PatchList);
			break;
		case FileID::bullet:
			std::cout << clr::white << "Loading file " << clr::yellow << FileList[dwFileID].sName << clr::white << "..." << std::endl;
			RetInf = FileParser.GetPatchDataList(ParamList[ParamID::bullet], BulletPatchInf.PatchList);
			break;
		default:
			return false;
	}

	if (RetInf.RetID == ReturnID::success)
		std::cout << clr::white << "\t|--> Modifications loaded." << std::endl << std::endl;
	else
	{
		PrintParserError(RetInf);
		return false;
	}

	return true;
}

void Cleanup()
{
	if (hKeyboardHook != nullptr)
		UnhookWindowsHookEx(hKeyboardHook);

	ParamUtil::DestroyParamList(ParamList, PARAMLIST_SIZE);

	SetConsoleTextAttribute(clr::hConsole, DefaultConsoleInfo.wAttributes == 0 ? DEFAULT_COLOR : DefaultConsoleInfo.wAttributes);
}

int main(int argc, char** argv)
{
	MSG Msg;

	if (SetConsoleCtrlHandler(static_cast<PHANDLER_ROUTINE>(ConsoleEventHandler), TRUE) == FALSE)
		return EXIT_FAILURE;

	clr::hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(clr::hConsole, &DefaultConsoleInfo);

	pProcMem = Memory::GetInstance();
	SetConsoleTitle("GitGud");

	std::cout << clr::white << "Loading " << clr::yellow << PROCESS_NAME << clr::white << "..." << std::endl;

	while (pProcMem->LoadProcess(PROCESS_NAME) == false)
	{
		std::cout << clr::red << "Process not found. Retrying in 5 seconds..." << std::endl;
		Sleep(FIVE_SECONDS);
	}

	if (LoadModules() == false)
	{
		std::cout << clr::red << "Process is not running. Exiting..." << std::endl;
		Cleanup();
		return EXIT_FAILURE;
	}

	bool bFlag = false;
	do
	{
		if (pProcMem->IsProcessRunning() == false)
		{
			std::cout << clr::red << "Process is not running. Exiting..." << std::endl;
			Cleanup();
			return EXIT_FAILURE;
		}
		if (ScanSignatures() == true)
		{
			ParamPatch = PatternList[SigID::param_patch].Address + PatternList[SigID::param_patch].dwAddrOffset;
			BaseA = PatternList[SigID::base_a].Address + PatternList[SigID::base_a].dwAddrOffset;
			BaseB = PatternList[SigID::base_b].Address + PatternList[SigID::base_b].dwAddrOffset;
			BaseC = PatternList[SigID::base_c].Address;
			if (ParamUtil::LoadParamList(ParamPatch, ParamList, PARAMLIST_SIZE) == true)
			{
				PrintParamList();
				bFlag = true;
			}
			else
			{
				std::cout << clr::red << "Failed to load the param list. Retrying in 5 seconds..." << std::endl;
				Sleep(FIVE_SECONDS);
			}
		}
		else
		{
			std::cout << clr::red << "Failed to load the signatures addresses. Retrying in 5 seconds..." << std::endl;
			Sleep(FIVE_SECONDS);
		}
	} while (bFlag == false);

	WeaponPatchInf.PatchList.reserve(VECTOR_START_CAP);
	EffectPatchInf.PatchList.reserve(VECTOR_START_CAP);

	PrintInputOptions();

	hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, nullptr, 0);

	while (GetMessage(&Msg, nullptr, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	Cleanup();

	return EXIT_SUCCESS;
}
