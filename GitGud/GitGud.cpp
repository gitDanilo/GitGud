#include <iostream>
#include <string>
#include "Color.h"
#include "Memory.h"
#include "DSMParser.h"
#include "DSWeapon.h"

///////////////////
//// Constants ////
///////////////////
#define FIVE_SECONDS 5000
#define DEFAULT_COLOR 7
#define PROCESS_NAME "DarkSoulsIII.exe"

/////////////////
//// Structs ////
/////////////////
typedef struct _MODULE_INFO
{
	char sModuleName[32];
	PBYTE ModuleBaseAddr;
	DWORD dwModuleSize;
	PBYTE aobModule;
} MODULE_INFO, *PMODULE_INFO;

typedef struct _AOB_PATTERN
{
	DWORD dwModuleID;
	DWORD dwOffset;
	DWORD dwExtra;
	DWORD dwAddrOffset;
	PBYTE Address;
	PBYTE aobPattern;
	DWORD dwPatternSize;
} AOB_PATTERN, *PAOB_PATTERN;

typedef struct _PARAM_OBJECT
{
	DWORD dwID;
	DWORD dwOffset;
	PBYTE Address;
} PARAM_OBJECT, *PPARAM_OBJECT;

typedef struct _PARAM_CLASS
{
	char sParamName[32];
	PBYTE ParamAddr;
	WORD wObjCount;
	PPARAM_OBJECT ObjList;
} PARAM_CLASS, *PPARAM_CLASS;

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
	{"DarkSoulsIII.exe", 0, 0, NULL}
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
	{ModuleID::base, 3, 7, 0, 0, const_cast<PBYTE>(AOB_PARAM_PATCH), sizeof(AOB_PARAM_PATCH)},
	{ModuleID::base, 3, 7, 0, 0, const_cast<PBYTE>(AOB_BASE_A)     , sizeof(AOB_BASE_A)},
	{ModuleID::base, 3, 7, 0, 0, const_cast<PBYTE>(AOB_BASE_B)     , sizeof(AOB_BASE_B)},
	{ModuleID::base, 0, 0, 0, 0, const_cast<PBYTE>(AOB_BASE_C)     , sizeof(AOB_BASE_C)}
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

#define PARAMLIST_SIZE 15
static PARAM_CLASS ParamList[PARAMLIST_SIZE] =
{
	{"EquipParamWeapon"    , 0, 0, NULL},
	{"EquipParamProtector" , 0, 0, NULL},
	{"EquipParamGoods"     , 0, 0, NULL},
	{"EquipParamAccessory" , 0, 0, NULL},
	{"SpEffectParam"       , 0, 0, NULL},
	{"ThrowParam"          , 0, 0, NULL},
	{"BehaviorParam_PC"    , 0, 0, NULL},
	{"AtkParam_Pc"         , 0, 0, NULL},
	{"SpEffectVfxParam"    , 0, 0, NULL},
	{"ObjectParam"         , 0, 0, NULL},
	{"BonfireWarpParam"    , 0, 0, NULL},
	{"HitEffectSfxParam"   , 0, 0, NULL},
	{"ShopLineupParam"     , 0, 0, NULL},
	{"ItemLotParam"        , 0, 0, NULL},
	{"SwordArtsParam"      , 0, 0, NULL}
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
		sword_arts_param
	};
}

namespace ActionID
{
	enum ID: DWORD
	{
		toggle_gravity, toggle_ai
	};
}

//////////////////////////
//// Global variables ////
//////////////////////////
HHOOK hKeyboardHook = 0;
Memory* pProcMem = NULL;
CONSOLE_SCREEN_BUFFER_INFO DefaultConsoleInfo = {};
DSPlayer* pDSPlayer = NULL;
DSWeapon* pDSWeapon = NULL;
DSEffect* pDSEffect = NULL;
PBYTE ParamPatch = 0;
PBYTE BaseA = 0;
PBYTE BaseB = 0;
PBYTE BaseC = 0;

