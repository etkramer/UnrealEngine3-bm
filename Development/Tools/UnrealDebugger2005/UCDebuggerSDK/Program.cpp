// Program.cpp : Implementation of CProgram

#include "stdafx.h"
#include "Program.h"
#include "EventBase.h"
#include "Breakpoint.h"
#include "StackFrame.h"
#include "Context.h"
#include "MemoryBytes.h"
#include "Engine.h"
#include "Breakpoint.h"

CProgram *g_Program = NULL;
DWORD g_mainThread = NULL; // For posting messages back to the main thread.


HANDLE g_VSSocketEvent[2] = { INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };	// read/write
HANDLE g_UCSocketEvent[2] = { INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };	// read/write
// HANDLE g_VSSocketEvent[2] = { INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };
// HANDLE g_UCSocketEvent[2] = { INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };


// CProgram

//////////////////////////////////////////////////////////////////////////////
// IDebugProgramNode2 methods
HRESULT CProgram::GetProgramName(BSTR* pbstrProgramName)
{ return E_NOTIMPL; }
HRESULT CProgram::GetHostName(DWORD dwHostNameType, BSTR* pbstrHostName)
{ return E_NOTIMPL; }
HRESULT CProgram::GetHostMachineName_V7(BSTR* pbstrHostMachineName)
{ return E_NOTIMPL; }
HRESULT CProgram::DetachDebugger_V7(void)
{ return E_NOTIMPL; }

HRESULT CProgram::GetEngineInfo(BSTR* pbstrEngine, GUID* pguidEngine)
{
    if (NULL != pbstrEngine) {
        *pbstrEngine = SysAllocString(L"Text Interpreter DE");
        if (NULL == *pbstrEngine) {
            return E_OUTOFMEMORY;
        }
    }
    if (NULL != pguidEngine) {
        *pguidEngine = __uuidof(Engine);
    }
    return S_OK;
}

HRESULT CProgram::GetHostPid(AD_PROCESS_ID *pHostProcessId)
{
    if (NULL == pHostProcessId)
        return E_POINTER;
    pHostProcessId->ProcessIdType = AD_PROCESS_ID_SYSTEM;
    pHostProcessId->ProcessId.dwProcessId = GetCurrentProcessId();

    return S_OK;
}

HRESULT CProgram::Attach_V7(IDebugProgram2* pMDMProgram,
   IDebugEventCallback2* pCallback, DWORD dwReason)
{ 
	LOG(L"CProgram::Attach_V7");	
	return E_NOTIMPL;
}


//////////////////////////////////////////////////////////////////////////////
// IDebugProgram2 methods
HRESULT CProgram::EnumThreads(IEnumDebugThreads2** ppEnum)
{ return E_NOTIMPL; }
HRESULT CProgram::GetName(BSTR* pbstrName)
{ return E_NOTIMPL; }
HRESULT CProgram::GetProcess(IDebugProcess2** ppProcess)
{ return E_NOTIMPL; }
HRESULT CProgram::Terminate(void)
{ 
	SendCommandToUC(1, L"terminate");
	return Detach();
}
HRESULT CProgram::Attach(IDebugEventCallback2* pCallback)
{ return E_NOTIMPL; }
HRESULT CProgram::Detach(void)
{ 
	NotifyProgramEnd();
	return S_OK;
}
HRESULT CProgram::GetDebugProperty(IDebugProperty2** ppProperty)
{ return E_NOTIMPL; }
HRESULT CProgram::CauseBreak(void)
{ 
	SendCommandToUC(1, L"break");
	return S_OK; 
}
HRESULT CProgram::EnumCodeContexts(IDebugDocumentPosition2* pDocPos, IEnumDebugCodeContexts2** ppEnum)
{ return E_NOTIMPL; }
HRESULT CProgram::GetDisassemblyStream(DISASSEMBLY_STREAM_SCOPE dwScope, IDebugCodeContext2* pCodeContext, IDebugDisassemblyStream2** ppDisassemblyStream)
{ return E_NOTIMPL; }
HRESULT CProgram::EnumModules(IEnumDebugModules2** ppEnum)
{ return E_NOTIMPL; }
HRESULT CProgram::GetENCUpdate(IDebugENCUpdate** ppUpdate)
{ return E_NOTIMPL; }
HRESULT CProgram::EnumCodePaths(LPCOLESTR pszHint, IDebugCodeContext2* pStart, IDebugStackFrame2* pFrame, BOOL fSource, IEnumCodePaths2** ppEnum, IDebugCodeContext2** ppSafety)
{ return E_NOTIMPL; }
HRESULT CProgram::WriteDump(DUMPTYPE DumpType,LPCOLESTR pszCrashDumpUrl)
{ return E_NOTIMPL; }
HRESULT CProgram::CanDetach(void)
{ return E_NOTIMPL; }

