#include "ParamUtil.h"

bool ParamUtil::WC2MB(LPWCH wcString, int wcCount, LPCH mbString, int mbMaxSize)
{
	int size = WideCharToMultiByte(CP_UTF8, 0, wcString, wcCount, nullptr, 0, nullptr, nullptr);
	if (size <= 0 || size > mbMaxSize)
		return false;
	WideCharToMultiByte(CP_UTF8, 0, wcString, wcCount, mbString, size, nullptr, nullptr);
	mbString[size] = 0;
	return true;
}

BYTE* ParamUtil::ReadMultiLvlPtr(BYTE* BaseAddr, DWORD dwID)
{
	int i, j = MAX_OFFSET_COUNT - 1;
	PBYTE Addr = BaseAddr;
	Memory* pProcMem = Memory::GetInstance();

	if (pProcMem->ReadProcMem(Addr, Addr) == false)
		return NULL;

	for (i = 0; i < MAX_OFFSET_COUNT; ++i)
	{
		Addr += OffsetList[dwID][i];

		if (OffsetList[dwID][i + 1] == MAXDWORD || i == j)
			break;

		if (pProcMem->ReadProcMem(Addr, Addr) == false)
			return NULL;
	}

	return Addr;
}

bool ParamUtil::PatchBin(PatchData &BinPatch)
{
	Memory* pProcMem = Memory::GetInstance();
	PBIT pBit = reinterpret_cast<PBIT>(BinPatch.Data);
	BYTE byte;

	if (pProcMem->ReadProcMem(BinPatch.Address, byte) == false)
		return false;

	if (pBit->one)
		SETBIT(byte, pBit->bit);
	else
		CLBIT(byte, pBit->bit);

	if (pProcMem->WriteProcMem(BinPatch.Address, byte) == false)
		return false;

	return true;
}

bool ParamUtil::PatchMemory(const PLAYER_STRUCT &PlayerData, BYTE* BaseA, BYTE* BaseB, BYTE* BaseC, const PARAM_CLASS &EffectParam)
{
	BYTE byte;
	int int_32;
	BYTE* Addr;
	Memory* pProcMem = Memory::GetInstance();

	// Invisible
	Addr = ReadMultiLvlPtr(BaseA, OffsetID::invisible);
	if (Addr == 0)
		return false;

	byte = PlayerData.bInvisible ? 0 : 1;
	if (pProcMem->WriteProcMem(Addr, byte) == false)
		return false;

	// Super Armor
	Addr = ReadMultiLvlPtr(BaseA, OffsetID::super_armor);
	if (Addr == 0)
		return false;

	if (pProcMem->ReadProcMem(Addr, byte) == false)
		return false;

	if (PlayerData.bSuperArmor)
		SETBIT(byte, 0);
	else
		CLBIT(byte, 0);

	if (pProcMem->WriteProcMem(Addr, byte) == false)
		return false;

	// No Damage
	Addr = ReadMultiLvlPtr(BaseA, OffsetID::player_basic_status);
	if (Addr == 0)
		return false;

	if (pProcMem->ReadProcMem(Addr, byte) == false)
		return false;

	if (PlayerData.bNoDamage)
		SETBIT(byte, NODAMAGE_BIT_OFFSET);
	else
		CLBIT(byte, NODAMAGE_BIT_OFFSET);

	// No FP Cost
	if (PlayerData.bNoFPCost)
		SETBIT(byte, NOFPCOST_BIT_OFFSET);
	else
		CLBIT(byte, NOFPCOST_BIT_OFFSET);

	// No Stamina Consume
	if (PlayerData.bNoStaminaConsume)
		SETBIT(byte, NOSTAMINA_BIT_OFFSET);
	else
		CLBIT(byte, NOSTAMINA_BIT_OFFSET);

	if (pProcMem->WriteProcMem(Addr, byte) == false)
		return false;

	// No Goods Consume
	Addr = ReadMultiLvlPtr(BaseA, OffsetID::no_goods_consume);
	if (Addr == 0)
		return false;

	if (pProcMem->ReadProcMem(Addr, byte) == false)
		return false;

	if (PlayerData.bNoGoodsConsume)
		SETBIT(byte, NOGOODSCONSUME_BIT_OFFSET);
	else
		CLBIT(byte, NOGOODSCONSUME_BIT_OFFSET);

	if (pProcMem->WriteProcMem(Addr, byte) == false)
		return false;

	// No Arrows Consume
	byte = PlayerData.bNoArrowsConsume ? 1 : 0;
	if (pProcMem->WriteProcMem(BaseB + 0x4, byte) == false)
		return false;

	// Backflip Animation
	if (pProcMem->WriteProcMem(BaseC, const_cast<BYTE*>(PlayerData.bBackflipAnimation ? AOB_ROLL_PATCH : AOB_ROLL), sizeof(AOB_ROLL)) == false)
		return false;

	if (PlayerData.bBackflipAnimation)
	{
		Addr = ReadMultiLvlPtr(BaseA, OffsetID::backflip_animation);
		if (Addr == 0)
			return false;

		int_32 = 0;
		if (pProcMem->WriteProcMem(Addr, int_32) == false)
			return false;
	}

	int_32 = PlayerData.bEditNPCStat ? 0x00001B58 : 0xFFFFFFFF;
	for (int i = 0; i < NPCEFFECT_LIST_SIZE; ++i)
	{
		Addr = GetIDAddr(EffectParam, NPCEffectList[i]);
		if (pProcMem->WriteProcMem(Addr + EFFECT_ID_OFFSET, int_32) == false)
			return false;
	}

	return true;
}