/////////////////////////////
//// Methods declaration ////
/////////////////////////////
bool WC2MB(LPWCH wcString, int wcCount, LPCH mbString, int mbMaxSize);
BOOL WINAPI ConsoleEventHandler(DWORD dwEvent);
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);
bool ScanSignatures();
WORD LoadObjList(PARAM_CLASS &ParamClass);
PBYTE GetIDAddr(const PARAM_CLASS &ParamClass, DWORD dwID);
bool LoadParamList();
void OnKBEvent(bool bShift, BYTE bFileID);
void OnKBEvent2(bool bShift, BYTE bActionID);
bool LoadFile(DWORD dwFileID);
void PrintParserError(RET_INF &Inf);
bool OverwriteData(DWORD dwFileID);
bool RestoreData(DWORD dwFileID);
void InitDSClasses();
void Cleanup();
bool ToggleGravity();
bool ToggleAI();
bool IncrementZPos(float fValue);
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

bool ToggleAI()
{
	BYTE byte;
	PBYTE Addr = BaseB + 0x9 + 0x4;

	if (pProcMem->ReadProcMem(Addr, byte) == false)
		return false;

	byte = (byte ? 0 : 1);

	if (pProcMem->WriteProcMem(Addr, byte) == false)
		return false;

	return true;
}

bool ToggleGravity()
{
	BYTE byte;
	PBYTE Addr = BaseA;

	if (pProcMem->ReadProcMem(Addr, Addr) == false)
		return false;

	Addr += 0x80;

	if (pProcMem->ReadProcMem(Addr, Addr) == false)
		return false;

	Addr += 0x1A08;

	if (pProcMem->ReadProcMem(Addr, byte) == false)
		return false;

	if (GETBIT(byte, 6))
		CLBIT(byte, 6);
	else
		SETBIT(byte, 6);

	if (pProcMem->WriteProcMem(Addr, byte) == false)
		return false;

	return true;
}

bool IncrementZPos(float fValue)
{
	PBYTE Addr = BaseA;
	float fBaseValue = 0;

	if (pProcMem->ReadProcMem(Addr, Addr) == false)
		return false;

	Addr += 0x40;

	if (pProcMem->ReadProcMem(Addr, Addr) == false)
		return false;

	Addr += 0x28;

	if (pProcMem->ReadProcMem(Addr, Addr) == false)
		return false;

	Addr += 0x84;

	if (pProcMem->ReadProcMem(Addr, fBaseValue) == false)
		return false;

	fBaseValue += fValue;

	if (pProcMem->WriteProcMem(Addr, fBaseValue) == false)
		return false;

	return true;
}

bool WC2MB(LPWCH wcString, int wcCount, LPCH mbString, int mbMaxSize)
{
	int size = WideCharToMultiByte(CP_UTF8, 0, wcString, wcCount, NULL, 0, NULL, NULL);
	if (size <= 0 || size > mbMaxSize)
		return false;
	WideCharToMultiByte(CP_UTF8, 0, wcString, wcCount, mbString, size, NULL, NULL);
	mbString[size] = 0;
	return true;
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
				OnKBEvent(GetAsyncKeyState(VK_SHIFT), FileID::player);
				break;
			}
			case VK_F2:
			{
				OnKBEvent(GetAsyncKeyState(VK_SHIFT), FileID::weapon);
				break;
			}
			case VK_F3:
			{
				OnKBEvent(GetAsyncKeyState(VK_SHIFT), FileID::effect);
				break;
			}
			case VK_F4:
			{
				OnKBEvent2(GetAsyncKeyState(VK_SHIFT), ActionID::toggle_gravity);
				break;
			}
			case VK_F5:
			{
				OnKBEvent2(GetAsyncKeyState(VK_SHIFT), ActionID::toggle_ai);
				break;
			}
			case VK_F12:
			{
				std::cout << clr::white << "Exiting..." << std::endl;
				PostQuitMessage(0);
				break;
			}
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