HRESULT CProgram::GetMemoryBytes(IDebugMemoryBytes2** ppMemoryBytes)
{
    HRESULT retval = E_POINTER;
    if (ppMemoryBytes != NULL) {
        CComObject<CMemoryBytes> *pMemoryBytes;
        CComObject<CMemoryBytes>::CreateInstance(&pMemoryBytes);
        if (pMemoryBytes != NULL) {
            retval = pMemoryBytes->QueryInterface(__uuidof(IDebugMemoryBytes2),
                                                  (void **)ppMemoryBytes);
        }
    }
    return retval;
}

HRESULT CProgram::GetProgramId(GUID* pguidProgramId)
{ 
	LOG(L"CProgram::GetProgramId");
    if (NULL == pguidProgramId)
        return E_POINTER;
    *pguidProgramId = m_guidProgId;
    return S_OK;
}

HRESULT CProgram::Execute(void)
{ 
	LOG(L"CProgram::Execute");
	m_inBreak = FALSE;
	SendCommandToUC(1, L"go");

	PostDbgCmd(WM_CONTINUE);
	return S_OK; 
}
HRESULT CProgram::Continue(IDebugThread2 *pThread)
{
	LOG(L"CProgram::Continue");
	m_inBreak = FALSE;
	SendCommandToUC(1, L"go");
	PostDbgCmd(WM_CONTINUE);
   return S_OK; 

}
HRESULT CProgram::Step(IDebugThread2 *pThread, STEPKIND sk, STEPUNIT step)
{
	LOG(L"CProgram::Step");
	m_inBreak = FALSE;
	if(sk == STEP_INTO)
	{
		SendCommandToUC(1, L"stepinto");
	}
	else if(sk == STEP_OVER)
	{
		SendCommandToUC(1, L"stepover");
	}
	else if(sk == STEP_OUT)
	{
		SendCommandToUC(1, L"stepoutof");
	}
	else // if(sk == STEP_BACKWARDS)
	{
		return E_NOTIMPL;
	}

	PostThreadMessage(GetCurrentThreadId(), WM_CONTINUE, 0, 0);
	return S_OK; 
}

//////////////////////////////////////////////////////////////////////////////
//IDebugThread2 methods
HRESULT CProgram::SetThreadName(LPCOLESTR pszName)
{ return E_NOTIMPL; }
HRESULT CProgram::GetProgram(IDebugProgram2** ppProgram)
{ return E_NOTIMPL; }
HRESULT CProgram::CanSetNextStatement(
    IDebugStackFrame2 *pStackFrame, IDebugCodeContext2 *pCodeContext)
{ return E_NOTIMPL; }
HRESULT CProgram::SetNextStatement(
    IDebugStackFrame2 *pStackFrame, IDebugCodeContext2 *pCodeContext)
{ return E_NOTIMPL; }
HRESULT CProgram::Suspend(DWORD *pdwSuspendCount)
{ return E_NOTIMPL; }
HRESULT CProgram::Resume(DWORD *pdwSuspendCount)
{ return E_NOTIMPL; }
HRESULT CProgram::GetThreadProperties(
    THREADPROPERTY_FIELDS dwFields, THREADPROPERTIES *ptp)
{ return E_NOTIMPL; }
HRESULT CProgram::GetLogicalThread(
    IDebugStackFrame2 *pStackFrame, IDebugLogicalThread2 **ppLogicalThread)
{ return E_NOTIMPL; }

HRESULT CProgram::EngineAttach(
    IDebugEngine2*        pEngine,
    IDebugEventCallback2* pCallback, 
    DWORD                 dwReason)
{
//	MessageBox(NULL, _T("CProgram EngineAttach"), _T("Debug Message Box") ,MB_OK);
	LOG(L"CProgram::EngineAttach");
	LOG(L"Attach");
	m_inBreak = FALSE;
	bAttached = true;

    // Store the pointer to the engine.
    if (NULL == pEngine)
	{
		return E_POINTER;
	}
    
	m_spEngine = pEngine;
	g_Program = this;
	g_mainThread = GetCurrentThreadId();

	if(!g_pUCDebgger)
	{
		g_pUCDebgger = new IPC_VS;
	}

    // Store the pointer to the callback interface
    m_spCallback = pCallback;

	HRESULT hr = S_OK;
	if ( m_CallStack == NULL )
	{
		hr = CComObject<CCallStack>::CreateInstance(&m_CallStack);
		m_CallStack->AddRef();
	}

	CEngineCreateEvent *pDECE = new CEngineCreateEvent(m_spEngine);
    if (NULL == pDECE)
	{
		return E_OUTOFMEMORY;
	}
    pDECE->SendEvent(pCallback, m_spEngine, NULL, NULL);

    CProgramCreateEvent *pDPCE = new CProgramCreateEvent;
    if (NULL == pDPCE)
	{
		return E_OUTOFMEMORY;
	}
    pDPCE->SendEvent(pCallback, m_spEngine, this, NULL);

    CLoadCompleteEvent *pDLCE = new CLoadCompleteEvent;
    if (NULL == pDLCE)
	{
		return E_OUTOFMEMORY;
	}
    pDLCE->SendEvent(pCallback, m_spEngine, this, this);

    return hr;
}

