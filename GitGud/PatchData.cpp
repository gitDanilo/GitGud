#include "PatchData.h"

PatchData::PatchData(DataT DataType)
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
		default:
			this->dwSize = 0;
			return;
	}
}

DWORD PatchData::GetSize()
{
	return this->dwSize;
}

//PatchData::PatchData(const BYTE* pData, DataT DataType, BYTE* Address)
//{
//	switch (DataType)
//	{
//		case DataT::bit:
//		case DataT::bool_8:
//		case DataT::uint_8:
//			this->dwSize = 1;
//			break;
//		case DataT::uint_16:
//		case DataT::int_16:
//			this->dwSize = 2;
//			break;
//		case DataT::uint_32:
//		case DataT::int_32:
//		case DataT::float_32:
//			this->dwSize = 4;
//		default:
//			this->dwSize = 0;
//			return;
//	}
//	memcpy(&this->Data, pData, sizeof(this->dwSize));
//	this->Address = Address;
//}
//
//PatchData::~PatchData()
//{
//
//}
//
//BYTE* PatchData::GetData()
//{
//	return this->Data;
//}
//
//DWORD PatchData::GetDataSize()
//{
//	return this->dwSize;
//}
//
//BYTE*& PatchData::GetDataAddress()
//{
//	return this->Address;
//}
//
//DataT PatchData::GetDataType()
//{
//	return this->DataType;
//}
