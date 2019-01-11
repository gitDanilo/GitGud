#include "PatchData.h"

PatchData::PatchData()
{
	memset(this->Data, 0, PATCHDATA_SIZE);
	this->Address = 0;
}

void PatchData::SetType(DataT DataType)
{
	this->DataType = DataType;
	switch (this->DataType)
	{
		case DataT::bool_8:
		case DataT::uint_8:
			this->dwSize = 1;
			break;
		case DataT::bit:
		case DataT::uint_16:
		case DataT::int_16:
			this->dwSize = 2;
			break;
		case DataT::uint_32:
		case DataT::int_32:
		case DataT::float_32:
			this->dwSize = 4;
			break;
		default:
			this->dwSize = 0;
	}
}

DWORD PatchData::GetSize()
{
	return this->dwSize;
}

DataT PatchData::GetType()
{
	return this->DataType;
}
