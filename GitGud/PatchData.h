#pragma once

#include <wtypes.h>

#define PATCHDATA_SIZE 4

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
	_BIT()
	{
		this->one = false;
		this->bit = 0;
	}
} BIT, *PBIT;

class PatchData
{
private:
	DataT DataType;
	DWORD dwSize;
public:
	BYTE Data[4];
	BYTE* Address;
	PatchData();
	void SetType(DataT DataType);
	DWORD GetSize();
	DataT GetType();
};
