// FXenonConsole.h : Declaration of the CFXenonConsole

#pragma once
#include "resource.h"       // main symbols

#include "XeCOMClasses.h"
#include <ImageHlp.h>
#include <vector>
#include <list>
#include <string>
#include <xbdm.h>
#include <dia2.h>
#include "..\..\Engine\Inc\UnConsoleTools.h"

// forward declarations
class CFXenonConsole;
class FThreadData;

// CFXenonConsole

typedef void (__stdcall *TTYCALLBACK)(const wchar_t *Txt);
typedef void (__stdcall *CRASHCALLBACK)(const wchar_t *Txt, const wchar_t *MiniDumpLocation);
typedef DWORD (WINAPI *THREADCALLBACK)(CComPtr<IXboxConsole> Target, FThreadData *ThreadData);

struct PDBEntry
{
	DM_PDB_SIGNATURE PDBSig;
	DWORD BaseAddress;
	DWORD Size;
};

class FThreadData
{
public:
	//NOTE: We explicitly copy the function pointers as they can be changed by the UI thread while we're parsing callstack data
	TTYCALLBACK TTYFunc;
	CFXenonConsole *Console;
	THREADCALLBACK ThreadCallback;

	virtual ~FThreadData(){}
};

class FCrashThreadData : public FThreadData
{
public:
	//NOTE: We explicitly copy the function pointers as they can be changed by the UI thread while we're parsing callstack data
	CRASHCALLBACK CrashFunc;
	std::vector<DWORD> CallStack;
	std::vector<PDBEntry> ModulePDBList;
	DWORD CrashDumpFlags;
};

class FProfileThreadData : public FThreadData
{
public:
	FConsoleSupport::EProfileType Type;
	std::wstring FileName;
};

class ATL_NO_VTABLE CFXenonConsole :
	public IDispatchImpl<XboxEvents>,
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CFXenonConsole, &CLSID_FXenonConsole>,
	public IDispatchImpl<IFXenonConsole, &IID_IFXenonConsole, &LIBID_XeCOMClassesLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
private:
	static HMODULE DbgLibraryHandle;
	static HMODULE XbdmLibraryHandle;
	static HMODULE DiaLibraryHandle;

	static void LoadSymbolHelpers();

private:
	TTYCALLBACK TxtCallback;
	CRASHCALLBACK CrashCallback;
	CComPtr<IXboxConsole> ConsolePtr;
	CComPtr<IXboxDebugTarget> DebugTgt;
	CComPtr<IConnectionPoint> ConnectionPt;
	CComQIPtr<IConnectionPointContainer> Container;
	CComBSTR TMName;
	CComBSTR CachedName;
	std::list<HANDLE> ThreadHandles;
	VARIANT_BOOL bCurrentlyDebugging;
	VARIANT_BOOL bIsDefaultConsole;
	DWORD EndpointCookie;
	DWORD LastExceptionTick;
	DWORD CrashFilter;
	DWORD DumpType;
	LONG ExecState;

	STDMETHOD(OnTTY)(const wchar_t *Txt);
	STDMETHOD(Advise)();
	STDMETHOD(UnAdvise)();
	STDMETHOD(ParseStackFrame)(IXboxThread* Thread, std::vector<DWORD> &Buffer);

	static std::wstring ResolveAddressToString(FCrashThreadData *ThreadData, std::vector<CComPtr<IDiaSession> > &LoadedModules, DWORD Address);
	static HRESULT GetLocalExecutableDirectory(std::wstring &LocalExeDir);
	static HRESULT GetRemoteExecutableDirectory(CComPtr<IXboxConsole> Target, std::wstring &OutRemoteDir);
	static HRESULT NeedsToCopyFileInternal(CComPtr<IXboxConsole> Target, BSTR SourceFilename, BSTR DestFilename, VARIANT_BOOL bReverse, VARIANT_BOOL *bOutShouldCopy);
	static HRESULT InternalRetrievePdbFile( CComPtr<IXboxConsole> Target, FThreadData *ThreadData, std::wstring& SymbolPath, std::wstring& LocalExe, std::wstring LocalPathOverride );
	static DWORD WINAPI CrashThreadProc(CComPtr<IXboxConsole> Target, FThreadData *BaseThreadData);
	static DWORD WINAPI ProfileThreadProc(CComPtr<IXboxConsole> Target, FThreadData *BaseThreadData);
	static DWORD WINAPI ThreadMain(LPVOID lpParameter);
	static char* BuildSymbolSearchPath(const wchar_t *PDBSymbolPath);
	static void ShowTargetErrorMessage(CComPtr<IXboxConsole> Target, const wchar_t *Caption, const wchar_t *Message);
	static HRESULT LoadSymbolsForModules(FCrashThreadData *ThreadData, std::vector<CComPtr<IDiaSession> > &OutLoadedModules);

