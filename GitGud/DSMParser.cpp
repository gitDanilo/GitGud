#include "DSMParser.h"

DSMParser::DSMParser(DWORD dwFileID)
{
	if (dwFileID > FileID::_last)
		return;

	this->dwFileID = dwFileID;

	inStream.open(FileList[dwFileID].sName, std::ios::in);
}

DSMParser::~DSMParser()
{
	if (inStream.is_open())
		inStream.close();
}

void DSMParser::SetFile(DWORD dwFileID)
{
	if (dwFileID > FileID::_last)
		return;

	if (inStream.is_open())
		inStream.close();

	this->dwFileID = dwFileID;

	inStream.open(FileList[dwFileID].sName, std::ios::in);
}

bool DSMParser::ReadNextLine()
{
	++dwLineCount;

	if (!std::getline(inStream, sLine))
		return false;

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

bool DSMParser::GetPatchData(const FIELD& Field, bool bHex, PatchData &Patch)
{
	DWORD dwTemp;
	try
	{
		switch (Field.DataType)
		{
			case DataT::uint_8:
			{
				dwTemp = std::stoul(sFieldData, nullptr, bHex ? 16 : 10);
				if (dwTemp > MAXBYTE)
					return false;
				(*Patch.Data) = static_cast<BYTE>(dwTemp);
				break;
			}
			case DataT::uint_16:
			case DataT::int_16:
			{
				dwTemp = std::stoul(sFieldData, nullptr, bHex ? 16 : 10);
				if (dwTemp > MAXWORD)
					return false;
				(*reinterpret_cast<WORD*>(Patch.Data)) = static_cast<WORD>(dwTemp);
				break;
			}
			case DataT::uint_32:
			case DataT::int_32:
			{
				(*reinterpret_cast<DWORD*>(Patch.Data)) = std::stoul(sFieldData, nullptr, bHex ? 16 : 10);
				break;
			}
			case DataT::float_32:
			{
				if (bHex)
					return false;
				(*reinterpret_cast<FLOAT*>(Patch.Data)) = std::stof(sFieldData);
				break;
			}
			case DataT::bit:
			{
				PBIT pBit = reinterpret_cast<PBIT>(Patch.Data);
				if (bHex)
					return false;
				if (sFieldData.compare("true") == 0)
					pBit->one = true;
				else if (sFieldData.compare("false") == 0)
					pBit->one = false;
				else
					return false;
				pBit->bit = Field.BitOffset;
				break;
			}
			case DataT::bool_8:
			{
				if (bHex)
					return false;
				if (sFieldData.compare("true") == 0)
					(*reinterpret_cast<bool*>(Patch.Data)) = true;
				else if (sFieldData.compare("false") == 0)
					(*reinterpret_cast<bool*>(Patch.Data)) = false;
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
		return false;
	}
	return true;
}

RET_INF DSMParser::GetPatchDataList(const PARAM_CLASS &Param, std::vector<PatchData> &PatchDataList)
{
	bool bHex;
	PFIELD pFieldList;
	PatchData Patch;
	DWORD i;
	DWORD dwID;
	DWORD dwListSize;
	BYTE* Addr;

	if (inStream.sync() != 0 || !inStream.seekg(0, std::ios::beg))
		return SetRetInf(ReturnID::read_error);

	dwLineCount = 0;
	pFieldList = FileList[this->dwFileID].pFieldList;
	dwListSize = FileList[this->dwFileID].dwListSize;

	if (this->dwFileID > FileID::_last || pFieldList == nullptr)
		return SetRetInf(ReturnID::read_error);

	do
	{
		bHeader = true;
		Addr = 0;
		for (i = 0; i < dwListSize; ++i)
		{
			while (ReadNextLine() == false && inStream.good());

			if (inStream.bad())
				return SetRetInf(ReturnID::read_error);

			if (inStream.eof() && i != dwListSize - 1)
				return SetRetInf(ReturnID::end_of_file);

			if (GetNextField() == false)
				return SetRetInf(ReturnID::syntax_error);

			if (sField.compare(pFieldList[i].sName) != 0)
				return SetRetInf(ReturnID::invalid_field);

			if (sFieldData.empty())
				return SetRetInf(ReturnID::invalid_data);

			bHex = (sFieldData.find("0x") == 0);

			// Get header data
			if (i == 0)
			{
				try
				{
					dwID = std::stoul(sFieldData, nullptr, bHex ? 16 : 10);
				}
				catch (...)
				{
					return SetRetInf(ReturnID::invalid_data);
				}
				Addr = ParamUtil::GetIDAddr(Param, dwID);
				if (Addr == nullptr)
					return SetRetInf(ReturnID::invalid_id);
			}
			// Get field data
			else
			{
				// Skip
				if (sFieldData.compare("@") == 0)
					continue;

				if (GetPatchData(pFieldList[i], bHex, Patch) == false)
					return SetRetInf(ReturnID::invalid_data);

				Patch.Address = Addr + pFieldList[i].dwAddrOffset;
				Patch.SetType(pFieldList[i].DataType);

				PatchDataList.push_back(Patch);
			}
		}
	} while (inStream.eof() == false);

	return SetRetInf(ReturnID::success);
}

RET_INF DSMParser::GetStruct(BYTE* pStruct)
{
	bool bHex;
	DWORD i;
	DWORD dwListSize;
	PSTRUCT_FIELD pFieldList;

	if (inStream.sync() != 0 || !inStream.seekg(0, std::ios::beg))
		return SetRetInf(ReturnID::read_error);

	dwLineCount = 0;

	pFieldList = FileList[this->dwFileID].pStructFieldList;
	dwListSize = FileList[this->dwFileID].dwListSize;

	if (this->dwFileID > FileID::_last || pFieldList == nullptr)
		return SetRetInf(ReturnID::read_error);

	bHeader = true;
	for (i = 0; i < dwListSize; ++i)
	{
		while (ReadNextLine() == false && inStream.good());

		if (inStream.bad())
			return SetRetInf(ReturnID::read_error);

		if (inStream.eof() && i != dwListSize - 1)
			return SetRetInf(ReturnID::end_of_file);

		if (GetNextField() == false)
			return SetRetInf(ReturnID::syntax_error);

		if (sField.compare(pFieldList[i].sName) != 0)
			return SetRetInf(ReturnID::invalid_field);

		if (pFieldList[i].DataType == DataT::no_type && !sFieldData.empty())
			return SetRetInf(ReturnID::invalid_data);

		if (i == 0)
			continue;

		bHex = (sFieldData.find("0x") == 0);

		// Get field data
		switch (pFieldList[i].DataType)
		{
			case DataT::bool_8:
			{
				if (sFieldData.compare("true") == 0)
					(*reinterpret_cast<bool*>(pStruct + pFieldList[i].dwStructOffset)) = true;
				else if (sFieldData.compare("false") == 0)
					(*reinterpret_cast<bool*>(pStruct + pFieldList[i].dwStructOffset)) = false;
				else
					return SetRetInf(ReturnID::invalid_data);
				break;
			}
			default:
			{
				return SetRetInf(ReturnID::invalid_data);
			}
		}
	}

	return SetRetInf(ReturnID::success);
}