bool ScanSignatures()
{
	PBYTE ptrAOB = 0;
	int i, j;

	for (i = 0; i < MODULELIST_SIZE; ++i)
	{
		std::cout << clr::white << "Scanning for signatures on module " << clr::yellow << ModuleList[i].sModuleName;
		std::cout << clr::white << '(' << clr::green << "0x" << std::hex << std::uppercase << ModuleList[i].ModuleBaseAddr << clr::white << ")..." << std::endl;

		ModuleList[i].aobModule = new BYTE[ModuleList[i].dwModuleSize];
		if (pProcMem->ReadProcMem(ModuleList[i].ModuleBaseAddr, ModuleList[i].aobModule, ModuleList[i].dwModuleSize) == false)
		{
			std::cout << clr::red << "\t|--> Error reading module's memory!" << std::endl;
			delete[] ModuleList[i].aobModule;
			ModuleList[i].aobModule = NULL;
			return false;
		}
		for (j = 0; j < PATTERNLIST_SIZE; ++j)
		{
			if (PatternList[j].dwModuleID == i)
			{
				ptrAOB = pProcMem->AOBScan(ModuleList[i].aobModule, ModuleList[i].dwModuleSize, PatternList[j].aobPattern, PatternList[j].dwPatternSize);
				if (ptrAOB == NULL)
				{
					std::cout << clr::red << "\t|--> AOB scan with pattern id " << j << " failed!" << std::endl;
					delete[] ModuleList[i].aobModule;
					ModuleList[i].aobModule = NULL;
					return false;
				}
				PatternList[j].dwAddrOffset = AOB2DWORD(ptrAOB, PatternList[j].dwOffset) + PatternList[j].dwExtra;
				PatternList[j].Address = ModuleList[i].ModuleBaseAddr + (ptrAOB - ModuleList[i].aobModule);
				std::cout << clr::white << "\t|--> Signature " << clr::green << "0x" << std::hex << std::uppercase << PatternList[j].Address << clr::white << " found for pattern id " << j << '!' << std::endl;
			}
		}
		delete[] ModuleList[i].aobModule;
		ModuleList[i].aobModule = NULL;
	}

	return true;
}

WORD LoadObjList(PARAM_CLASS &ParamClass)
{
	WORD i;
	WORD wObjCount = 0;
	PBYTE Addr = 0;
	
	ParamClass.wObjCount = 0;
	if (ParamClass.ObjList != NULL)
	{
		delete[] ParamClass.ObjList;
		ParamClass.ObjList = NULL;
	}

	pProcMem->ReadProcMem(ParamClass.ParamAddr + 0x68, Addr);
	pProcMem->ReadProcMem(Addr + 0x68, Addr);
	pProcMem->ReadProcMem(Addr + 0xA, wObjCount);

	if (wObjCount > 0)
	{
		ParamClass.wObjCount = wObjCount;
		ParamClass.ObjList = new PARAM_OBJECT[wObjCount];

		for (i = 0; i < wObjCount; ++i)
		{
			pProcMem->ReadProcMem(Addr + 0x40 + (0x18 * i), ParamClass.ObjList[i].dwID);
			pProcMem->ReadProcMem(Addr + 0x48 + (0x18 * i), ParamClass.ObjList[i].dwOffset);
			ParamClass.ObjList[i].Address = Addr + ParamClass.ObjList[i].dwOffset;
		}
	}

	return wObjCount;
}

PBYTE GetIDAddr(const PARAM_CLASS &ParamClass, DWORD dwID)
{
	if (ParamClass.wObjCount == 0)
		return 0;

	DWORD l = 0;
	DWORD m;
	DWORD r = ParamClass.wObjCount - 1;

	while (l <= r)
	{
		m = (l + r) / 2;
		if (ParamClass.ObjList[m].dwID == dwID)
			return ParamClass.ObjList[m].Address;
		if (ParamClass.ObjList[m].dwID < dwID)
			l = m + 1;
		else
			r = m - 1;
	}

	return 0;
}

