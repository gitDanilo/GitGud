#include "DSMParser.h"

DSMParser::DSMParser(FileID FileID)
{
	this->File = FileID;
	switch (this->File)
	{
		case FileID::player:
			inStream.open(DSM_PLAYER_FILE, std::ios::in);
			dwEnabledListOffset = offsetof(PLAYER_STRUCT, FieldEnabled);
			break;
		case FileID::weapon:
			inStream.open(DSM_WEAPON_FILE, std::ios::in);
			dwEnabledListOffset = offsetof(WEAPON_STRUCT, FieldEnabled);
			break;
		case FileID::effect:
			inStream.open(DSM_EFFECT_FILE, std::ios::in);
			dwEnabledListOffset = offsetof(EFFECT_STRUCT, FieldEnabled);
			break;
	}
}

DSMParser::~DSMParser()
{
	if (inStream.is_open())
		inStream.close();
}

bool DSMParser::ReadNextLine()
{
	if (!std::getline(inStream, sLine))
		return false;

	++dwLineCount;

	sLine.erase(std::remove(sLine.begin(), sLine.end(), ' '), sLine.end());

	if (sLine.empty())
		return false;

	std::transform(sLine.begin(), sLine.end(), sLine.begin(), tolower);

	lPos = sLine.find("--");
	if (lPos != std::string::npos)
		sLine.erase(lPos, sLine.size() - lPos);

	return (!sLine.empty());
}

bool DSMParser::GetNextField()
{
	if (bHeader)
	{
		if (*sLine.begin() != '[' || *(sLine.end() - 1) != ']')
			return false;

		lPos = sLine.find(':');
		if (lPos != std::string::npos)
		{
			if (lPos == 1 || (lPos + 1) == sLine.size() - 1)
				return false;

			sField = sLine.substr(1, lPos - 1);
			sFieldData = sLine.substr(lPos + 1, sLine.size() - lPos - 2);
			bHeader = false;

			return true;
		}

		if (sLine.size() <= 2)
			return false;

		sField = sLine.substr(1, sLine.size() - 2);
		sFieldData.clear();
		bHeader = false;
	}
	else
	{
		lPos = sLine.find('=');
		if (lPos == std::string::npos || lPos == 0)
			return false;

		sField = sLine.substr(0, lPos);
		sFieldData = sLine.substr(lPos + 1, sLine.size() - lPos - 1);
	}

	return true;
}

RET_INF DSMParser::SetRetInf(ReturnID RetID)
{
	this->RetInf.RetID = RetID;
	this->RetInf.dwLine = this->dwLineCount;
	return RetInf;
}

bool DSMParser::GetPatchData(const FIELD& Field, PatchData &Data)
{
	bool bHex = (sFieldData.rfind("0x", 0) == 0);
	try
	{
		switch (Field.DataType)
		{
			case DataT::uint_8:
			{
				DWORD dwTemp = std::stoul(sFieldData, nullptr, bHex ? 16 : 10);
				if (dwTemp > MAXBYTE)
					return false;
				
				(*reinterpret_cast<BYTE*>(Data.Data)) = static_cast<BYTE>(dwTemp);
				break;
			}
			case DataT::uint_16:
			{
				(*reinterpret_cast<WORD*>(pBasePtrData + pFieldList[i].dwOffset)) = static_cast<WORD>(std::stoul(sFieldData, nullptr, bHex ? 16 : 10));
				break;
			}
			case DataT::int_16:
			{
				(*reinterpret_cast<short*>(pBasePtrData + pFieldList[i].dwOffset)) = static_cast<short>(std::stol(sFieldData, nullptr, bHex ? 16 : 10));
				break;
			}
			case DataT::uint_32:
			{
				(*reinterpret_cast<DWORD*>(pBasePtrData + pFieldList[i].dwOffset)) = std::stoul(sFieldData, nullptr, bHex ? 16 : 10);
				break;
			}
			case DataT::int_32:
			{
				(*reinterpret_cast<int*>(pBasePtrData + pFieldList[i].dwOffset)) = std::stoi(sFieldData, nullptr, bHex ? 16 : 10);
				break;
			}
			case DataT::float_32:
			{
				if (bHex)
					return false;
				(*reinterpret_cast<FLOAT*>(pBasePtrData + pFieldList[i].dwOffset)) = std::stof(sFieldData);
				break;
			}
			case DataT::bit:
			{
				if (bHex)
					return false;
				break;
			}
			case DataT::bool_8:
			{
				if (bHex)
					return false;
				if (sFieldData.compare("true") == 0)
					(*reinterpret_cast<BOOL*>(pBasePtrData + pFieldList[i].dwOffset)) = TRUE;
				else if (sFieldData.compare("false") == 0)
					(*reinterpret_cast<BOOL*>(pBasePtrData + pFieldList[i].dwOffset)) = FALSE;
				else
					return false;
				break;
			}
			default:
			{
				return false;
			}
		}
	}
	catch (...)
	{
		return ReturnID::invalid_data;
	}
	return ReturnID::success;
}

RET_INF DSMParser::GetFirstDataStruct(PVOID ptrData)
{
	if (inStream.sync() != 0 || !inStream.seekg(0, std::ios::beg))
		return SetRetInf(ReturnID::read_error);

	bHeader = true;
	dwLineCount = 0;
	
	return GetNextDataStruct(ptrData);
}

RET_INF DSMParser::GetNextDataStruct(PVOID ptrData)
{
	char* pBasePtrData = static_cast<char*>(ptrData);
	int i, iFieldCount;
	PFIELD pFieldList;
	switch (File)
	{
		case FileID::player:
			iFieldCount = PlayerID::_size;
			pFieldList = PlayerFieldList;
			break;
		case FileID::weapon:
			iFieldCount = WeaponID::_size;
			pFieldList = WeaponFieldList;
			break;
		case FileID::effect:
			iFieldCount = EffectID::_size;
			pFieldList = EffectFieldList;
			break;
		default:
			iFieldCount = 0;
			pFieldList = NULL;
	}
	for (i = 0; i < iFieldCount; ++i)
	{
		while (ReadNextLine() == false && inStream.good());

		if (inStream.bad())
			return SetRetInf(ReturnID::read_error);

		if (inStream.eof() && i != iFieldCount - 1)
			return SetRetInf(ReturnID::end_of_file);

		if (GetNextField() == false)
			return SetRetInf(ReturnID::syntax_error);

		if (sField.compare(pFieldList[i].sName) != 0)
			return SetRetInf(ReturnID::invalid_field);

		// Disable a field
		if (sFieldData.compare("@") == 0)
		{
			if (i != 0)
				continue;
			return SetRetInf(ReturnID::invalid_data);
		}

		GetPatchData();
	}
	bHeader = true;
	return SetRetInf(ReturnID::success);
}
