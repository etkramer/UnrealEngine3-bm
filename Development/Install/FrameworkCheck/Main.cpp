#define _WIN32_WINNT 0x0501 //we are running on XP or later

#include <Windows.h>
#include <Shlwapi.h>

/**
 * Checks to see if we're running on a 64-bit OS.
 *
 * @return	Returns TRUE if it's a 64-bit OS.
 */
BOOL Is64BitOS()
{
	BOOL bRet = FALSE;

	if(sizeof(void*) == 8)
	{
		bRet = TRUE;
	}
	else
	{
		IsWow64Process(GetCurrentProcess(), &bRet);
	}

	return bRet;
}

/**
 * Installs the .net 2.0 framework.
 */
void ExecuteFrameworkInstallation()
{
	if(Is64BitOS())
	{
		// install 64-bit framework
		if(PathFileExists(TEXT("netfx64.exe")))
		{
			ShellExecute(NULL, TEXT("open"), TEXT("netfx64.exe"), NULL, NULL, SW_SHOWNORMAL);
		}
		else
		{
			// setup doesn't exist, open the website to download the .net framework
			ShellExecute(NULL, TEXT("open"), TEXT("http://www.microsoft.com/downloads/details.aspx?familyid=B44A0000-ACF8-4FA1-AFFB-40E78D788B00&displaylang=en"), NULL, NULL, SW_SHOWNORMAL);
		}
	}
	else
	{
		// install 32-bit framework
		if(PathFileExists(TEXT("dotnetfx.exe")))
		{
			ShellExecute(NULL, TEXT("open"), TEXT("dotnetfx.exe"), NULL, NULL, SW_SHOWNORMAL);
		}
		else
		{
			// setup doesn't exist, open the website to download the .net framework
			ShellExecute(NULL, TEXT("open"), TEXT("http://www.microsoft.com/downloads/details.aspx?FamilyID=0856EACB-4362-4B0D-8EDD-AAB15C5E04F5&displaylang=en"), NULL, NULL, SW_SHOWNORMAL);
		}
	}
}

/**
 * Normal execution when no cmd line args are passed in.
 *
 * @param	bFrameworkInstalled		True if the .net framework 2.0 is installed.
 */
void RunNormal(bool bFrameworkInstalled)
{
	if(!bFrameworkInstalled)
	{
		// run setup if the framework doesn't exist
		if(PathFileExists(TEXT("SetupUT3.exe")))
		{
			// run setup which installs the game and the .net framework
			ShellExecute(NULL, TEXT("open"), TEXT("SetupUT3.exe"), NULL, NULL, SW_SHOWNORMAL);
		}
	}
	else
	{		
		// If .net 2.0 is installed then execute the regular launcher (a .net 2.0 app).
		ShellExecute(NULL, TEXT("open"), TEXT("Launcher.exe"), NULL, NULL, SW_SHOWNORMAL);
	}
}

/**
 * WinMain()
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Check for the .net 2.0 framework
	bool bFrameworkInstalled = false;
	DWORD KeyType = 0;
	DWORD KeyValue = 0;
	DWORD Size = sizeof( KeyValue );

	HKEY Key = 0;

	if( RegOpenKeyExA( HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\v2.0.50727", 0, KEY_READ, &Key ) == ERROR_SUCCESS )
	{
		RegQueryValueExA( Key, "Install", NULL, NULL, ( LPBYTE )&KeyValue, &Size );
		if( KeyValue == 1 )
		{
			bFrameworkInstalled = true;
		}

		RegCloseKey(Key);
	}

	int Argc = 0;
	LPWSTR *Argv = CommandLineToArgvW(GetCommandLineW(), &Argc);

	if(Argc > 1 && wcscmp(Argv[1], L"-install") == 0)
	{
		if(!bFrameworkInstalled)
		{
			ExecuteFrameworkInstallation();
		}
	}
	else
	{
		RunNormal(bFrameworkInstalled);
	}

	LocalFree(Argv);
	Argv = NULL;

	return 0;
}