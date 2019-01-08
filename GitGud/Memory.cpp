#include "Memory.h"

Memory::Memory()
{

}

Memory* Memory::GetInstance()
{
	static Memory mInstance;
	return &mInstance;
}

Memory::~Memory()
{
	if (hProcess != nullptr)
		CloseHandle(hProcess);
}

bool Memory::LoadProcess(const char* sProcessName)
{
	PROCESSENTRY32 PE32;
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	hProcess = nullptr;
	dwProcessID = 0;
	PE32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hProcessSnap, &PE32))
	{
		CloseHandle(hProcessSnap);
		return false;
	}

	do
	{
		if (!strcmp(PE32.szExeFile, sProcessName))
		{
			dwProcessID = PE32.th32ProcessID;
			hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, dwProcessID);
			break;
		}
	} while (Process32Next(hProcessSnap, &PE32));

	CloseHandle(hProcessSnap);
	return (hProcess != 0);
}

inline bool CompareByteArray(const BYTE* ByteArray1, const BYTE* ByteArray2, DWORD dwLength)
{
	for (DWORD i = 0; i < dwLength; ++i)
	{
		if (ByteArray2[i] != 0xCC && ByteArray1[i] != ByteArray2[i])
		{
			return false;
		}
	}
	return true;
}

BYTE* Memory::AOBScan(const BYTE* AOB, DWORD dwImageSize, const BYTE* TargetAOB, DWORD dwLength)
{
	BYTE* Addr = nullptr;
	DWORD dwFinalAddress = dwImageSize - dwLength;
	for (DWORD i = 0; i < dwFinalAddress; ++i)
	{
		if (CompareByteArray((AOB + i), TargetAOB, dwLength))
		{
			Addr = (PBYTE)AOB + i;
			break;
		}
	}
	return Addr;
}
//byte* FindPattern(byte* pBaseAddress, byte* pbMask, const char* pszMask, size_t nLength)
//{
//	auto DataCompare = [](const byte* pData, const byte* mask, const char* cmask, byte chLast, size_t iEnd)->bool
//	{
//		if (pData[iEnd] != chLast)
//			return false;
//		for (; *cmask; ++cmask, ++pData, ++mask)
//		{
//			if (*cmask == 'x' && *pData != *mask)
//			{
//				return false;
//			}
//		}
//
//		return true;
//	};
//
//	auto iEnd = strlen(pszMask) - 1;
//	auto chLast = pbMask[iEnd];
//
//	auto* pEnd = pBaseAddress + nLength - strlen(pszMask);
//	for (; pBaseAddress < pEnd; ++pBaseAddress)
//	{
//		if (DataCompare(pBaseAddress, pbMask, pszMask, chLast, iEnd))
//		{
//			return pBaseAddress;
//		}
//	}
//
//	return nullptr;
//}

bool Memory::LoadModule(const char* sModuleName, BYTE* &ModuleBaseAddr, DWORD &dwModuleSize)
{
	MODULEENTRY32 ME32;
	HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dwProcessID);

	ModuleBaseAddr = 0;
	dwModuleSize = 0;
	ME32.dwSize = sizeof(MODULEENTRY32);

	if (!Module32First(hModuleSnap, &ME32))
	{
		CloseHandle(hModuleSnap);
		return false;
	}

	do
	{
		if (!strcmp(ME32.szModule, sModuleName))
		{
			ModuleBaseAddr = ME32.modBaseAddr;
			dwModuleSize = ME32.modBaseSize;
			break;
		}
	} while (Module32Next(hModuleSnap, &ME32));

	CloseHandle(hModuleSnap);
	return (ModuleBaseAddr != 0);
}

bool Memory::IsProcessRunning()
{
	PROCESSENTRY32 PE32;
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	PE32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hProcessSnap, &PE32))
	{
		CloseHandle(hProcessSnap);
		return false;
	}

	do
	{
		if (PE32.th32ProcessID == dwProcessID)
		{
			CloseHandle(hProcessSnap);
			return true;
		}
	} while (Process32Next(hProcessSnap, &PE32));

	CloseHandle(hProcessSnap);
	return false;
}
