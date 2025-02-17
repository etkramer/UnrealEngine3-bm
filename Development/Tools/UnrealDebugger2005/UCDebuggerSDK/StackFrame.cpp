// TIStackFrame.cpp : Implementation of CTIStackFrame

#include "stdafx.h"
#include "StackFrame.h"
#include "RegisterContainerProperty.h"
#include "Context.h"
#include "Engine.h"


extern CCallStack* g_CallStack = NULL;

// CTIStackFrame

//////////////////////////////////////////////////////////////////////////////
// IDebugStackFrame2
HRESULT CTIStackFrame::GetCodeContext(IDebugCodeContext2 **ppCodeCxt)
{ return E_NOTIMPL; }
HRESULT CTIStackFrame::GetName(BSTR* pbstrName)
{ return E_NOTIMPL; }
HRESULT CTIStackFrame::GetInfo(
    FRAMEINFO_FLAGS dwFieldSpec, UINT nRadix, FRAMEINFO *pFrameInfo)
{ return E_NOTIMPL; }
HRESULT CTIStackFrame::GetPhysicalStackRange(UINT64* paddrMin, UINT64* paddrMax)
{ return E_NOTIMPL; }
HRESULT CTIStackFrame::GetLanguageInfo(BSTR* pbstrLanguage, GUID* pguidLanguage)
{ return E_NOTIMPL; }
HRESULT CTIStackFrame::GetDebugProperty(IDebugProperty2 **ppDebugProp)
{ return E_NOTIMPL; }
HRESULT CTIStackFrame::GetExpressionContext(IDebugExpressionContext2 **ppExprCxt)
{
    HRESULT retval = E_INVALIDARG;
    if (ppExprCxt != NULL)
	{
        CComQIPtr<IDebugExpressionContext2> pExpressionContext(m_spEngine);
        if (pExpressionContext != NULL)
		{
            *ppExprCxt = pExpressionContext.Detach();
            retval = S_OK;
        }
		else
		{
            retval = E_NOTIMPL;
        }
    }
    return retval ;
}
HRESULT CTIStackFrame::EnumProperties(
    DEBUGPROP_INFO_FLAGS dwFields, UINT nRadix, REFGUID guidFilter,
    DWORD dwTimeout, ULONG *pcelt, IEnumDebugPropertyInfo2 **ppepi)
{
    HRESULT retval = E_POINTER;

    if (pcelt != NULL && ppepi != NULL)
	{
        *pcelt = 0;
        *ppepi = NULL;
        retval = E_NOTIMPL;
        if (guidFilter == guidFilterRegisters)
		{
            CComObject<CRegisterContainerProperty> *pRegisterContainerProperty;
            retval = CComObject<CRegisterContainerProperty>::CreateInstance(&pRegisterContainerProperty);
            if (SUCCEEDED(retval))
			{
                pRegisterContainerProperty->AddRef();
                pRegisterContainerProperty->Init();
                retval = pRegisterContainerProperty->EnumChildren(dwFields,
                                                          nRadix,
                                                          guidFilter,
                                                          DBG_ATTRIB_ALL,
                                                          NULL,
                                                          dwTimeout,
                                                          ppepi);
                if (SUCCEEDED(retval))
				{
                    if (retval == S_FALSE)
					{
                        *pcelt = 0;
                    }
					else
					{
                        (*ppepi)->GetCount(pcelt);
                    }
                }
                pRegisterContainerProperty->Release();
            }
        }
    }
    return retval;
}
HRESULT CTIStackFrame::GetThread(IDebugThread2** ppThread)
{ return E_NOTIMPL; }

ULONG g_curFrame = 0;
void CALLBACK SetStackTimerProc(HWND hWnd, UINT, UINT idEvent, DWORD)
{
	WCHAR cmd[512];
	KillTimer(NULL, idEvent);
	ULONG CallStackSize=0;
	g_CallStack->GetCount(&CallStackSize);

	wsprintfW(cmd, L"changestack %d", CallStackSize - g_curFrame -1);
	SendCommandToUC(1, cmd);
}
HRESULT CTIStackFrame::GetDocumentContext(IDebugDocumentContext2 **ppCxt)
{
   if (NULL == ppCxt)
   {
	   return E_POINTER;
   }
   *ppCxt = m_spDocContext;
   (*ppCxt)->AddRef();
   g_curFrame = m_frameID;
   if(GetKeyState(VK_LBUTTON)&0x1000)
   {
	   // Not sure which event I need to trap to get a stack change, but this seems to work for now.
	   // Send stack change to Unreal
	   static  UINT_PTR ltimer = 0;
	   KillTimer(NULL, ltimer);
	   ltimer = ::SetTimer(NULL, 123, 200, SetStackTimerProc);
   }


   return S_OK;
}

void CTIStackFrame::Init(IDebugDocumentContext2* pDocContext,IDebugEngine2 *pDebugEngine)
{
    m_spDocContext = pDocContext;
    m_spEngine = pDebugEngine;
}
// TIStackFrame.cpp : Implementation of CEnumFrameInfo

