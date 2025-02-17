// TIStackFrame.h : Declaration of the CTIStackFrame

#pragma once
#include "resource.h"       // main symbols

#ifdef STANDARDSHELL_UI_MODEL
#include "resource.h"
#endif
#ifdef POCKETPC2003_UI_MODEL
#include "resourceppc.h"
#endif 
#ifdef SMARTPHONE2003_UI_MODEL
#include "resourcesp.h"
#endif
#ifdef AYGSHELL_UI_MODEL
#include "resourceayg.h"
#endif

#include "stdafx.h"
#include "UCDebuggerSDK.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error Single-threaded COM objects are not properly supported on Windows CE platforms that do not include full DCOM support.  Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support them anyway. You must also change the threading model in your rgs file from 'Apartment'/'Single' to 'Free'.
#endif

extern class CCallStack* g_CallStack;

// CTIStackFrame
class ATL_NO_VTABLE CTIStackFrame : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDebugStackFrame2
{
public:
    CTIStackFrame()
    {
    }


BEGIN_COM_MAP(CTIStackFrame)
    COM_INTERFACE_ENTRY(IDebugStackFrame2)
END_COM_MAP()

    ////////////////////////////////////////////////////////////
    // IDebugStackFrame2
    STDMETHOD(GetCodeContext)(IDebugCodeContext2 **ppCodeCxt);
    STDMETHOD(GetDocumentContext)(IDebugDocumentContext2 **ppCxt);
    STDMETHOD(GetName)(BSTR* pbstrName);
    STDMETHOD(GetInfo)(FRAMEINFO_FLAGS dwFieldSpec, UINT nRadix, FRAMEINFO *pFrameInfo);
    STDMETHOD(GetPhysicalStackRange)(UINT64* paddrMin, UINT64* paddrMax);
    STDMETHOD(GetExpressionContext)(IDebugExpressionContext2 **ppExprCxt);
    STDMETHOD(GetLanguageInfo)(BSTR* pbstrLanguage, GUID* pguidLanguage);
    STDMETHOD(GetDebugProperty)(IDebugProperty2 **ppDebugProp);
    STDMETHOD(EnumProperties)(
        DEBUGPROP_INFO_FLAGS dwFields,
        UINT nRadix,
        REFGUID guidFilter,
        DWORD dwTimeout,
        ULONG *pcelt,
        IEnumDebugPropertyInfo2 **ppepi);
    STDMETHOD(GetThread)(IDebugThread2** ppThread);


    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct()
    {
        return S_OK;
    }
    
    void FinalRelease() 
    {
    }

public:

protected:
    CComPtr<IDebugDocumentContext2> m_spDocContext;
    CComPtr<IDebugEngine2> m_spEngine;
public:
	INT m_frameID;
    void Init(IDebugDocumentContext2* pDocContext,IDebugEngine2* pEngine);
};


class CFrameInfoWrapper
{
private:
	FRAMEINFO* pFrameInfo;

public:
	CFrameInfoWrapper() : pFrameInfo(NULL)
	{
		InitializeFrameInfo();
	}
	~CFrameInfoWrapper()
	{
		ReleaseFrameInfo();
	}

	operator FRAMEINFO*()
	{
		return pFrameInfo;
	}
	FRAMEINFO* operator->()
	{
		return pFrameInfo;
	}
	const FRAMEINFO* operator->() const
	{
		return pFrameInfo;
	}
	FRAMEINFO* operator*()
	{
		return pFrameInfo;
	}
	const FRAMEINFO* operator*() const
	{
		return pFrameInfo;
	}

	CFrameInfoWrapper& operator =( const CFrameInfoWrapper& Other )
	{
		if ( IsValid() )
		{
			ReleaseFrameInfo();
			InitializeFrameInfo();
		}

		if ( Other.IsValid() )
		{
			// Start with rough copy.
			*pFrameInfo = **Other;

			pFrameInfo->m_bstrFuncName = SysAllocString(Other->m_bstrFuncName);
			pFrameInfo->m_bstrLanguage = SysAllocString(Other->m_bstrLanguage);

			pFrameInfo->m_pFrame = Other->m_pFrame;
			if ( pFrameInfo->m_pFrame )
			{
				pFrameInfo->m_pFrame->AddRef();
			}
			if (NULL != pFrameInfo->m_pModule)
			{
				pFrameInfo->m_pModule->AddRef();
			}
		}
	}
	bool IsValid() const { return (NULL != pFrameInfo); }

private:
	/** hide copy ctor */
	CFrameInfoWrapper( const CFrameInfoWrapper& Other )
	: pFrameInfo(NULL)
	{}

	void InitializeFrameInfo()
	{
		if ( NULL == pFrameInfo )
		{
			pFrameInfo = (FRAMEINFO*) malloc(sizeof(FRAMEINFO));
		}

		if (NULL != pFrameInfo)
		{
			memset(pFrameInfo, 0, sizeof(FRAMEINFO));
		}
	}
	void ReleaseFrameInfo()
	{
		if (NULL != pFrameInfo)
		{
			// Release the pointers to interface
			if (NULL != pFrameInfo->m_pFrame)
			{
				pFrameInfo->m_pFrame->Release();
			}
			if (NULL != pFrameInfo->m_pModule)
			{
				pFrameInfo->m_pModule->Release();
			}

			// Free the BSTRs
			SysFreeString(pFrameInfo->m_bstrArgs);
			SysFreeString(pFrameInfo->m_bstrFuncName);
			SysFreeString(pFrameInfo->m_bstrLanguage);
			SysFreeString(pFrameInfo->m_bstrModule);
			SysFreeString(pFrameInfo->m_bstrReturnType);

			// Free the memory
			free(pFrameInfo);
			pFrameInfo = NULL;
		}
	}
};

// CEnumFrameInfo

class ATL_NO_VTABLE CCallStack : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IEnumDebugFrameInfo2
{
public:
    CCallStack()
	: m_NextFrameIdx(0)
    {
		if ( g_CallStack == NULL )
		{
			g_CallStack = this;
		}
	}


BEGIN_COM_MAP(CCallStack)
	COM_INTERFACE_ENTRY(IEnumDebugFrameInfo2)
END_COM_MAP()

    ////////////////////////////////////////////////////////////
    // IEnumDebugFrameInfo2
    STDMETHOD(Next)(ULONG celt, FRAMEINFO* rgelt, ULONG* pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)();
    STDMETHOD(Clone)(IEnumDebugFrameInfo2** ppEnum);
    STDMETHOD(GetCount)(ULONG* pcelt);

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct()
    {
       return S_OK;
    }
    
    void FinalRelease() 
    {
		Clear();
		if ( g_CallStack == this )
		{
			g_CallStack = NULL;
		}
    }

public:
	void Add(LPCWSTR frame, DWORD line);
	HRESULT GetStackEnum(IDebugEngine2 *pEng, IEnumDebugFrameInfo2 **ppEnum);
	BOOL HasNodes() const;
	void Clear();

protected:
	CHeapPtrArray<CFrameInfoWrapper>	m_StackFrames;

private:
    void CopyEnumElement(FRAMEINFO* pDest, FRAMEINFO* pSrc);
	ULONG m_NextFrameIdx;
};
