// DebugProperty.cpp : Implementation of CDebugProperty

#include "stdafx.h"
#include "DebugProperty.h"


//////////////////////////////////////////////////////////////////////////////
// IDebugProperty2 interface

// Set the value of this property
HRESULT CDebugProperty::SetValueAsString(
		LPCOLESTR pszValue,
		DWORD dwRadix,
		DWORD dwTimeout)
{
	HRESULT retval = E_SETVALUE_VALUE_IS_READONLY;

	if (!(m_dwAttrib & DBG_ATTRIB_VALUE_READONLY)) {
		m_bstrValue = pszValue;
		retval = S_OK;
	}
	return(retval);
}

// Set the value of this property
HRESULT CDebugProperty::SetValueAsReference(
		IDebugReference2** rgpArgs,
		DWORD dwArgCount,
		IDebugReference2* pValue,
		DWORD dwTimeout)
{ return E_SETVALUEASREFERENCE_NOTSUPPORTED; }

// Enum the children of this property
HRESULT CDebugProperty::EnumChildren(
		DEBUGPROP_INFO_FLAGS dwFields,
		DWORD dwRadix,
		REFGUID guidFilter,
		DBG_ATTRIB_FLAGS dwAttribFilter,
		LPCOLESTR pszNameFilter,
		DWORD dwTimeout,
		IEnumDebugPropertyInfo2** ppEnum)
{ return S_FALSE; }

// Get the parent of this property
HRESULT CDebugProperty::GetParent(
		IDebugProperty2** ppParent)
{
	HRESULT retval = E_POINTER;
	if (ppParent != NULL) {
		retval = S_GETPARENT_NO_PARENT;
		if (m_pParent != NULL) {
			retval = m_pParent->QueryInterface(__uuidof(IDebugProperty2),
											   (LPVOID *)ppParent);
		}
	}
	return(retval);
}

// Get the property that describes the derived most property of this property
HRESULT CDebugProperty::GetDerivedMostProperty(
		IDebugProperty2** ppDerivedMost)
{
	HRESULT retval = E_POINTER;
	if (ppDerivedMost != NULL) {
		*ppDerivedMost = NULL;
		retval = S_GETDERIVEDMOST_NO_DERIVED_MOST;
	}
	return(retval);
}

// Get the memory bytes that contains this property
HRESULT CDebugProperty::GetMemoryBytes(
		IDebugMemoryBytes2** ppMemoryBytes)
{ return E_NOTIMPL; }

// Get the size (in bytes) of this property
HRESULT CDebugProperty::GetSize(
		DWORD* pdwSize)
{ return E_NOTIMPL; }

// Get a reference for this property
HRESULT CDebugProperty::GetReference(
		IDebugReference2** ppReference)
{ return E_GETREFERENCE_NO_REFERENCE; }

	// Get extended info for this property
HRESULT CDebugProperty::GetExtendedInfo(
		REFGUID guidExtendedInfo,
		VARIANT* pExtendedInfo)
{ return S_GETEXTENDEDINFO_NO_EXTENDEDINFO; }

//----------------------------------------------------------------------------

	// Get the DEBUG_PROPERTY_INFO that describes this property
HRESULT CDebugProperty::GetPropertyInfo(
		DEBUGPROP_INFO_FLAGS dwFields,
		DWORD dwRadix,
		DWORD dwTimeout,
		IDebugReference2** rgpArgs,
		DWORD dwArgCount,
		DEBUG_PROPERTY_INFO* pPropertyInfo)
{
	HRESULT retval = E_POINTER;
	if (pPropertyInfo != NULL) {
		retval = S_OK;
		ZeroMemory(pPropertyInfo,sizeof(pPropertyInfo[0]));
		pPropertyInfo->dwFields = 0;
		if (dwFields & DEBUGPROP_INFO_FULLNAME) {
			m_bstrName.CopyTo(&pPropertyInfo->bstrFullName);
			pPropertyInfo->dwFields |= DEBUGPROP_INFO_FULLNAME;
		}
		if (dwFields & DEBUGPROP_INFO_NAME) {
			m_bstrName.CopyTo(&pPropertyInfo->bstrName);
			pPropertyInfo->dwFields |= DEBUGPROP_INFO_NAME;
		}
		if (dwFields & DEBUGPROP_INFO_TYPE) {
			m_bstrType.CopyTo(&pPropertyInfo->bstrType);
			pPropertyInfo->dwFields |= DEBUGPROP_INFO_TYPE;
		}
		if (dwFields & DEBUGPROP_INFO_VALUE) {
			m_bstrValue.CopyTo(&pPropertyInfo->bstrValue);
			pPropertyInfo->dwFields |= DEBUGPROP_INFO_VALUE;
		}
		if (dwFields & DEBUGPROP_INFO_PROP) {
			HRESULT hr = this->QueryInterface(__uuidof(IDebugProperty2),
										(LPVOID *)&pPropertyInfo->pProperty);
			if (SUCCEEDED(hr)) {
				pPropertyInfo->dwFields |= DEBUGPROP_INFO_PROP;
			}
		}
	}
	return(retval);
}

// Get a memory context for this property within the memory bytes returned by
// GetMemoryBytes
HRESULT CDebugProperty::GetMemoryContext(
		IDebugMemoryContext2** ppMemory)
{ return E_NOTIMPL; }

//////////////////////////////////////////////////////////////////////////////
// CDebugProperty methods.

void CDebugProperty::Init(BSTR pName, BSTR pValue, BSTR pType, bool bIsReadOnly/* = false*/)
{
	m_bstrName = pName;
	m_bstrValue = pValue;
	m_bstrType  = pType;
	if (bIsReadOnly) {
		m_dwAttrib |= DBG_ATTRIB_VALUE_READONLY;
	}
}

void CDebugProperty::SetParent(CDebugProperty *pParent)
{
	m_pParent = pParent;
}
