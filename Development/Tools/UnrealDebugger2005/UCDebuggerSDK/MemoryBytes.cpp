// MemoryBytes.cpp : Implementation of CMemoryBytes

#include "stdafx.h"
#include "MemoryBytes.h"

//////////////////////////////////////////////////////////////////////////////
// This class represents a chunk of memory.
// It is used to simulate accessing memory through a Memory window.
class CLocalMemory {
    
    private:
        BYTE *pMemory;
        DWORD length;
    public:
        CLocalMemory()
            : length(8192)
        {
            pMemory = new BYTE[length];
            if (NULL != pMemory)
            {
                WORD *pTemp = reinterpret_cast<WORD *>(pMemory);
                // Fill memory with interesting pattern
                for (DWORD i = 0; i < length; i += sizeof(pTemp[0])) {
                    *pTemp++ = 0x55aa;
                }
            }
        }
        ~CLocalMemory()
        {
            if (NULL != pMemory)
                delete[] pMemory;
            pMemory = NULL;
        }

        DWORD& Length() { return(length); }
        BYTE *Memory() { return(pMemory); }
};

// This is global so the same memory is accessed in any Memory window.
static CLocalMemory gLocalMemory;
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// IDebugMemoryBytes2 interface

HRESULT CMemoryBytes::ReadAt(
        IDebugMemoryContext2* pStartContext,
        DWORD dwCount,
        BYTE* rgbMemory,
        DWORD* pdwRead,
        DWORD* pdwUnreadable)
{
    if (NULL == rgbMemory)
        return E_POINTER;
    if (NULL == gLocalMemory.Memory())
        return E_OUTOFMEMORY;
    UINT64 address = 0;
    HRESULT retval = GetMemoryAddress(pStartContext,&address);
    if (SUCCEEDED(retval)) {
        UINT64 memoryLength = gLocalMemory.Length();
        if (address >= memoryLength) {
            // Outside our memory space
            if (pdwRead != NULL) {
                *pdwRead = 0;    // we are in unreadable space, nothing to read
            }
            UINT64 testAddress = address + dwCount;
            DWORD unreadableCount = dwCount;
            if (testAddress < address) {
                // We have wrapped into our memory space
                unreadableCount -= static_cast<DWORD>(testAddress);
            }
            if (pdwUnreadable != NULL) {
                *pdwUnreadable = unreadableCount;
            }
            return S_OK;
        }
        DWORD bytesToRead = min(dwCount,static_cast<DWORD>(memoryLength - address));
        DWORD offset = static_cast<DWORD>(address);
        for (DWORD i = 0; i < bytesToRead; i++) {
            rgbMemory[i] = gLocalMemory.Memory()[offset + i];
        }
        if (pdwRead != NULL) {
            *pdwRead = bytesToRead;
        }
        if (pdwUnreadable != NULL) {
            *pdwUnreadable = dwCount - bytesToRead;
        }
    }
    return(retval);
}


HRESULT CMemoryBytes::WriteAt(
        IDebugMemoryContext2* pStartContext,
        DWORD dwCount,
        BYTE* rgbMemory)
{
    if (NULL == rgbMemory)
        return E_POINTER;
    if (NULL == gLocalMemory.Memory())
        return E_OUTOFMEMORY;
    UINT64 address = 0;
    HRESULT retval = GetMemoryAddress(pStartContext,&address);
    if (SUCCEEDED(retval)) {
        UINT64 memoryLength = gLocalMemory.Length();
        if (address < memoryLength) {
            DWORD bytesToWrite = min(dwCount,static_cast<DWORD>(memoryLength - address));
            DWORD offset = static_cast<DWORD>(address);
            for (DWORD i = 0; i < bytesToWrite; i++) {
                gLocalMemory.Memory()[offset + i] = rgbMemory[i];
            }
        }
    }
    return(retval);
}


HRESULT CMemoryBytes::GetSize(UINT64* pqwSize)
{
    HRESULT retval = E_POINTER;
    if (pqwSize != NULL) {
        *pqwSize = gLocalMemory.Length();
        retval = S_OK;
    }

    return(retval);
}

//////////////////////////////////////////////////////////////////////////////
// CMemoryBytes functions.

HRESULT CMemoryBytes::GetMemoryAddress(IDebugMemoryContext2 *pContext, UINT64 *pAddress) 
{
    HRESULT retval = E_POINTER;
    if (pContext != NULL && pAddress != NULL) {
        CONTEXT_INFO ci = { 0 };
        retval = pContext->GetInfo(CIF_ADDRESS,&ci);
        if (SUCCEEDED(retval)) {
            if (ci.bstrAddress != NULL) {
                *pAddress = _wcstoui64(ci.bstrAddress,NULL,0);
            } else {
                retval = E_INVALIDARG;
            }
        }
    }
    return(retval);
}