bool LoadParamList()
{
	DWORD i, j;
	PBYTE Start = 0;
	PBYTE End = 0;
	DWORD dwCount = 0;
	PBYTE ParamAddr = 0;
	wchar_t wcParam[32] = {0};
	char sParam[32] = {0};
	PBYTE ParamStrAddr = 0;
	DWORD dwParamCount = 0;

	std::cout << clr::white << "Loading param list..." << std::endl;

	ParamPatch = PatternList[SigID::param_patch].Address + PatternList[SigID::param_patch].dwAddrOffset;

	if (pProcMem->ReadProcMem(ParamPatch, ParamPatch) == false)
		return false;
	
	ParamPatch += 0x10;
	if (pProcMem->ReadProcMem(ParamPatch, Start) == false)
		return false;
	if (pProcMem->ReadProcMem(ParamPatch + 0x8, End) == false)
		return false;

	if (Start != 0 && End != 0)
	{
		dwCount = static_cast<DWORD>((End - Start) / 8);
		for (i = 0; i < dwCount; ++i)
		{
			if (pProcMem->ReadProcMem(Start + (i * 8), ParamAddr) == false)
				return false;
			if (pProcMem->ReadProcMem(ParamAddr + 0x20, dwParamCount) == false)
				return false;
			if (dwParamCount > 31)
				continue;
			if (pProcMem->ReadProcMem(ParamAddr + 0x10, ParamStrAddr) == false)
				continue;
			if (pProcMem->ReadProcMem(ParamStrAddr, reinterpret_cast<PBYTE>(wcParam), (dwParamCount * 2)) == false)
				continue;
			WC2MB(wcParam, dwParamCount, sParam, 32);
			for (j = 0; j < PARAMLIST_SIZE; ++j)
			{
				if (!strcmp(sParam, ParamList[j].sParamName))
				{
					ParamList[j].ParamAddr = ParamAddr;
					std::cout << clr::green << "0x" << std::hex << std::uppercase << ParamAddr << clr::white << ": " << clr::yellow << sParam << std::endl;
					std::cout << clr::white << "\t|--> " << std::dec << LoadObjList(ParamList[j]) << " object(s) loaded!" << std::endl;
					break;
				}
			}
		}
	}
	
	for (i = 0; i < PARAMLIST_SIZE; ++i)
	{
		if (ParamList[i].ParamAddr == NULL)
			return false;
	}

	return true;
}

void OnKBEvent(bool bShift, BYTE bFileID)
{
	if (bShift)
	{
		if (RestoreData(bFileID) == true)
			SuccessBeep2();
		else
		{
			if (pProcMem->IsProcessRunning() == false)
			{
				std::cout << clr::red << "Process is not running. Exiting..." << std::endl;
				PostQuitMessage(0);
			}
			else
				FailBeep();
		}
	}
	else
	{
		RestoreData(bFileID);
		if (LoadFile(bFileID) == true && OverwriteData(bFileID) == true)
			SuccessBeep();
		else
		{
			if (pProcMem->IsProcessRunning() == false)
			{
				std::cout << clr::red << "Process is not running. Exiting..." << std::endl;
				PostQuitMessage(0);
			}
			else
				FailBeep();
		}
	}
}

void OnKBEvent2(bool bShift, BYTE bActionID)
{
	switch (bActionID)
	{
		case ActionID::toggle_ai:
		{
			if (ToggleAI())
				SuccessBeep();
			else
			{
				if (pProcMem->IsProcessRunning() == false)
				{
					std::cout << clr::red << "Process is not running. Exiting..." << std::endl;
					PostQuitMessage(0);
				}
				else
					FailBeep();
			}
			break;
		}
		case ActionID::toggle_gravity:
		{
			if (bShift)
			{
				if (IncrementZPos(1.0f))
					Beep(500, 50);
				else
				{
					if (pProcMem->IsProcessRunning() == false)
					{
						std::cout << clr::red << "Process is not running. Exiting..." << std::endl;
						PostQuitMessage(0);
					}
					else
						FailBeep();
				}
			}
			else
			{
				if (ToggleGravity())
					SuccessBeep();
				else
				{
					if (pProcMem->IsProcessRunning() == false)
					{
						std::cout << clr::red << "Process is not running. Exiting..." << std::endl;
						PostQuitMessage(0);
					}
					else
						FailBeep();
				}
			}
			break;
		}
	}
}

