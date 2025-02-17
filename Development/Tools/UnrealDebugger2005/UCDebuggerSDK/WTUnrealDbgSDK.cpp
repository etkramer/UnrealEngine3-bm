// WTUnrealDbgSDK.cpp : Implementation of WinMain


#include "stdafx.h"
#include "resource.h"
#include "UCDebuggerSDK.h"
#include "Engine.h"
#include "Program.h"

extern const WCHAR strRegistrationRoot[] = L"Software\\Microsoft\\VisualStudio\\8.0"; 
extern HANDLE g_VSSocketEvent[2];	// read/write
extern HANDLE g_UCSocketEvent[2];	// read/write

#ifdef _USE_MONITOR_THREAD
static DWORD gdwMonitorThreadID;    // id of monitor thread

DWORD GetMonitorThreadId()
{
    return gdwMonitorThreadID;
}
#endif


class CWTUnrealDbgSDKModule : public CAtlExeModuleT< CWTUnrealDbgSDKModule >
{
private:
    typedef CAtlExeModuleT<CWTUnrealDbgSDKModule> base;
public :
    DECLARE_LIBID(LIBID_UCDebuggerSDKLib)
    DECLARE_REGISTRY_APPID_RESOURCEID(IDR_WTUnrealDbgSDK, "{9962D938-D691-4822-A735-E5DD5BADA019}")
    static DWORD WINAPI MonitorProc(void * pv);
    void MonitorShutdown(void);
    HANDLE StartMonitor();
    HRESULT RegisterServer(BOOL bRegTypeLib = 0, const CLSID * pCLSID = 0);
    HRESULT UnregisterServer(BOOL bUnRegTypeLib, const CLSID * pCLSID = 0);
};

CWTUnrealDbgSDKModule _AtlModule;


const DWORD dwPause = 1000; // time to wait for threads to finish up

LPCTSTR FindOneOf(LPCTSTR p1, LPCTSTR p2)
{
    while (p1 != NULL && *p1 != NULL)
    {
        LPCTSTR p = p2;
        while (p != NULL && *p != NULL)
        {
            if (*p1 == *p)
			{
				return CharNext(p1);
			}
            p = CharNext(p);
        }
        p1 = CharNext(p1);
    }
    return NULL;
}
//
extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, 
                                LPTSTR lpCmdLine, int /*nShowCmd*/)
{
	// Uncomment the messagebox so it stops while we attach with remote debugger
//  	MessageBox(NULL, _T("UCDebuggerSDK starting"), _T("UCDebuggerSDK") ,MB_OK);
// 	g_WTGlobals.OpenLog(L"UCDebuggerSDK");
	LOG(L"UCDebuggerSDK");
    lpCmdLine = GetCommandLine(); //this line necessary for _ATL_MIN_CRT

#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
    HRESULT hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
    HRESULT hRes = CoInitialize(NULL);
#endif
    _ASSERTE(SUCCEEDED(hRes));

    TCHAR szTokens[] = _T("-/");

    int nRet = 0;
    BOOL bPrompt = FALSE;
    BOOL bRun = TRUE;
    LPCTSTR lpszToken = FindOneOf(lpCmdLine, szTokens);
    while (lpszToken != NULL)
    {
        if (lstrcmpi(lpszToken, _T("UnregServer"))==0)
        {
            _AtlModule.UpdateRegistryFromResource(IDR_WTUnrealDbgSDK, FALSE);
            nRet = _AtlModule.UnregisterServer(TRUE);
            bRun = FALSE;
            break;
        }
        if (lstrcmpi(lpszToken, _T("RegServer"))==0)
        {
            _AtlModule.UpdateRegistryFromResource(IDR_WTUnrealDbgSDK, TRUE);
            nRet = _AtlModule.RegisterServer(FALSE);
            bRun = FALSE;
            break;
        }
        if (lstrcmpi(lpszToken, _T("Prompt")) == 0)
        {
            bPrompt = TRUE;
        }
    
        lpszToken = FindOneOf(lpszToken, szTokens);
    }
    if (bRun)
    {
        if (bPrompt)
        {
            // This is used to allow us a chance to attach to this program
            // before any significant functionality is started.  After
            // attaching, click the OK button to proceed to the first break
            // point.
            MessageBox(NULL, _T("UCDebuggerSDK starting"), _T("UCDebuggerSDK") ,MB_OK);
        }
        _AtlModule.StartMonitor();
#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
        hRes = _AtlModule.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE | REGCLS_SUSPENDED);
        _ASSERTE(SUCCEEDED(hRes));
        hRes = CoResumeClassObjects();
#else
        hRes = _AtlModule.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE);
#endif
        _ASSERTE(SUCCEEDED(hRes));

        MSG msg;
        while (GetMessage(&msg, 0, 0, 0))
		{
			DispatchMessage(&msg);
		}

		_AtlModule.RevokeClassObjects();
        Sleep(dwPause); //wait for any threads to finish
    }

    _AtlModule.Term();
    CoUninitialize();
    return nRet;
}

HANDLE CWTUnrealDbgSDKModule::StartMonitor()
{
    m_hEventShutdown = ::CreateEvent(NULL, false, false, NULL);
    if (m_hEventShutdown == NULL)
    {
        return NULL;
    }
#ifdef _USE_MONITOR_THREAD
    HANDLE hThread = ::CreateThread(NULL, 0, MonitorProc, this, 0, &gdwMonitorThreadID);
#else
    DWORD dwThreadID;
    HANDLE hThread = ::CreateThread(NULL, 0, MonitorProc, this, 0, &dwThreadID);
#endif
    if(hThread==NULL)
    {
        ::CloseHandle(m_hEventShutdown);
    }

	return hThread;
}