public:
	CFXenonConsole();

DECLARE_REGISTRY_RESOURCEID(IDR_FXENONCONSOLE)


BEGIN_COM_MAP(CFXenonConsole)
	COM_INTERFACE_ENTRY(IFXenonConsole)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IXboxEvents)
	COM_INTERFACE_ENTRY(XboxEvents)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IXboxEvents interface
	STDMETHOD(OnStdNotify)(_XboxDebugEventType eEventCode, IXboxEventInfo *pEventInfo);
	STDMETHOD(OnTextNotify)(BSTR Source, BSTR Notification);

	// IFXenonConsole
	STDMETHOD(Initialize)(IXboxConsole *Console, BSTR TargetMgrName, VARIANT_BOOL bIsDefault);
	STDMETHOD(ConnectDebugger)(BSTR DebuggerName);
	STDMETHOD(DisconnectDebugger)();
	STDMETHOD(get_Name)(BSTR *OutName);
	STDMETHOD(get_RunningProcessName)(BSTR *OutName);
	STDMETHOD(get_IPAddressTitle)(DWORD *OutIPAddress);
	STDMETHOD(get_IsDebugging)(VARIANT_BOOL *OutValue);
	STDMETHOD(MakeDirectory)(BSTR DirectoryName);
	STDMETHOD(GetFileObject)(BSTR FilePath, IXboxFile **OutFile);
	STDMETHOD(SendFile)(BSTR LocalName, BSTR RemoteName);
	STDMETHOD(ReceiveFile)(BSTR LocalName, BSTR RemoteName);
	STDMETHOD(Reboot)(BSTR Name, BSTR MediaDirectory, BSTR CmdLine, _XboxRebootFlags Flags);
	STDMETHOD(get_Threads)(IXboxThreads **OutThreads);
	STDMETHOD(ScreenShot)(BSTR FileName);
	STDMETHOD(SendCommand)(BSTR Command);
	STDMETHOD(get_IsDefault)(VARIANT_BOOL *bOutIsDefault);
	STDMETHOD(get_ExecState)(LONG *bOutExecState);
	STDMETHOD(NeedsToCopyFile)(BSTR SourceFilename, BSTR DestFilename, VARIANT_BOOL bReverse, VARIANT_BOOL *bOutShouldCopy);
	STDMETHOD(SetTTYCallback)(unsigned hyper CallbackPtr);
	STDMETHOD(get_TargetManagerName)(BSTR *OutTMName);
	STDMETHOD(SetCrashCallback)(unsigned hyper CallbackPtr);
	STDMETHOD(get_DebugChannelIPAddress)(DWORD *OutIPAddress);
	STDMETHOD(get_TargetType)(int *OutTargetType);
	STDMETHOD(put_CrashReportFilter)(DWORD Filter);
	STDMETHOD(put_DumpType)(DWORD DumpType);
	STDMETHOD(get_DumpType)(DWORD *OutDumpType);

public:


};

OBJECT_ENTRY_AUTO(__uuidof(FXenonConsole), CFXenonConsole)