void PrintParserError(RET_INF &Inf)
{
	switch (Inf.dwRetID)
	{
		case ReturnID::end_of_file:
			std::cout << clr::red << "\t|--> File is missing fields." << std::endl;
			break;
		case ReturnID::read_error:
			std::cout << clr::red << "\t|--> Error reading the file." << std::endl;
			break;
		case ReturnID::syntax_error:
			std::cout << clr::red << "\t|--> Syntax error on line: " << std::dec << Inf.dwLine << '.' << std::endl;
			break;
		case ReturnID::invalid_field:
			std::cout << clr::red << "\t|--> Invalid field name on line: " << std::dec << Inf.dwLine << '.' << std::endl;
			break;
		case ReturnID::invalid_data:
			std::cout << clr::red << "\t|--> Invalid data on line: " << std::dec << Inf.dwLine << '.' << std::endl;
			break;
		default:
			std::cout << clr::red << "\t|--> Unknown error." << std::endl;
	}
}

bool LoadFile(DWORD dwFileID)
{
	DSMParser FileParser(dwFileID);
	PPARAM_CLASS pParam;
	TEMP_STRUCT tmpData;
	RET_INF Inf;
	DWORD dwID;
	PBYTE Addr;
	int i;
	
	switch (dwFileID)
	{
		case FileID::player:
		{
			std::cout << clr::white << "Loading file " << clr::yellow << DSM_PLAYER_FILE << clr::white << "..." << std::endl;
			tmpData.init(sizeof(PLAYER_STRUCT));
			pParam = NULL;
			if (pPlayer)
			{
				delete[] pPlayer;
				pPlayer = NULL;
			}
			if (pPlayerBackup)
			{
				delete[] pPlayerBackup;
				pPlayerBackup = NULL;
			}
			break;
		}
		case FileID::weapon:
		{
			std::cout << clr::white << "Loading file " << clr::yellow << DSM_WEAPON_FILE << clr::white << "..." << std::endl;
			tmpData.init(sizeof(WEAPON_STRUCT));
			pParam = &ParamList[ParamID::equip_param_weapon];
			for (i = 0; i < WeaponList.size(); ++i)
				delete[] WeaponList[i];
			WeaponList.clear();
			for (i = 0; i < WeaponListBackup.size(); ++i)
				delete[] WeaponListBackup[i];
			WeaponListBackup.clear();
			break;
		}
		case FileID::effect:
		{
			std::cout << clr::white << "Loading file " << clr::yellow << DSM_EFFECT_FILE << clr::white << "..." << std::endl;
			tmpData.init(sizeof(EFFECT_STRUCT));
			pParam = &ParamList[ParamID::sp_effect_param];
			for (i = 0; i < EffectList.size(); ++i)
				delete[] EffectList[i];
			EffectList.clear();
			for (i = 0; i < EffectListBackup.size(); ++i)
				delete[] EffectListBackup[i];
			EffectListBackup.clear();
			break;
		}
		default:
		{
			return false;
		}
	}

	Inf = FileParser.GetFirstDataStruct(tmpData.ptrData);
	if (Inf.dwRetID != ReturnID::success)
	{
		PrintParserError(Inf);
		return false;
	}

	do
	{
		dwID = (*reinterpret_cast<DWORD*>(tmpData.ptrData));
		if (dwID == MAXDWORD)
			std::cout << clr::white << "\t|--> Modifications loaded!" << std::endl;
		else
		{
			Addr = (dwID == 0 ? 0 : GetIDAddr(*pParam, dwID));
			if (Addr == 0)
			{
				std::cout << clr::red << "\t|--> Invalid id: " << clr::green << "0x" << std::hex << std::uppercase << dwID << clr::red << '.' << std::endl;
				return false;
			}
			std::cout << clr::white << "\t|--> Modifications loaded for: " << clr::green << "0x" << std::hex << std::uppercase << dwID << clr::white << '!' << std::endl;
		}

		switch (dwFileID)
		{
			case FileID::player:
			{
				POINTER_STRUCT PtrData;
				PtrData.BaseA = BaseA;
				PtrData.BaseB = BaseB;
				PtrData.BaseC = BaseC;
				pPlayer = new DSPlayer(pProcMem, reinterpret_cast<PPLAYER_STRUCT>(tmpData.ptrData), &PtrData);
				pPlayerBackup = new DSPlayer(pProcMem, NULL, &PtrData);
				pPlayerBackup->ReadMemory();
				//break;
				return true;
			}
			case FileID::weapon:
			{
				WeaponList.push_back(new DSWeapon(Addr, pProcMem, reinterpret_cast<PWEAPON_STRUCT>(tmpData.ptrData)));
				WeaponListBackup.push_back(new DSWeapon(Addr, pProcMem));
				WeaponListBackup.back()->SetID(dwID);
				WeaponListBackup.back()->ReadMemory();
				break;
			}
			case FileID::effect:
			{
				EffectList.push_back(new DSEffect(Addr, pProcMem, reinterpret_cast<PEFFECT_STRUCT>(tmpData.ptrData)));
				EffectListBackup.push_back(new DSEffect(Addr, pProcMem));
				EffectListBackup.back()->SetID(dwID);
				EffectListBackup.back()->ReadMemory();
				break;
			}
			default:
			{
				return false;
			}
		}

		Inf = FileParser.GetNextDataStruct(tmpData.ptrData);
	} while (Inf.dwRetID == ReturnID::success);
	
	if (Inf.dwRetID != ReturnID::end_of_file)
	{
		PrintParserError(Inf);
		return false;
	}

	return true;
}

