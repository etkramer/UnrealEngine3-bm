// XeCOMClasses.cpp : Implementation of DLL Exports.


#include "stdafx.h"
#include "resource.h"
#include "XeCOMClasses.h"

// global critical section for synchronizing calls into dbghelp.dll
extern CRITICAL_SECTION SymbolCS;

class CXeCOMClassesModule : public CAtlDllModuleT< CXeCOMClassesModule >
{
public :
	DECLARE_LIBID(LIBID_XeCOMClassesLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_XECOMCLASSES, "{15128273-71F2-4F33-B8C5-AF19B512D00A}")
};

CXeCOMClassesModule _AtlModule;


#ifdef _MANAGED
#pragma managed(push, off)
#endif

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	switch(dwReason)
	{
	case DLL_PROCESS_ATTACH:
		{
			InitializeCriticalSection(&SymbolCS);
			break;
		}
	case DLL_PROCESS_DETACH:
		{
			DeleteCriticalSection(&SymbolCS);
			break;
		}
	}

    return _AtlModule.DllMain(dwReason, lpReserved); 
}

#ifdef _MANAGED
#pragma managed(pop)
#endif




// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow(void)
{
    return _AtlModule.DllCanUnloadNow();
}


// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}


// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
    // registers object, typelib and all interfaces in typelib
    HRESULT hr = _AtlModule.DllRegisterServer();
	return hr;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();
	return hr;
}

