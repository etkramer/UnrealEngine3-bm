#include "StdAfx.h"
#include "RegisterContainerProperty.h"
#include "RegisterPack.h"
#include "RegisterProperty.h"

//////////////////////////////////////////////////////////////////////////////
// CRegisterContainerProperty methods.
// (This class inherits all interface methods from CContainerProperty.  This
// class supplies only an initialization method to fill in the container.)

CRegisterContainerProperty::CRegisterContainerProperty(void)
{
}

CRegisterContainerProperty::~CRegisterContainerProperty(void)
{
}

// Construct the list of register groups each of which contain a list of
// registers.
void CRegisterContainerProperty::Init()
{
	CContainerProperty::Init(CComBSTR("Registers Root"),CComBSTR(""), CComBSTR(""),true);

	// Instantiate register pack to access globally defined registers.
	CRegisterPack registerPack;

	size_t numGroups = registerPack.NumGroups();

	// For each group do
	for (size_t i = 0; i < numGroups; i++) {
		CComObject<CContainerProperty> *pGroup;
		CComObject<CContainerProperty>::CreateInstance(&pGroup);
		if (pGroup == NULL) {
			break;
		}
		pGroup->AddRef();
		CComBSTR groupName = registerPack.GetRegisterGroupName(i);
		pGroup->Init(groupName,CComBSTR(""), CComBSTR(""),true);
		AddChild(pGroup);
		size_t numRegisters = registerPack.NumRegistersInGroup(i);
		// For each register do
		for (size_t r = 0; r < numRegisters; r++) {
			Register reg = registerPack.GetRegister(i,r);
			if (reg.IsValid()) {
				CComBSTR regValue = registerPack.ValueToString(reg.value);
				CComObject<CRegisterProperty> *pRegisterProperty;
				CComObject<CRegisterProperty>::CreateInstance(&pRegisterProperty);
				if (pRegisterProperty == NULL) {
					i = numGroups;
					break;
				}
				pRegisterProperty->AddRef();
				pRegisterProperty->Init(groupName,static_cast<ULONG>(reg.index),reg.name,regValue);
				pGroup->AddChild(pRegisterProperty);
				pRegisterProperty->Release();
			}
		}
		pGroup->Release();
	}
}