bool OverwriteData(DWORD dwFileID)
{
	int i;
	switch (dwFileID)
	{
		case FileID::player:
		{
			if (pPlayer == NULL)
				return false;

			if (pPlayer->OverwriteMemory() == false)
				return false;
			break;
		}
		case FileID::weapon:
		{
			if (WeaponList.empty())
				return false;

			for (i = 0; i < WeaponList.size(); ++i)
			{
				if (WeaponList[i]->ReadMemory() == false)
					return false;
			}
			break;
		}
		case FileID::effect:
		{
			if (EffectList.empty())
				return false;

			for (i = 0; i < EffectList.size(); ++i)
			{
				if (EffectList[i]->ReadMemory() == false)
					return false;
			}
			break;
		}
		default:
		{
			return false;
		}
	}

	return true;
}

bool RestoreData(DWORD dwFileID)
{
	int i;
	switch (dwFileID)
	{
		case FileID::player:
		{
			if (pPlayerBackup == NULL)
				return false;

			if (pPlayerBackup->ReadMemory() == false)
				return false;
			break;
		}
		case FileID::weapon:
		{
			if (WeaponListBackup.empty())
				return false;

			for (i = 0; i < WeaponListBackup.size(); i++)
			{
				if (WeaponListBackup[i]->ReadMemory() == false)
					return false;
			}
			break;
		}
		case FileID::effect:
		{
			if (EffectListBackup.empty())
				return false;

			for (i = 0; i < EffectListBackup.size(); i++)
			{
				if (EffectListBackup[i]->ReadMemory() == false)
					return false;
			}
			break;
		}
		default:
		{
			return false;
		}
	}

	return true;
}

void InitDSClasses()
{
	POINTER_STRUCT PtrStruct;
	PtrStruct.BaseA = BaseA;
	PtrStruct.BaseB = BaseB;
	PtrStruct.BaseC = BaseC;

	if (pDSPlayer == NULL)
		pDSPlayer = new DSPlayer(pProcMem, PtrStruct);
	if (pDSWeapon == NULL)
		pDSWeapon = new DSWeapon(pProcMem);
	if (pDSEffect == NULL)
		pDSEffect = new DSEffect(pProcMem);
}

void Cleanup()
{
	int i;

	if (hKeyboardHook != 0)
		UnhookWindowsHookEx(hKeyboardHook);

	for (i = 0; i < PARAMLIST_SIZE; ++i)
	{
		if (ParamList[i].ObjList != NULL)
		{
			delete[] ParamList[i].ObjList;
			ParamList[i].ObjList = NULL;
			ParamList[i].wObjCount = 0;
		}
	}

	if (pDSPlayer != NULL)
	{
		delete[] pDSPlayer;
		pDSPlayer = NULL;
	}
	if (pDSWeapon != NULL)
	{
		delete[] pDSWeapon;
		pDSWeapon = NULL;
	}
	if (pDSEffect != NULL)
	{
		delete[] pDSEffect;
		pDSEffect = NULL;
	}

	SetConsoleTextAttribute(clr::hConsole, DefaultConsoleInfo.wAttributes == 0 ? DEFAULT_COLOR : DefaultConsoleInfo.wAttributes);
}

