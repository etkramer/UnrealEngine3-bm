// Engine.cpp : Implementation of CEngine

#include "stdafx.h"
#include "Engine.h"
#include "Expression.h"
#include "MemoryProperty.h"
#include "Breakpoint.h"

CEngine *g_Engine = NULL;

//////////////////////////////////////////////////////////////////////////////
//IDebugEngine2 methods
HRESULT CEngine::EnumPrograms(IEnumDebugPrograms2** ppEnum)
{ return E_NOTIMPL; }

HRESULT CEngine::Attach(
    IDebugProgram2 **rgpPrograms,
    IDebugProgramNode2 **rgpProgramNodes,
    DWORD celtPrograms,
    IDebugEventCallback2 *pCallback,
    ATTACH_REASON dwReason)
{
	g_Engine = this;
	g_Engine->AddRef();

	LOG(L"CEngine::Attach");
    GUID guidProgram = GUID_NULL;
    for (DWORD cProgram = 0; cProgram < celtPrograms; cProgram++)
    {
        if ( (NULL != rgpProgramNodes[cProgram]) && (NULL != rgpPrograms[cProgram]) )
        {
            CComPtr<IDebugProviderProgramNode2> srpDebugProviderProgramNode;
            CComPtr<IBatchProgramNode> srpBatchProgramNode;
            // Check if the pointer to IDebugProgramNode is actually a provider; in this case we
            // have to get the pointer to the actual implementation of the interface.
            HRESULT hr = rgpProgramNodes[cProgram]->QueryInterface(IID_IDebugProviderProgramNode2, (void**)&srpDebugProviderProgramNode);
            if ( SUCCEEDED(hr) )
            {
                hr = srpDebugProviderProgramNode->UnmarshalDebuggeeInterface(IID_IBatchProgramNode, (void**)&srpBatchProgramNode);
            }
            else
            {
                hr = rgpProgramNodes[cProgram]->QueryInterface(IID_IBatchProgramNode, (void**)&srpBatchProgramNode);
            }
            
            if ( FAILED(hr) )
                continue;

            srpBatchProgramNode.QueryInterface(&m_spProgram);
            srpBatchProgramNode->EngineAttach(static_cast<IDebugEngine2*>(this), pCallback, dwReason);
        }
    }

    m_spCallback = pCallback;
    return S_OK;
}
HRESULT CEngine::SetException(EXCEPTION_INFO* pException)
{ return E_NOTIMPL; }
HRESULT CEngine::RemoveSetException(EXCEPTION_INFO* pException)
{ return E_NOTIMPL; }
HRESULT CEngine::RemoveAllSetExceptions(REFGUID guidType)
{ return E_NOTIMPL; }
HRESULT CEngine::GetEngineId(GUID *pguidEngine)
{
    if (NULL == pguidEngine)
        return E_POINTER;
    *pguidEngine = __uuidof(Engine);
    return S_OK;
}
HRESULT CEngine::DestroyProgram(IDebugProgram2* pProgram)
{ return E_NOTIMPL; }
HRESULT CEngine::SetLocale(WORD wLangID)
{ return E_NOTIMPL; }
HRESULT CEngine::SetRegistryRoot(LPCOLESTR pszRegistryRoot)
{ return E_NOTIMPL; }
HRESULT CEngine::SetMetric(LPCOLESTR pszMetric, VARIANT varValue)
{ return E_NOTIMPL; }
HRESULT CEngine::CauseBreak(void)
{ return E_NOTIMPL; }

HRESULT CEngine::ContinueFromSynchronousEvent(IDebugEvent2* pEvent)
{ 
    PostThreadMessage(GetCurrentThreadId(), WM_CONTINUE_SYNC_EVENT, 0, 0);
    return S_OK;
}

HRESULT CEngine::CreatePendingBreakpoint(
   IDebugBreakpointRequest2 *pBPRequest, 
   IDebugPendingBreakpoint2** ppPendingBP)
{
	LOG(L"CEngine::CreatePendingBreakpoint");
    if (NULL == ppPendingBP)
        return E_POINTER;
    *ppPendingBP = NULL;

    CComObject<CPendingBreakpoint> *pPending;
    HRESULT hr = CComObject<CPendingBreakpoint>::CreateInstance(&pPending);
    if (FAILED(hr)) return hr;
    pPending->AddRef();

    pPending->Initialize(pBPRequest, m_spCallback, static_cast<IDebugEngine2*>(this));
    hr = pPending->QueryInterface(ppPendingBP);
   
    pPending->Release();
    return hr;
}