#include "stdafx.h"
#include "StackFrame.h"


// CEnumFrameInfo
void CCallStack::Add( LPCWSTR frame, DWORD line )
{
	g_curFrame = 0;
	INT pos = (INT) m_StackFrames.Add();
	m_StackFrames[pos] = CHeapPtr<CFrameInfoWrapper>(new CFrameInfoWrapper);

	CFrameInfoWrapper& FrameWrapper = *m_StackFrames[pos];

	// allocate our CTIStackFrame
	CComObject<CTIStackFrame>* pFrame=NULL;
	HRESULT hr = CComObject<CTIStackFrame>::CreateInstance(&pFrame);
	if ( FAILED(hr) )
	{
		return;
	}
	pFrame->AddRef();
	pFrame->m_frameID = pos;

	hr = pFrame->QueryInterface(IID_IDebugStackFrame2, (void**)&FrameWrapper->m_pFrame);
	if ( FAILED(hr) )
	{
		// If QueryInterface fails the ref count for pFrame is 0, so we have to delete it.
		delete pFrame;
		return;
	}
	FRAMEINFO_FLAGS fi = FIF_LANGUAGE | FIF_FUNCNAME | FIF_STACKRANGE | FIF_FRAME;
	FrameWrapper->m_dwValidFields = fi;


	// Set File/Line
	CComObject<CDebugContext>* pContext=NULL;
	hr = CComObject<CDebugContext>::CreateInstance(&pContext);
	if (FAILED(hr))
	{
		return;
	}
	pContext->AddRef();

	CComBSTR file = GetFileFromClass(frame);
	FrameWrapper->m_bstrLanguage = SysAllocString(L"UC");
	FrameWrapper->m_bstrFuncName = file.Copy();
	
	TEXT_POSITION posBeg;
	posBeg.dwLine = line-1;
	posBeg.dwColumn = 0;

	pContext->Initialize(file, posBeg);
	pFrame->Init(pContext, g_Engine);

	pContext->Release();
	pFrame->Release();
}

void CCallStack::Clear()
{
	Reset();
	m_StackFrames.RemoveAll();
}

void CCallStack::CopyEnumElement(FRAMEINFO* pDest, FRAMEINFO* pSrc)
{
	if ((NULL != pSrc) && (NULL != pDest) )
	{
		// Start with rough copy.
		*pDest = *pSrc;

		pDest->m_bstrFuncName = SysAllocString(pSrc->m_bstrFuncName);
		pDest->m_bstrLanguage = SysAllocString(pSrc->m_bstrLanguage);

		pDest->m_pFrame = pSrc->m_pFrame;
		if (pDest->m_pFrame)
		{
			pDest->m_pFrame->AddRef();
		}
	}
}

HRESULT CCallStack::GetStackEnum(IDebugEngine2* pEng, IEnumDebugFrameInfo2** ppEnum)
{
	// Create and initialize enumerator.
	return QueryInterface(IID_IEnumDebugFrameInfo2, (void**)ppEnum);
}

BOOL CCallStack::HasNodes() const
{
	return m_StackFrames.GetCount() > 0;
}

//////////////////////////////////////////////////////////////////////////////
// IEnumDebugFrameInfo2
HRESULT CCallStack::Skip(ULONG celt)
{ return E_NOTIMPL; }
HRESULT CCallStack::Clone(IEnumDebugFrameInfo2** ppEnum)
{ return E_NOTIMPL; }

HRESULT CCallStack::GetCount(ULONG* pcelt)
{ 
	if (NULL == pcelt)
	{
		return E_POINTER;
	}

	*pcelt = (ULONG) m_StackFrames.GetCount();
	return S_OK; 
}

HRESULT CCallStack::Reset()
{
	m_NextFrameIdx = 0;
	return S_OK;
}

HRESULT CCallStack::Next(ULONG celt, FRAMEINFO* rgelt, ULONG* pceltFetched)
{ 
	if ( celt == 0 )
	{
		return S_OK;
	}
	if (NULL == rgelt)
	{
		return E_POINTER;
	}
	
	const ULONG frameCount = (ULONG)m_StackFrames.GetCount();
	const ULONG CurrentFrameIdx = m_NextFrameIdx;
	m_NextFrameIdx++;

	if ( CurrentFrameIdx + celt > frameCount )
	{
		celt = frameCount - CurrentFrameIdx;
	}


	for ( ULONG i = 0; i < celt; i++ )
	{
		const ULONG desiredFrame = CurrentFrameIdx+i;
		CopyEnumElement( &rgelt[desiredFrame], *m_StackFrames[frameCount - desiredFrame - 1] );
	}

// 	for ( INT i = celt; i > 0; i-- )
// 	{
// 		CopyEnumElement(&rgelt[celt - i], *m_StackFrames[i - 1]);
// 	}

	if (NULL != pceltFetched)
	{
		*pceltFetched = celt;
	}

	return S_OK; 
}