typedef struct tagTHREADNAME_INFO
{
	DWORD dwType;		// must be 0x1000
	LPCSTR szName;		// pointer to name (in user addr space)
	DWORD dwThreadID;	// thread ID (-1=caller thread)
	DWORD dwFlags;		// reserved for future use, must be zero
} THREADNAME_INFO;

void SetThreadName( DWORD dwThreadID, LPCSTR szThreadName )
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = szThreadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	// set thread name
	__try
	{
		RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD), (DWORD*)&info );
	}
	__except(EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}

DWORD WINAPI CWTUnrealDbgSDKModule::MonitorProc(void * pv)
{
	LOG(L"MonitorProc");
    if (NULL == pv)
        return 0;
    CWTUnrealDbgSDKModule* p = static_cast<CWTUnrealDbgSDKModule*>(pv);
    CoInitialize(NULL);

	SetThreadName(GetCurrentThreadId(), "Monitor Thread");

    CComObject<CProgram> *pProgram;
    if (FAILED(CComObject<CProgram>::CreateInstance(&pProgram)))
    {
        CoUninitialize();
        return 0;
    }
    pProgram->AddRef();
    pProgram->Start();

	g_VSSocketEvent[0] = CreateEvent(NULL, TRUE, TRUE, "ReadVSSocketEvent");
	g_VSSocketEvent[1] = CreateEvent(NULL, TRUE, TRUE, "WriteVSSocketEvent");
	g_UCSocketEvent[0] = CreateEvent(NULL, TRUE, TRUE, "ReadUCSocketEvent");
	g_UCSocketEvent[1] = CreateEvent(NULL, TRUE, TRUE, "WriteUCSocketEvent");

	pProgram->Release();
    p->MonitorShutdown();

	for ( size_t i = 0; i < 2; i++ )
	{
		::CloseHandle(g_UCSocketEvent[i]);
		::CloseHandle(g_VSSocketEvent[i]);
		g_UCSocketEvent[i]=INVALID_HANDLE_VALUE;
		g_VSSocketEvent[i]=INVALID_HANDLE_VALUE;
	}

    CoUninitialize();
    return 0;
}

void CWTUnrealDbgSDKModule::MonitorShutdown(void)
{
	LOG(L"MonitorShutdown");
    while (1) 
	{
        DWORD dwWait = MsgWaitForMultipleObjects(1, &m_hEventShutdown, FALSE, INFINITE, QS_ALLINPUT);
        // Shutdown event.
        if (dwWait == WAIT_OBJECT_0)
        {
			HANDLE SyncEvents[] = { m_hEventShutdown, g_VSSocketEvent[0], g_VSSocketEvent[1], g_UCSocketEvent[0], g_UCSocketEvent[1] };
            do
			{
                m_bActivity = false;
                dwWait = WaitForMultipleObjects(5, SyncEvents, TRUE, m_dwTimeOut);
            } while (dwWait == WAIT_OBJECT_0);
            // timed out
            
			if (!m_bActivity && m_nLockCnt == 0) // if no activity let's really bail
            {
 #if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
                CoSuspendClassObjects();
                if (!m_bActivity && m_nLockCnt == 0)
#endif
                break;
            }
        }
        // Windows message.
		else if (dwWait == WAIT_OBJECT_0 + 1)
		{
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				LPDEBUG_EVENT EV = (LPDEBUG_EVENT)msg.lParam;
				if ( (WM_BREAK == msg.message) && g_Program )
				{
					g_Program->NotifyBreakpointHit();
				}
				else if ( (WM_OUTPUTDEBUGSTRING == msg.message) && g_Program )
				{
					CComBSTR *bstr = (CComBSTR *)msg.wParam;
					g_Program->OutputDebugMessage(*bstr);
					delete bstr;
				}
				else if ( (WM_CLOSE == msg.message) && g_Program )
				{
					g_Program->NotifyProgramEnd();
					goto done;
				}
				else if (WM_CONTINUE == msg.message && g_Program)
				{
					g_Program->Go();
				}
				else if(WM_USER == msg.message)
				{
					DispatchMessage(&msg);
				}
				else
				{
					DispatchMessage(&msg);
				}
			}
        }
    }
done:
	LOG(L"MonitorShutdown closing");
    CloseHandle(m_hEventShutdown);
    PostThreadMessage(m_dwMainThreadID, WM_QUIT, 0, 0);
	LOG(L"MonitorShutdown done");
}

HRESULT CWTUnrealDbgSDKModule::RegisterServer(BOOL bRegTypeLib, const CLSID * pCLSID)
{
    SetMetric(metrictypeEngine, __uuidof(Engine),
        metricName, L"Unreal Script",
        false, strRegistrationRoot);
    SetMetric(metrictypeEngine, __uuidof(Engine),
        metricCLSID, CLSID_Engine,
        false, strRegistrationRoot);
    SetMetric(metrictypeEngine, __uuidof(Engine),
        metricProgramProvider, CLSID_MsProgramProvider,
//        metricProgramProvider, __uuidof(Engine),
        false, strRegistrationRoot);

    return base::RegisterServer(bRegTypeLib, pCLSID);
}

HRESULT CWTUnrealDbgSDKModule::UnregisterServer(BOOL bUnRegTypeLib, const CLSID * pCLSID)
{
    RemoveMetric(metrictypeEngine, __uuidof(Engine), metricName, strRegistrationRoot);
    RemoveMetric(metrictypeEngine, __uuidof(Engine), metricCLSID, strRegistrationRoot);
    RemoveMetric(metrictypeEngine, __uuidof(Engine), metricProgramProvider, strRegistrationRoot);

    return base::UnregisterServer(bUnRegTypeLib, pCLSID);
}