//////////////////////////////////////////////////////////////////////////////
//IDebugExpressionContext2 methods.
HRESULT CEngine::GetName(BSTR *pbstrName)
{
	LOG(L"CEngine::GetName");
    HRESULT retval = E_INVALIDARG;

    if (pbstrName != NULL) {
        CComBSTR name = L"UCDebuggerSDK Expression Context";
        *pbstrName = name.Detach();
        retval = S_OK;
    }
    return(retval);
}

HRESULT CEngine::ParseText(LPCOLESTR pszCode,
                           PARSEFLAGS dwFlags,
                           UINT nRadix,
                           IDebugExpression2 **ppExpr,
                           BSTR *pbstrError,
                           UINT *pichError)
{
    HRESULT retval = E_INVALIDARG;
    if (ppExpr != NULL) {
        *ppExpr = NULL;
        // A real debug engine would instantiate a symbol provider here.
        // However, a custom debug engine would have to implement its own
        // symbol provider and that is going way beyond what this sample
        // is meant for.
        //
        // Since we don't provide a symbol provider, we cannot resolve
        // variables of any kind.  This leaves only numerical expressions.
        if (m_spExpressionEvaluator == NULL) {
            CLSID clsidEE = { 0 };
            extern const WCHAR strRegistrationRoot[];
            // A debug engine is supposed to ask the symbol provider for the
            // GUID of the language of the document that contains the current
            // location we are stopped at.  But since we don't have a symbol
            // provider, we hardcode the GUID of the MyCEE sample evaluator.
            //
            // An alternative is if we know the language name, we could try
            // to look up the GUID in the registry under
            // HKLM\
            //   SOFTWARE\
            //     Microsoft\
            //       VisualStudio\
            //         <X.Y>\
            //           Language Services\
            //             <Language Name>\
            //               Debugger Languages\
            //                 <GUID>
            //
            // <X.Y> is the root hive for Visual Studio (e.g., 8.0, 8.0exp)
            // <Language Name> is the exact name of the language
            // <GUID> is the language guid.  Note: this is a registry entry so
            // you will need to enumerate the "Debugger Languages" sub-key.
            static GUID languageGuid = {0x462D4A3E,0xB257,0x4AEE,0x97,0xCD,0x59,0x18,0xC7,0x53,0x17,0x57};
            static GUID vendorGuid   = {0x994B45C4,0xE6E9,0x11D2,0x90,0x3F,0x00,0xC0,0x4F,0xA3,0x02,0xA1};
            // Get the expression evaluator's CLSID from the registry.
            ::GetEEMetric(languageGuid,vendorGuid,metricCLSID,&clsidEE,strRegistrationRoot);
            if (!IsEqualGUID(clsidEE,GUID_NULL)) {
                // Instantiate the expression evaluator
                m_spExpressionEvaluator.CoCreateInstance(clsidEE);
            }
        }
        if (m_spExpressionEvaluator != NULL) {
            if (dwFlags & PARSE_EXPRESSION){
                CComPtr<IDebugParsedExpression> pParsedExpression;
                retval = m_spExpressionEvaluator->Parse(
                    pszCode,
                    dwFlags,
                    nRadix,
                    pbstrError,
                    pichError,
                    &pParsedExpression);
                if (SUCCEEDED(retval)) {
                    CComObject<CExpression> *pExpression;
                    CComObject<CExpression>::CreateInstance(&pExpression);
                    if (pExpression != NULL) {
                        // Bundle the IDebugParsedExpression into our new
                        // IDebugExpression2 object, along with a few items
                        // that will be of use when IDebugExpression2 calls
                        // IDebugParsedExpression::EvaluateSync.
                        CComPtr<IDebugProgram2> srpProgram;
                        m_spProgram.QueryInterface(&srpProgram);
                        CComPtr<IDebugThread2> srpThread;
                        m_spProgram.QueryInterface(&srpThread);
                        pExpression->Init(
                            pParsedExpression,
                            dwFlags,
                            NULL,    // symbol provider
                            NULL,    // binder
                            NULL,    // address
                            m_spCallback,
                            this,
                            srpProgram,
                            srpThread);
                        *ppExpr = pExpression;
                    }
                }

            } else {
                // We don't support anything but parsing
                retval = E_INVALIDARG;
            }
        } else {
            // No evaluator, no parsing
            retval = E_NOTIMPL;
        }
    }

    return(retval);
}