int main(int argc, char** argv)
{
	MSG Msg;
	int i;

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

	for (i = 0; i < MODULELIST_SIZE; ++i)
	{
		if (pProcMem->IsProcessRunning() == false)
		{
			std::cout << clr::red << "Process is not running. Exiting..." << std::endl;
			Cleanup();
			return EXIT_FAILURE;
		}
		std::cout << clr::white << "Loading module " << clr::yellow << ModuleList[i].sModuleName << clr::white << " (" << (i + 1) << '/' << MODULELIST_SIZE << ')' << std::endl;
		if (pProcMem->LoadModule(ModuleList[i].sModuleName, ModuleList[i].ModuleBaseAddr, ModuleList[i].dwModuleSize) == false)
		{
			std::cout << clr::red << "Failed to load module. Retrying in 5 seconds..." << std::endl;
			--i;
			Sleep(FIVE_SECONDS);
		}
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
			BaseA = PatternList[SigID::base_a].Address + PatternList[SigID::base_a].dwAddrOffset;
			BaseB = PatternList[SigID::base_b].Address + PatternList[SigID::base_b].dwAddrOffset;
			BaseC = PatternList[SigID::base_c].Address;
			if (LoadParamList() == false)
				std::cout << clr::red << "Failed to load the param list. Retrying in 5 seconds..." << std::endl;
			else
				bFlag = true;
		}
		else
			std::cout << clr::red << "Failed to load the signatures addresses. Retrying in 5 seconds..." << std::endl;
		if (bFlag == false)
			Sleep(FIVE_SECONDS);
	} while (bFlag == false);

	InitDSClasses();

	std::cout << std::endl << clr::pink << 'N' << clr::cyan << 'i' << clr::green << 'c' << clr::yellow << 'e' << clr::white << '!' << std::endl << std::endl;
	std::cout << clr::white << '<' << clr::cyan << "    F1    " << clr::white << "> - Load modifications from " << clr::cyan << DSM_PLAYER_FILE << clr::white << '.' << std::endl;
	std::cout << clr::white << '<' << clr::pink << "SHIFT + F1" << clr::white << "> - Restore modifications from " << clr::pink << DSM_PLAYER_FILE << clr::white << '.' << std::endl;
	std::cout << clr::white << '<' << clr::cyan << "    F2    " << clr::white << "> - Load modifications from " << clr::cyan << DSM_WEAPON_FILE << clr::white << '.' << std::endl;
	std::cout << clr::white << '<' << clr::pink << "SHIFT + F2" << clr::white << "> - Restore modifications from " << clr::pink << DSM_WEAPON_FILE << clr::white << '.' << std::endl;
	std::cout << clr::white << '<' << clr::cyan << "    F3    " << clr::white << "> - Load modifications from " << clr::cyan << DSM_EFFECT_FILE << clr::white << '.' << std::endl;
	std::cout << clr::white << '<' << clr::pink << "SHIFT + F3" << clr::white << "> - Restore modifications from " << clr::pink << DSM_EFFECT_FILE << clr::white << '.' << std::endl;
	std::cout << clr::white << '<' << clr::cyan << "    F4    " << clr::white << "> - Toggle gravity."  << std::endl;
	std::cout << clr::white << '<' << clr::pink << "SHIFT + F4" << clr::white << "> - Increment player Z pos." << std::endl;
	std::cout << clr::white << '<' << clr::cyan << "    F5    " << clr::white << "> - Toggle AI." << std::endl;
	std::cout << clr::white << '<' << clr::pink << "   F12    " << clr::white << "> - Exit." << std::endl;
	std::cout << clr::white << "Waiting for input..." << std::endl << std::endl;

	hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, NULL, 0);

	while (GetMessage(&Msg, NULL, 0, 0) > 0) // PeekMessage(&Msg, NULL, 0, 0, PREMOVE);
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	Cleanup();

	return EXIT_SUCCESS;
}
