// Expression.cpp : Implementation of CExpression

#include "stdafx.h"
#include "Expression.h"
#include "eventbase.h"
#include "MemoryProperty.h"

// CExpression

HRESULT CExpression::Abort()
{ return E_NOTIMPL; }
HRESULT CExpression::EvaluateAsync(EVALFLAGS dwFlags, IDebugEventCallback2 *pExprCallback) 
{
    HRESULT retval = E_INVALIDARG;
    if (pExprCallback == NULL) {
        if (m_spParsedExpression != NULL) {
            CComPtr<IDebugProperty2> pResult = NULL;
            retval = m_spParsedExpression->EvaluateSync(dwFlags,
                                                        INFINITE,
                                                        m_spSymbolProvider,
                                                        m_spAddress,
                                                        m_spBinder,
                                                        NULL,
                                                        &pResult);
            
            if (SUCCEEDED(retval)) {
                if (m_spCallback != NULL) {
                    CExpressionEvaluationCompleteEvent *pEvent;
                    pEvent = new CExpressionEvaluationCompleteEvent(this,pResult);
                    if (NULL == pEvent)
                        retval = E_OUTOFMEMORY;
                    else
                        pEvent->SendEvent(m_spCallback,m_spEngine,m_spProgram,m_spThread);
                }
            }
        }
    }
    return retval;
}

HRESULT CExpression::EvaluateSync(EVALFLAGS dwFlags,
                                  DWORD dwTimeout,
                                  IDebugEventCallback2 *pExprCallback,
                                  IDebugProperty2 **ppResult) 
{
    HRESULT retval = E_INVALIDARG;
    if (pExprCallback == NULL) {
        retval = E_POINTER;
        if (ppResult != NULL) {
            IDebugProperty2 *pProperty = NULL;
            if (m_spParsedExpression != NULL) {
                retval = m_spParsedExpression->EvaluateSync(dwFlags,
                                                            dwTimeout,
                                                            m_spSymbolProvider,
                                                            m_spAddress,
                                                            m_spBinder,
                                                            NULL,
                                                            &pProperty);

                if (SUCCEEDED(retval) && (m_parseFlags & PARSE_FUNCTION_AS_ADDRESS))
                {
                    // Evaluating a memory address so create a version of the
                    // IDebugProperty that will contain a memory context for
                    // the address.
                    CComObject<CMemoryProperty> *pMemoryProperty = NULL;
                    retval = CComObject<CMemoryProperty>::CreateInstance(&pMemoryProperty);
                    if ( SUCCEEDED(retval) && (pMemoryProperty != NULL) ) {
                        pMemoryProperty->AddRef();
                        DEBUG_PROPERTY_INFO dpi = { 0 };
                        // Get the evaluated address
                        retval = pProperty->GetPropertyInfo(DEBUGPROP_INFO_VALUE,10,INFINITE,NULL,0,&dpi);
                        if (SUCCEEDED(retval)) {
                            pMemoryProperty->Init(dpi.bstrValue,NULL);
                            pProperty->Release();    // throw out the property returned by the ee
                            retval = pMemoryProperty->QueryInterface(__uuidof(IDebugProperty2),(void **)&pProperty);
                        }
                        pMemoryProperty->Release();
                    }
                }
                *ppResult = pProperty;
            }
        }
    }
    return retval;
}

void CExpression::Init(IDebugParsedExpression *pParsedExpression,
                       PARSEFLAGS parseFlags,
                       IDebugSymbolProvider *pProvider,
                       IDebugBinder *pBinder,
                       IDebugAddress *pAddress,
                       IDebugEventCallback2 *pCallback,
                       IDebugEngine2 *pEngine,
                       IDebugProgram2 *pProgram,
                       IDebugThread2 *pThread)
{
    m_spParsedExpression = pParsedExpression;
    m_spSymbolProvider = pProvider;
    m_spBinder = pBinder;
    m_spAddress = pAddress;
    m_spCallback = pCallback;
    m_spEngine = pEngine;
    m_spProgram = pProgram;
    m_spThread = pThread;
    m_parseFlags = parseFlags;
}