bool ParamUtil::PatchMemory(std::vector<PatchData> &PatchList)
{
	Memory* pProcMem = Memory::GetInstance();

	for (auto &Patch : PatchList)
	{
		if (Patch.GetType() == DataT::bit)
		{
			if (PatchBin(Patch) == false)
				return false;
		}
		else
		{
			if (pProcMem->WriteProcMem(Patch.Address, Patch.Data, Patch.GetSize()) == false)
				return false;
		}
	}
	PatchList.clear();
	return true;
}

bool ParamUtil::PatchMemoryWithBackup(PATCH_INF &PatchInf)
{
	Memory* pProcMem = Memory::GetInstance();
	PatchData TempPatch;

	PatchInf.PatchBackupList.reserve(PatchInf.PatchList.size());

	for (auto &Patch : PatchInf.PatchList)
	{
		TempPatch = Patch;

		if (Patch.GetType() == DataT::bit)
		{
			if (PatchBin(Patch) == false)
				return false;
		}
		else
		{
			if (pProcMem->WriteProcMem(Patch.Address, Patch.Data, Patch.GetSize()) == false)
				return false;
		}

		if (pProcMem->ReadProcMem(TempPatch.Address, TempPatch.Data, TempPatch.GetSize()) == false)
			return false;

		PatchInf.PatchBackupList.push_back(TempPatch);
	}
	PatchInf.PatchList.clear();
	return true;
}

WORD ParamUtil::LoadObjList(PARAM_CLASS &ParamClass)
{
	Memory* pProcMem = Memory::GetInstance();
	WORD wObjCount = 0;
	BYTE* Addr = 0;
	int i;

	ParamClass.wObjCount = 0;
	if (ParamClass.ObjList != nullptr)
	{
		delete[] ParamClass.ObjList;
		ParamClass.ObjList = nullptr;
	}

	pProcMem->ReadProcMem(ParamClass.ParamAddr + 0x68, Addr);
	pProcMem->ReadProcMem(Addr + 0x68, Addr);
	pProcMem->ReadProcMem(Addr + 0xA, wObjCount);

	if (wObjCount > 0)
	{
		ParamClass.ObjList = new PARAM_OBJECT[wObjCount];
		ParamClass.wObjCount = wObjCount;

		for (i = 0; i < wObjCount; ++i)
		{
			pProcMem->ReadProcMem(Addr + 0x40 + (0x18 * i), ParamClass.ObjList[i].dwID);
			pProcMem->ReadProcMem(Addr + 0x48 + (0x18 * i), ParamClass.ObjList[i].dwOffset);
			ParamClass.ObjList[i].Address = Addr + ParamClass.ObjList[i].dwOffset;
		}
	}

	return wObjCount;
}

