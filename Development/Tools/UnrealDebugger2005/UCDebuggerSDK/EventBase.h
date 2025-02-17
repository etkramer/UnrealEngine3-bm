#pragma once
#include "msdbg.h"

// IUnknown methods declaration macro
#define DECLARE_IUNKNOWN \
    STDMETHOD_(ULONG, AddRef)(void); \
    STDMETHOD_(ULONG, Release)(void); \
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObj);

class CEventBase :
	public IDebugEvent2
{
public:
	CEventBase(REFIID iid, DWORD dwAttrib);
	virtual ~CEventBase(void);

	////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugEvent2 methods
    STDMETHOD(GetAttributes)(DWORD* pdwAttrib);

protected:
	DWORD m_dwAttrib;
	IID m_riid;
	unsigned long m_iRefCount;
public:
	HRESULT SendEvent(IDebugEventCallback2 * pCallback, IDebugEngine2 * pEngine, IDebugProgram2 * pProgram, IDebugThread2 * pThread);
};

class CEngineCreateEvent :
    public IDebugEngineCreateEvent2,
    public CEventBase
{
public:
    CEngineCreateEvent(IDebugEngine2 *pEngine);
	~CEngineCreateEvent(void);

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    DECLARE_IUNKNOWN

	////////////////////////////////////////////////////////////
    // IDebugEngineCreateEvent2 methods
    STDMETHOD(GetEngine)(IDebugEngine2 **ppEngine);

protected:
	CComPtr<IDebugEngine2> m_spEngine;
};

class CProgramCreateEvent :
    public IDebugProgramCreateEvent2,
    public CEventBase
{
public:
	CProgramCreateEvent(void);
	~CProgramCreateEvent(void);

	////////////////////////////////////////////////////////////
    // IUnknown methods
    DECLARE_IUNKNOWN

};

class CLoadCompleteEvent :
    public IDebugLoadCompleteEvent2,
    public CEventBase
{
public:
	CLoadCompleteEvent(void);
	~CLoadCompleteEvent(void);
    
	////////////////////////////////////////////////////////////
    // IUnknown methods
    DECLARE_IUNKNOWN
};

class CEntryPointEvent :
    public IDebugEntryPointEvent2,
    public CEventBase
{
public:
	CEntryPointEvent(void);
	~CEntryPointEvent(void);
    
	////////////////////////////////////////////////////////////
    // IUnknown methods
    DECLARE_IUNKNOWN
};

class CBreakpointBoundEvent :
    public IDebugBreakpointBoundEvent2,
    public CEventBase
{
public:
    CBreakpointBoundEvent(
        IEnumDebugBoundBreakpoints2 *pEnum,
        IDebugPendingBreakpoint2 *pPending);
	~CBreakpointBoundEvent(void);

	////////////////////////////////////////////////////////////
    // IUnknown methods
    DECLARE_IUNKNOWN

    ////////////////////////////////////////////////////////////
    //IDebugBreakpointBoundEvent2 methods
    STDMETHOD(GetPendingBreakpoint)(IDebugPendingBreakpoint2** ppPendingBP);
    STDMETHOD(EnumBoundBreakpoints)(IEnumDebugBoundBreakpoints2** ppEnum);

protected:
	CComPtr<IEnumDebugBoundBreakpoints2> m_spEnum;
	CComPtr<IDebugPendingBreakpoint2> m_spPending;
};

class CCodeBreakpointEvent :
    public IDebugBreakpointEvent2,
    public CEventBase
{
public:
    CCodeBreakpointEvent(IDebugBoundBreakpoint2 *pBP);
	~CCodeBreakpointEvent(void);

	////////////////////////////////////////////////////////////
    // IUnknown methods
    DECLARE_IUNKNOWN

    ////////////////////////////////////////////////////////////
    //IDebugBreakpointEvent2 methods
    STDMETHOD(EnumBreakpoints)(IEnumDebugBoundBreakpoints2 **ppEnum);
protected:
	CComPtr<IDebugBoundBreakpoint2> m_spBP;
};

class CProgramDestroyEvent :
    public IDebugProgramDestroyEvent2,
    public CEventBase
{
public:
    CProgramDestroyEvent(DWORD dwExitCode);
	~CProgramDestroyEvent(void);

	/////////////////////////////////////
    // IUnknown methods
    DECLARE_IUNKNOWN

    /////////////////////////////////////
    // IDebugProgramDestroyEvent2 methods
    STDMETHOD(GetExitCode)(DWORD *pdwExit);
protected:
	DWORD m_dwExitCode;
};

class CExpressionEvaluationCompleteEvent :
	public IDebugExpressionEvaluationCompleteEvent2,
	public CEventBase
{
public:
	CExpressionEvaluationCompleteEvent(IDebugExpression2 *pExpression, IDebugProperty2 *pResult);
	~CExpressionEvaluationCompleteEvent(void);

	/////////////////////////////////////
    // IUnknown methods
    DECLARE_IUNKNOWN

    /////////////////////////////////////
    // IDebugExpressionEvaluationCompleteEvent2 methods
	STDMETHOD(GetExpression)(IDebugExpression2 **ppExpr);
	STDMETHOD(GetResult)(IDebugProperty2 **ppResult);
protected:
	CComPtr<IDebugExpression2> m_spExpression;
	CComPtr<IDebugProperty2> m_spResult;
};

class CDebugOutputStringEvent2 : public IDebugOutputStringEvent2, public CEventBase  
{
public:
	CDebugOutputStringEvent2(LPCWSTR str)
		: CEventBase(IID_IDebugOutputStringEvent2, EVENT_ASYNCHRONOUS)
	{
		this->str = str;
	}
	virtual ~CDebugOutputStringEvent2()
	{}

	// IUnknown methods
	DECLARE_IUNKNOWN

	// IDebugEngineCreateEvent2 methods
	STDMETHOD(GetString)(BSTR *pbstrString)
	{
		*pbstrString = SysAllocString(str);
		return S_OK;
	}

protected:
	CComBSTR str;
};
class CDebugBreakEvent :
	public IDebugBreakEvent2,
	public CEventBase  
{
public:
	CDebugBreakEvent(void)
		: CEventBase(IID_IDebugBreakEvent2, EVENT_SYNC_STOP)
	{
		// 	LOGLINE();
	}

	CDebugBreakEvent::~CDebugBreakEvent(void)
	{
		// 	LOGLINE();
	}

	////////////////////////////////////////////////////////////
	// IUnknown methods
	DECLARE_IUNKNOWN

};
