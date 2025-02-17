// MemoryContext.cpp : Implementation of CMemoryContext

#include "stdafx.h"
#include <strstream>
#include "MemoryContext.h"


//////////////////////////////////////////////////////////////////////////////
// IDebugMemoryContext2 methods.

HRESULT CMemoryContext::GetName(
        BSTR* pbstrName)
{
    HRESULT retval = E_POINTER;
    if (pbstrName != NULL) {
        *pbstrName = ::SysAllocString(m_contextInfo.bstrAddress);
        retval = S_OK;
    }
    return(retval);
}

HRESULT CMemoryContext::GetInfo(
        CONTEXT_INFO_FIELDS dwFields,
        CONTEXT_INFO* pInfo)
{
    HRESULT retval = E_POINTER;
    if (pInfo != NULL) {
        DWORD fields = dwFields & m_contextInfo.dwFields;
        if (fields & CIF_ADDRESS) {
            pInfo->bstrAddress = ::SysAllocString(m_contextInfo.bstrAddress);
        }
        if (fields & CIF_ADDRESSOFFSET) {
            pInfo->bstrAddressOffset = ::SysAllocString(m_contextInfo.bstrAddressOffset);
        }
        if (fields & CIF_ADDRESSABSOLUTE) {
            pInfo->bstrAddressAbsolute = ::SysAllocString(m_contextInfo.bstrAddressAbsolute);
        }
        pInfo->dwFields = dwFields;
        retval = S_OK;
    }
    return(retval);
}

HRESULT CMemoryContext::Add(
        UINT64 dwCount,
        IDebugMemoryContext2** ppMemCxt)
{
    HRESULT retval = CreateNewContextFromAddress(m_address + dwCount,ppMemCxt);
    return(retval);
}


HRESULT CMemoryContext::Subtract(
        UINT64 dwCount,
        IDebugMemoryContext2** ppMemCxt)
{
    HRESULT retval = CreateNewContextFromAddress(m_address - dwCount,ppMemCxt);
    return(retval);
}

HRESULT CMemoryContext::CreateNewContextFromAddress(UINT64 address,IDebugMemoryContext2** ppMemCxt)
{
    HRESULT retval = E_POINTER;
    if (ppMemCxt != NULL) {
        *ppMemCxt = NULL;
        CComObject<CMemoryContext> *pNewContext;
        retval = CComObject<CMemoryContext>::CreateInstance(&pNewContext);
        if (SUCCEEDED(retval)) {
            pNewContext->Init(address);
            retval = pNewContext->QueryInterface(__uuidof(IDebugMemoryContext2),(void **)ppMemCxt);
        }
    }
    return(retval);
}


HRESULT CMemoryContext::Compare(
        CONTEXT_COMPARE compare,
        IDebugMemoryContext2** rgpMemoryContextSet,
        DWORD dwMemoryContextSetLen,
        DWORD* pdwMemoryContext)
{
    HRESULT retval = E_POINTER;
    if (pdwMemoryContext != NULL && rgpMemoryContextSet != NULL) {
        retval = S_FALSE;
        bool bFound = false;
        for (DWORD i = 0; i < dwMemoryContextSetLen && !bFound; i++) {
            CONTEXT_INFO ci = { 0 };
            // If we can't get the info about the current pointer (becuause it is NULL or
            // the call to GetInfo fails) we continue with the next item in the array.
            if (NULL != rgpMemoryContextSet[i])
            {
                if (FAILED(rgpMemoryContextSet[i]->GetInfo(CIF_ADDRESS, &ci)))
                    continue;
            }
            else
            {
                continue;
            }
            UINT64 testAddress = _wcstoui64(ci.bstrAddress,NULL,0);
            switch(compare) {
                
                case CONTEXT_EQUAL:
                    if (testAddress == m_address) {
                        *pdwMemoryContext = i;
                        bFound = true;
                    }
                    break;

                case CONTEXT_LESS_THAN:
                    if (testAddress < m_address) {
                        *pdwMemoryContext = i;
                        bFound = true;
                    }
                    break;

                case CONTEXT_GREATER_THAN:
                    if (testAddress > m_address) {
                        *pdwMemoryContext = i;
                        bFound = true;
                    }
                    break;

                case CONTEXT_LESS_THAN_OR_EQUAL:
                    if (testAddress <= m_address) {
                        *pdwMemoryContext = i;
                        bFound = true;
                    }
                    break;

                case CONTEXT_GREATER_THAN_OR_EQUAL:
                    if (testAddress >= m_address) {
                        *pdwMemoryContext = i;
                        bFound = true;
                    }
                    break;

                case CONTEXT_SAME_SCOPE:
                case CONTEXT_SAME_FUNCTION:
                case CONTEXT_SAME_MODULE:
                case CONTEXT_SAME_PROCESS:
                    retval = E_COMPARE_CANNOT_COMPARE;
                default:
                    break;

            }
            if (bFound) {
                retval = S_OK;
            }
        }
    }
    return(retval);
}

//////////////////////////////////////////////////////////////////////////////
// CMemoryContext methods.

void CMemoryContext::Init(UINT64 address)
{
    m_address = address;
    wchar_t buffer[64];
    swprintf_s(buffer,sizeof(buffer)/sizeof(buffer[0]),L"0x%016I64X",address);
    m_contextInfo.bstrAddress         = ::SysAllocString(buffer);
    m_contextInfo.bstrAddressAbsolute = ::SysAllocString(m_contextInfo.bstrAddress);
    m_contextInfo.bstrAddressOffset   = NULL;
    m_contextInfo.bstrFunction        = NULL;
    m_contextInfo.bstrModuleUrl       = NULL;
    m_contextInfo.dwFields            = CIF_ADDRESS | CIF_ADDRESSABSOLUTE;
}

void CMemoryContext::FreeContextInfo()
{
    if (m_contextInfo.dwFields & CIF_ADDRESS) {
        ::SysFreeString(m_contextInfo.bstrAddress);
        m_contextInfo.bstrAddress = NULL;
    }
    if (m_contextInfo.dwFields & CIF_ADDRESSABSOLUTE) {
        ::SysFreeString(m_contextInfo.bstrAddressAbsolute);
        m_contextInfo.bstrAddressAbsolute = NULL;
    }
    if (m_contextInfo.dwFields & CIF_ADDRESSOFFSET) {
        ::SysFreeString(m_contextInfo.bstrAddressOffset);
        m_contextInfo.bstrAddressOffset = NULL;
    }
}
