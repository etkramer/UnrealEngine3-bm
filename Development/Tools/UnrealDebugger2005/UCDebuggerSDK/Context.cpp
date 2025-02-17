// DebugContext.cpp : Implementation of CDebugContext

#include "stdafx.h"
#include "Context.h"


// CDebugContext

//////////////////////////////////////////////////////////////////////////////
// IDebugMemoryContext2 methods
HRESULT CDebugContext::GetInfo(CONTEXT_INFO_FIELDS dwFields, CONTEXT_INFO* pInfo)
{ return E_NOTIMPL; }
HRESULT CDebugContext::Add(UINT64 dwCount, IDebugMemoryContext2** ppMemCxt)
{ return E_NOTIMPL; }
HRESULT CDebugContext::Subtract(UINT64 dwCount, IDebugMemoryContext2** ppMemCxt)
{ return E_NOTIMPL; }
HRESULT CDebugContext::Compare(CONTEXT_COMPARE compare, IDebugMemoryContext2** rgpMemoryContextSet, DWORD dwMemoryContextSetLen, DWORD* pdwMemoryContext)
{ return E_NOTIMPL; }

//////////////////////////////////////////////////////////////////////////////
// IDebugDocumentContext2 methods
HRESULT CDebugContext::GetDocument(IDebugDocument2 **ppDocument)
{ return E_NOTIMPL; }
HRESULT CDebugContext::GetName(BSTR* pbstrName)
{ return E_NOTIMPL; }
HRESULT CDebugContext::EnumCodeContexts(IEnumDebugCodeContexts2 **ppEnumCodeCxts)
{ return E_NOTIMPL; }
HRESULT CDebugContext::GetLanguageInfo(BSTR* pbstrLanguage, GUID* pguidLanguage)
{ return E_NOTIMPL; }
HRESULT CDebugContext::Compare(DOCCONTEXT_COMPARE compare, IDebugDocumentContext2** rgpDocContextSet, DWORD dwDocContextSetLen, DWORD* pdwDocContext)
{ return E_NOTIMPL; }
HRESULT CDebugContext::Seek(int ncount, IDebugDocumentContext2 **ppDocContext)
{ return E_NOTIMPL; }

//////////////////////////////////////////////////////////////////////////////
// IDebugCodeContext2 methods
HRESULT CDebugContext::GetDocumentContext(IDebugDocumentContext2 **ppSrcCxt)
{
    if (NULL == ppSrcCxt)
        return E_POINTER;
    *ppSrcCxt = (IDebugDocumentContext2 *) this;
    (*ppSrcCxt)->AddRef();
    return S_OK;
} 

HRESULT CDebugContext::GetName(GETNAME_TYPE gnType, BSTR *pbstrFileName)
{
    if (NULL == pbstrFileName)
        return E_POINTER;
    *pbstrFileName = SysAllocString(m_sbstrFileName);
    return S_OK;
}

HRESULT CDebugContext::GetSourceRange(
    TEXT_POSITION* pBegPosition,
    TEXT_POSITION* pEndPosition)
{
    if (pBegPosition)
        *pBegPosition = m_pos;
    if (pEndPosition)
        *pEndPosition = m_pos;
    return S_OK;
}

HRESULT CDebugContext::GetStatementRange(
    TEXT_POSITION* pBegPosition,
    TEXT_POSITION* pEndPosition)
{
    return GetSourceRange(pBegPosition, pEndPosition);
}

void CDebugContext::Initialize(LPCWSTR pszFileName, TEXT_POSITION& pos)
{
    m_sbstrFileName = pszFileName;
    m_pos = pos;
}


// CBreakpointResolution
CBreakpointResolution::CBreakpointResolution()
{
    memset(&m_bpResolutionInfo, 0, sizeof(BP_RESOLUTION_INFO));

    m_bpResolutionInfo.bpResLocation.bpType = BPT_CODE;
    m_bpResolutionInfo.dwFields = BPRESI_BPRESLOCATION;
}

CBreakpointResolution::~CBreakpointResolution()
{
    if (NULL != m_bpResolutionInfo.bpResLocation.bpResLocation.bpresCode.pCodeContext)
        m_bpResolutionInfo.bpResLocation.bpResLocation.bpresCode.pCodeContext->Release();
}

//////////////////////////////////////////////////////////////////////////////
// IDebugBreakpointResolution2
HRESULT CBreakpointResolution::GetBreakpointType(BP_TYPE* pBPType)
{
    if (NULL == pBPType)
        return E_POINTER;
    *pBPType = BPT_CODE;
    return S_OK;
}

HRESULT CBreakpointResolution::GetResolutionInfo(BPRESI_FIELDS dwFields, BP_RESOLUTION_INFO* pBPResolutionInfo)
{
    // Start with a raw copy
    if (pBPResolutionInfo == NULL)
        return E_POINTER;

    *pBPResolutionInfo = m_bpResolutionInfo;

    // Set the fields.
    pBPResolutionInfo->dwFields = dwFields & m_bpResolutionInfo.dwFields;

    // Fill in the bp resolution destination
    if (pBPResolutionInfo->dwFields & BPRESI_BPRESLOCATION)
    {
        if (NULL != pBPResolutionInfo->bpResLocation.bpResLocation.bpresCode.pCodeContext)
            pBPResolutionInfo->bpResLocation.bpResLocation.bpresCode.pCodeContext->AddRef();
        return S_OK;
    }
    return E_NOTIMPL;
}

//////////////////////////////////////////////////////////////////////////////
// CBreakpointResolution methods.

void CBreakpointResolution::Initialize(IDebugCodeContext2 * pCodeContext)
{
    m_bpResolutionInfo.bpResLocation.bpResLocation.bpresCode.pCodeContext = pCodeContext;
    if (NULL != m_bpResolutionInfo.bpResLocation.bpResLocation.bpresCode.pCodeContext)
        m_bpResolutionInfo.bpResLocation.bpResLocation.bpresCode.pCodeContext->AddRef();
}
