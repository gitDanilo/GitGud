#pragma once

#include <wtypes.h>

#define MAX_PATCHDATA_SIZE 4

enum class DataT
{
	no_type, uint_8, uint_16, int_16, uint_32, int_32, float_32, bit, bool_8
};

typedef struct _BIT
{
	bool one;
	BYTE bit;
	_BIT(bool one, BYTE bit)
	{
		this->one = one;
		this->bit = bit;
	}
} BIT, *PBIT;

typedef struct _PATCH_DATA
{
	DataT DataType;
	DWORD dwSize;
	BYTE Data[4];
	BYTE* Address;
} PATCH_DATA, *PPATCH_DATA;

//class PatchData
//{
//private:
//	BYTE Data[4];
//	DataT DataType;
//	DWORD dwSize;
//	BYTE* Address;
//public:
//	PatchData(const BYTE* pData, DataT DataType, BYTE* Address);
//	~PatchData();
//	BYTE* GetData();
//	DWORD GetDataSize();
//	BYTE*& GetDataAddress();
//	DataT GetDataType();
//};
