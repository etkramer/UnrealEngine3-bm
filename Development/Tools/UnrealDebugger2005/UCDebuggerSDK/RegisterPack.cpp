#include "stdafx.h"
#include <sstream>
#include <iomanip>
#include "RegisterPack.h"

// Globally declare and instantiate the register group list.
RegisterGroupList CRegisterPack::m_registerGroupList;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Class RegisterGroup
RegisterGroup::RegisterGroup(BSTR pName/* = L""*/)
: name(pName)
{
}

void RegisterGroup::AddRegister(Register r)
{
	r.index = registerList.size();
	registerList.push_back(r);
}

size_t RegisterGroup::NumRegisters()
{
	return(registerList.size());
}

CComBSTR& RegisterGroup::Name()
{
	return(name);
}

Register RegisterGroup::GetRegister(size_t index)
{
	Register reg;
	if (index < registerList.size()) {
		reg = registerList[index];
	}
	return(reg);
}

void RegisterGroup::SetRegisterValue(size_t index, ULONGLONG value)
{
	if (index < registerList.size()) {
		registerList[index].value = value;
	}
}



//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Class CRegisterPack
CRegisterPack::CRegisterPack()
{
	if (m_registerGroupList.empty()) {
		RegisterGroup group(L"General Registers");
		group.AddRegister(Register(L"AX"));
		group.AddRegister(Register(L"BX"));
		group.AddRegister(Register(L"CX"));
		group.AddRegister(Register(L"DX"));
		m_registerGroupList.push_back(group);

		group = RegisterGroup(L"Special Registers");
		group.AddRegister(Register(L"IP"));
		group.AddRegister(Register(L"SP"));
		group.AddRegister(Register(L"LP"));
		m_registerGroupList.push_back(group);
	}
}

RegisterGroupList& CRegisterPack::GetGroupList()
{
	return(m_registerGroupList);
}

size_t CRegisterPack::NumGroups()
{
	return(m_registerGroupList.size());
}

size_t CRegisterPack::NumRegistersInGroup(size_t groupIndex)
{
	size_t numRegisters = 0;
	if (groupIndex < m_registerGroupList.size()) {
		numRegisters = m_registerGroupList[groupIndex].NumRegisters();
	}
	return(numRegisters);
}

BSTR CRegisterPack::GetRegisterGroupName(size_t groupIndex)
{
	BSTR pName = NULL;

	if (groupIndex < m_registerGroupList.size()) {
		pName = m_registerGroupList[groupIndex].Name();
	}
	return(pName);
}

size_t CRegisterPack::GetRegisterGroupIndex(BSTR pName)
{
	size_t index = 0;
	RegisterGroupList::iterator groupIter = m_registerGroupList.begin();
	while (groupIter != m_registerGroupList.end()) {
		if (groupIter->Name() == pName) {
			break;
		}
		++groupIter;
		++index;
	}
	return(index);
}

Register CRegisterPack::GetRegister(size_t groupIndex,size_t registerIndex)
{
	Register reg;
	if (groupIndex < m_registerGroupList.size()) {
		reg = m_registerGroupList[groupIndex].GetRegister(registerIndex);
	}
	return(reg);
}

void CRegisterPack::SetRegisterValue(size_t groupIndex, size_t registerIndex, ULONGLONG value)
{
	if (groupIndex < m_registerGroupList.size()) {
		m_registerGroupList[groupIndex].SetRegisterValue(registerIndex,value);
	}
}

BSTR CRegisterPack::GetRegisterValueAsString(size_t groupIndex, size_t registerIndex)
{
	BSTR retval = NULL;
	Register reg = GetRegister(groupIndex,registerIndex);
	if (reg.IsValid()) {
		retval = ValueToString(reg.value);
	}
	return(retval);
}

//////////////////////////////////////////////////////////////////////////////
// Static methods
// Format a register value to a string.  Always use this to format a string
// for a register.
BSTR CRegisterPack::ValueToString(ULONGLONG value)
{
	CComBSTR valueString;
	std::wstringstream output;
	output << std::setfill(L'0') << std::setw(16) << std::hex << value;
	valueString = output.str().c_str();
	return(valueString.Detach());
}

// Parse a string into a value for a register.  Very basic.  Assumes
// hexadecimal values coming in.
ULONGLONG CRegisterPack::StringToValue(BSTR pValue)
{
	ULONGLONG value = 0;
	if (pValue != NULL) {
		value = _wcstoui64(pValue,NULL,16);
	}
	return(value);
}
