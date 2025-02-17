// Program.h : Declaration of the CProgram

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

#include "UCDebuggerSDK.h"
#include "Breakpoint.h"
#include "StackFrame.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error Single-threaded COM objects are not properly supported on Windows CE platforms that do not include full DCOM support.  Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support them anyway. You must also change the threading model in your rgs file from 'Apartment'/'Single' to 'Free'.
#endif

// CProgram

class ATL_NO_VTABLE CProgram : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDebugProgram2,
    public IDebugThread2,
    public IDebugProgramNode2,
    public IDebugProgramNodeAttach2,
    public IBatchProgramNode
{
public:
	CProgram();


BEGIN_COM_MAP(CProgram)
    COM_INTERFACE_ENTRY(IDebugProgram2)
    COM_INTERFACE_ENTRY(IDebugThread2)
    COM_INTERFACE_ENTRY(IDebugProgramNode2)
    COM_INTERFACE_ENTRY(IDebugProgramNodeAttach2)
    COM_INTERFACE_ENTRY(IBatchProgramNode)
END_COM_MAP()

    ////////////////////////////////////////////////////////////
    // IDebugProgramNode2
    STDMETHOD(GetProgramName)(BSTR* pbstrProgramName);
    STDMETHOD(GetHostName)(DWORD dwHostNameType, BSTR* pbstrHostName);
    STDMETHOD(GetHostPid)(AD_PROCESS_ID *pHostProcessId);
    STDMETHOD(GetHostMachineName_V7)(BSTR* pbstrHostMachineName);
    STDMETHOD(Attach_V7)(
        IDebugProgram2* pMDMProgram, IDebugEventCallback2* pCallback, 
        DWORD dwReason);
    STDMETHOD(GetEngineInfo)(BSTR* pbstrEngine, GUID* pguidEngine);
    STDMETHOD(DetachDebugger_V7)(void);

    ////////////////////////////////////////////////////////////
    // IDebugProgram2
    STDMETHOD(EnumThreads)(IEnumDebugThreads2** ppEnum);
    STDMETHOD(GetName)(BSTR* pbstrName);
    STDMETHOD(GetProcess)(IDebugProcess2** ppProcess);
    STDMETHOD(Terminate)(void);
    STDMETHOD(Attach)(IDebugEventCallback2* pCallback);
    STDMETHOD(Detach)(void);
    STDMETHOD(GetProgramId)(GUID* pguidProgramId);
    STDMETHOD(GetDebugProperty)(IDebugProperty2** ppProperty);
    STDMETHOD(Execute)(void);
    STDMETHOD(Continue)(IDebugThread2 *pThread);
    STDMETHOD(Step)(IDebugThread2 *pThread, STEPKIND sk, STEPUNIT step);
    STDMETHOD(CauseBreak)(void);
    //GetEngineInfo already defined for IDebugProgramNode2
    STDMETHOD(EnumCodeContexts)(
        IDebugDocumentPosition2* pDocPos,
        IEnumDebugCodeContexts2** ppEnum);
    STDMETHOD(GetMemoryBytes)(IDebugMemoryBytes2** ppMemoryBytes);
    STDMETHOD(GetDisassemblyStream)(
        DISASSEMBLY_STREAM_SCOPE dwScope,
        IDebugCodeContext2* pCodeContext,
        IDebugDisassemblyStream2** ppDisassemblyStream);
    STDMETHOD(EnumModules)(IEnumDebugModules2** ppEnum);
    STDMETHOD(GetENCUpdate)(IDebugENCUpdate** ppUpdate);
    STDMETHOD(EnumCodePaths)(
        LPCOLESTR pszHint,
        IDebugCodeContext2* pStart,
        IDebugStackFrame2* pFrame,
        BOOL fSource,
        IEnumCodePaths2** ppEnum,
        IDebugCodeContext2** ppSafety);
    STDMETHOD(WriteDump)(DUMPTYPE DumpType, LPCOLESTR pszCrashDumpUrl);
    STDMETHOD(CanDetach)(void);

    ////////////////////////////////////////////////////////////
    // IDebugThread2
    STDMETHOD(EnumFrameInfo)(
        FRAMEINFO_FLAGS dwFieldSpec,
        UINT nRadix,
        IEnumDebugFrameInfo2 **ppEnum);
    // GetName already defined for IDebugProgram
    STDMETHOD(SetThreadName)(LPCOLESTR pszName);
    STDMETHOD(GetProgram)(IDebugProgram2** ppProgram);
    STDMETHOD(CanSetNextStatement)(
        IDebugStackFrame2 *pStackFrame,
        IDebugCodeContext2 *pCodeContext);
    STDMETHOD(SetNextStatement)(
        IDebugStackFrame2 *pStackFrame,
        IDebugCodeContext2 *pCodeContext);
    STDMETHOD(GetThreadId)(DWORD* pdwThreadId);
    STDMETHOD(Suspend)(DWORD *pdwSuspendCount);
    STDMETHOD(Resume)(DWORD *pdwSuspendCount);
    STDMETHOD(GetThreadProperties)(
        THREADPROPERTY_FIELDS dwFields,
        THREADPROPERTIES *ptp);
    STDMETHOD(GetLogicalThread)(
        IDebugStackFrame2 *pStackFrame,
        IDebugLogicalThread2 **ppLogicalThread);

    ////////////////////////////////////////////////////////////
    // IDebugProgramNodeAttach2
    STDMETHOD(OnAttach)(
        REFGUID guidProgramId);

    ////////////////////////////////////////////////////////////
    // IBatchProgramNode
    STDMETHOD(EngineAttach)(
        IDebugEngine2*        pEngine,
        IDebugEventCallback2* pCallback, 
        DWORD                 dwReason);

	DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct()
    {
        _ASSERTE( !gBoundBreakpoint );
        gBoundBreakpoint = NULL;
#ifdef _USE_MARSHALLED_CALLBACK
        m_pInterfaceStream = NULL;
#endif
        return S_OK;
    }
	
    void FinalRelease() 
    {
        gBoundBreakpoint.Clear();
    }

public:

    void Start();
	void Stop();
    IDebugEventCallback2 *GetCallback();
protected:
    CComPtr<IDebugEngine2> m_spEngine;
    CComPtr<IDebugProgramPublisher2> m_srpProgramPublisher;
    GUID m_guidProgId;
    CComPtr<IDebugEventCallback2> m_spCallback;
    CComPtr<IGlobalInterfaceTable> m_spGIT;
	CComObject<CCallStack>* m_CallStack;

#ifdef _USE_MARSHALLED_CALLBACK
    IStream *m_pInterfaceStream;	// used for marshalling callback pointer
    CComPtr<IDebugEventCallback2> m_spMarshalledCallback;
    HRESULT SendCallbackInterface();
    HRESULT ReceiveCallbackInterface();
#endif

public:
    void Go(void);
	HRESULT CreatePendingBreakpoint(IDebugBreakpointRequest2 *pBPRequest, IDebugPendingBreakpoint2** ppPendingBP);
protected:
    bool bAttached;
    bool bEntered;
public:
    void NotifyProgramEnd(void);
	void NotifyBreakpointHit();
	void OutputDebugMessage( LPCWSTR msg );
	int m_inBreak;
};

