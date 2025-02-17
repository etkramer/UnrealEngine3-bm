// RegisterProperty.cpp : Implementation of CRegisterProperty

#include "stdafx.h"
#include "RegisterProperty.h"
#include "RegisterPack.h"


//////////////////////////////////////////////////////////////////////////////
// IDebugProperty2 interface

// Set the value of this property
HRESULT CRegisterProperty::SetValueAsString(
		LPCOLESTR pszValue,
		DWORD dwRadix,
		DWORD dwTimeout)
{
	HRESULT retval = E_SETVALUE_VALUE_IS_READONLY;
	if (!(m_dwAttrib & DBG_ATTRIB_VALUE_READONLY))
	{
		CRegisterPack registerPack;
		ULONGLONG value = registerPack.StringToValue(CComBSTR(pszValue));
		size_t groupIndex = registerPack.GetRegisterGroupIndex(m_bstrRegisterGroupName);
		registerPack.SetRegisterValue(groupIndex,m_ulRegisterIndex,value);
		m_bstrValue = registerPack.GetRegisterValueAsString(groupIndex,m_ulRegisterIndex);
		retval = S_OK;
	}
	return retval;
}

//////////////////////////////////////////////////////////////////////////////
// CRegisterProperty methods.

void CRegisterProperty::Init(BSTR registerGroupName,ULONG registerIndex,BSTR registerName,BSTR value)
{
	CDebugProperty::Init(registerName,value,CComBSTR("int"));
	m_bstrRegisterGroupName = registerGroupName;
	m_ulRegisterIndex = registerIndex;
}