HRESULT CProgram::GetThreadId(DWORD* pdwThreadId)
{
    if (NULL == pdwThreadId)
	{
		return E_POINTER;
	}
    *pdwThreadId = GetCurrentThreadId();
    return S_OK;
}

HRESULT CProgram::EnumFrameInfo(FRAMEINFO_FLAGS dwFieldSpec, UINT nRadix, IEnumDebugFrameInfo2 **ppEnum)
{
	LOG(L"NotifyProgramEnd");
	if( !g_CallStack->HasNodes() )
	{
		LOG(L"Error: no stack");
	}
	if (NULL == ppEnum)
	{
		return E_POINTER;
	}
	return g_CallStack->GetStackEnum(m_spEngine, ppEnum);
}

HRESULT CProgram::OnAttach(REFGUID guidProgramId)
{
	LOG(L"CProgram::OnAttach");
    HRESULT hr = S_OK;
    m_guidProgId = guidProgramId;

    return hr;
}

//////////////////////////////////////////////////////////////////////////////
// CProgram methods.
void CProgram::Start()
{
	LOG(L"CProgram::Start");
    HRESULT hr = m_srpProgramPublisher.CoCreateInstance(CLSID_ProgramPublisher);
    if ( FAILED(hr) )
    {
		WCHAR msg[256];
		wsprintfW(msg, L"Failed to create the program publisher: 0x%x.", hr);
        LOG(msg);
        return;
    }

    hr = m_srpProgramPublisher->PublishProgramNode(
        static_cast<IDebugProgramNode2*>(this));
    if ( FAILED(hr) )
    {
		WCHAR msg[256];
		wsprintfW(msg, L"Failed to publish the program node: 0x%x.", hr);
		LOG(msg);
        m_srpProgramPublisher.Release();
        return;
    }

    LOG(L"Added program node.\n");
}
void CProgram::Stop()
{
}

IDebugEventCallback2 *CProgram::GetCallback()
{
    return(m_spCallback);
}


void CProgram::Go(void)
{
}

void CProgram::NotifyProgramEnd(void)
{
	if ( !bAttached )
		return;

	g_CallStack = NULL;
	if ( m_CallStack != NULL )
	{
		m_CallStack->Release();
	}
	m_CallStack = NULL;

    CProgramDestroyEvent *pProgramDestroy = new CProgramDestroyEvent(0);
    if (NULL == pProgramDestroy)
        return;
    pProgramDestroy->SendEvent(m_spCallback, m_spEngine, this, NULL);
    if (m_srpProgramPublisher != NULL)
    {
        m_srpProgramPublisher->
            UnpublishProgramNode(static_cast<IDebugProgramNode2*>(this));
        m_srpProgramPublisher.Release();
    }

//	m_spEngine.Release();
//	m_spCallback.Release();

	bAttached = false;
	if ( m_spCallback )
	{
		LOG(L"CProgram::NotifyProgramEnd -> m_spCallback is STILL REFERENCED");
	}
	if ( m_spEngine )
	{
		LOG(L"CProgram::NotifyProgramEnd -> m_spEngine is STILL REFERENCED");
	}
}
HRESULT CProgram::CreatePendingBreakpoint(IDebugBreakpointRequest2 *pBPRequest, IDebugPendingBreakpoint2 **ppPendingBP)
{
	OutputDebugMessage(L"CreatePendingBreakpoint");

	CComObject<CPendingBreakpoint> *pPending;
	CComObject<CPendingBreakpoint>::CreateInstance(&pPending);
	pPending->AddRef();

	pPending->Initialize(pBPRequest, m_spCallback, m_spEngine);
	pPending->QueryInterface(ppPendingBP);

	pPending->Release();
	return S_OK;

}

CProgram::CProgram() : bAttached(false), bEntered(false), m_CallStack(NULL)
{
	bAttached = false;
	bEntered = false;
}
void CProgram::NotifyBreakpointHit()
{
	LOG(L"NotifyBreakpointHit");
	if(!m_inBreak)
	{
		LOG(L"NotifyBreakpointHit");
		CDebugBreakEvent *pCBPE = new CDebugBreakEvent();
		if (NULL != pCBPE)
		{
			pCBPE->SendEvent(m_spCallback, m_spEngine, this, this);
		}
		m_inBreak = TRUE;
	}
}
void CProgram::OutputDebugMessage( LPCWSTR msg )
{
	if ( bAttached && m_spEngine && m_spEngine )
	{
		CComBSTR line(msg);
		line += "\n";

		CDebugOutputStringEvent2 *pCBPE = new CDebugOutputStringEvent2(line);
		if (NULL != pCBPE)
		{
			pCBPE->SendEvent(m_spCallback, m_spEngine, this, this);
		}
	}
}