BYTE* ParamUtil::GetIDAddr(const PARAM_CLASS &ParamClass, DWORD dwID)
{
	if (ParamClass.wObjCount == 0)
		return nullptr;

	int l = 0;
	int m;
	int r = ParamClass.wObjCount - 1;

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

	return nullptr;
}

#define STR_PARAM_MAX 32
bool ParamUtil::LoadParamList(const BYTE* ParamPatch, PPARAM_CLASS ParamList, DWORD dwParamListSize)
{
	Memory* pProcMem               = Memory::GetInstance();
	BYTE* StartAddr                = nullptr;
	BYTE* EndAddr                  = nullptr;
	BYTE* ParamAddr                = nullptr;
	BYTE* ParamStrAddr             = nullptr;
	wchar_t wcParam[STR_PARAM_MAX] = {};
	char sParam[STR_PARAM_MAX]     = {};
	DWORD dwParamSize              = 0;
	DWORD dwCount                  = 0;
	DWORD i, j;

	if (pProcMem->ReadProcMem(ParamPatch, ParamPatch) == false)
		return false;

	ParamPatch += 0x10;
	if (pProcMem->ReadProcMem(ParamPatch, StartAddr) == false)
		return false;
	if (pProcMem->ReadProcMem(ParamPatch + 0x8, EndAddr) == false)
		return false;

	if (StartAddr != 0 && EndAddr != 0)
	{
		dwCount = static_cast<DWORD>(((EndAddr - StartAddr) / 8) - 1);
		for (i = 0; i < dwCount; ++i)
		{
			if (pProcMem->ReadProcMem(StartAddr + (i * 8), ParamAddr) == false)
				return false;
			if (pProcMem->ReadProcMem(ParamAddr + 0x20, dwParamSize) == false)
				return false;
			if (dwParamSize > STR_PARAM_MAX - 1)
				continue;
			if (dwParamSize > 7)
			{
				if (pProcMem->ReadProcMem(ParamAddr + 0x10, ParamStrAddr) == false)
					continue;
				if (pProcMem->ReadProcMem(ParamStrAddr, reinterpret_cast<BYTE*>(wcParam), (dwParamSize * 2)) == false)
					continue;
			}
			else
			{
				if (pProcMem->ReadProcMem(ParamAddr + 0x10, reinterpret_cast<BYTE*>(wcParam), (dwParamSize * 2)) == false)
					continue;
			}
			WC2MB(wcParam, dwParamSize, sParam, STR_PARAM_MAX);
			for (j = 0; j < dwParamListSize; ++j)
			{
				if (!strcmp(sParam, ParamList[j].sParamName))
				{
					ParamList[j].ParamAddr = ParamAddr;
					LoadObjList(ParamList[j]);
					break;
				}
			}
		}
	}

	for (i = 0; i < dwParamListSize; ++i)
	{
		if (ParamList[i].ParamAddr == nullptr)
			return false;
	}

	return true;
}

void ParamUtil::DestroyParamList(PPARAM_CLASS ParamList, DWORD dwParamListSize)
{
	DWORD i;
	for (i = 0; i < dwParamListSize; ++i)
	{
		if (ParamList[i].ObjList != nullptr)
		{
			delete[] ParamList[i].ObjList;
			ParamList[i].ObjList = nullptr;
			ParamList[i].wObjCount = 0;
		}
	}
}
