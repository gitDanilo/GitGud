#pragma once

#include <Windows.h>
#include <TlHelp32.h>

// Macros
#define GETBIT(A, B) (((A >> B) & 0x1))
#define SETBIT(A, B) (A |= (0x1 << B))
#define CLBIT(A, B) (A &= ~(0x1 << B))
#define AOB2DWORD(A, B) (((A)[3 + (B)] << 24) | ((A)[2 + (B)] << 16) | ((A)[1 + (B)] << 8) | ((A)[(B)]))
#define AOB2QWORD(A, B) (((A)[7 + (B)] << 56) | ((A)[6 + (B)] << 48) | ((A)[5 + (B)] << 40) | ((A)[4 + (B)] << 32) | ((A)[3 + (B)] << 24) | ((A)[2 + (B)] << 16) | ((A)[1 + (B)] << 8) | ((A)[(B)]))

typedef unsigned long long QWORD;

//enum MemoryAction { READ, WRITE };
//
//class MemoryException: public std::exception
//{
//private:
//	DWORD dwAddr;
//	MemoryAction Action;
//public:
//	MemoryException(MemoryAction Action, DWORD dwAddr)
//	{
//		this->dwAddr = dwAddr;
//		this->Action = Action;
//	}
//	DWORD getAddr()
//	{
//		return dwAddr;
//	}
//	const char* what() const throw()
//	{
//		switch (Action)
//		{
//			case MemoryAction::READ:
//				return "Error reading memory address";
//			case MemoryAction::WRITE:
//				return "Error writing memory address";
//			default:
//		}
//		return "Unknown exception!";
//	}
//};

class Memory
{
private:
	Memory();
	//static Memory* pInstance;
	HANDLE hProcess;
	DWORD dwProcessID;
public:
	Memory(Memory const&) = delete;
	//void operator=(Memory const&) = delete;
	static Memory* GetInstance();
	~Memory();
	bool LoadProcess(const char* sProcessName);
	template<class T>
	inline bool ReadProcMem(LPCVOID lpAddress, T &Data)
	{
		return ReadProcessMemory(hProcess, lpAddress, &Data, sizeof(T), nullptr);
	}
	inline bool ReadProcMem(LPCVOID lpAddress, BYTE* aobData, SIZE_T aobSize)
	{
		return ReadProcessMemory(hProcess, lpAddress, aobData, aobSize, nullptr);
	}
	template<class T>
	inline bool WriteProcMem(LPVOID lpAddress, const T &Data)
	{
		return WriteProcessMemory(hProcess, lpAddress, &Data, sizeof(T), nullptr);
	}
	inline bool WriteProcMem(LPVOID lpAddress, const BYTE* aobData, SIZE_T aobSize)
	{
		return WriteProcessMemory(hProcess, lpAddress, aobData, aobSize, nullptr);
	}
	BYTE* AOBScan(const BYTE* AOB, DWORD dwImageSize, const BYTE* TargetAOB, DWORD dwLength);
	bool LoadModule(const char* sModuleName, BYTE* &ModuleBaseAddr, DWORD &dwModuleSize);
	bool IsProcessRunning();
};
