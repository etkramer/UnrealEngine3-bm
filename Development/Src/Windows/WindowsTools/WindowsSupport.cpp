/*=============================================================================
	WindowsTools/WindowsSupport.cpp: Windows platform support.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#define NON_UE3_APPLICATION

#include "Common.h"
#include "StringUtils.h"
#include "..\..\IpDrv\Inc\GameType.h"
#include "..\..\IpDrv\Inc\DebugServerDefs.h"
#include "ByteStreamConverter.h"
#include "WindowsTarget.h"
#include "CriticalSection.h"
#include "NetworkManager.h"

#include "..\..\Engine\Inc\UnConsoleTools.h"

//forward declaration
void DebugOutput(const char* Buffer);

/**
 *	Simplified Windows support to be used for sending and receiving messages.
 *	Used for Debug Channel for PC.
 */
class CWindowsSupport
#ifndef TESTING_WINDOWS_TOOLS
	: public FConsoleSupport
#endif
{
protected:
	FNetworkManager NetworkManager;

	/** Cache the gamename coming from the editor */
	wstring GameName;

	/** Cache the configuration (debug/release) of the editor */
	wstring Configuration;


public:
	CWindowsSupport()
	{
	}

	/** Initialize the DLL with some information about the game/editor
	 *
	 * @param	GameName		The name of the current game ("ExampleGame", "UTGame", etc)
	 * @param	Configuration	The name of the configuration to run ("Debug", "Release", etc)
	 */
	virtual void Initialize(const wchar_t* GameName, const wchar_t* Configuration)
	{
		// cache the parameters
		this->GameName = GameName;
		this->Configuration = Configuration;
	}

	/**
	 * Return a string name descriptor for this platform (required to implement)
	 *
	 * @return	The name of the platform
	 */
	virtual const wchar_t* GetConsoleName()
	{
		return CONSOLESUPPORT_NAME_PC;
	}

	/**
	 * Return the default IP address to use when sending data into the game for object propagation
	 * Note that this is probably different than the IP address used to debug (reboot, run executable, etc)
	 * the console. (required to implement)
	 *
	 * @param	Handle The handle of the console to retrieve the information from.
	 *
	 * @return	The in-game IP address of the console, in an Intel byte order 32 bit integer
	 */
	virtual unsigned int GetIPAddress(TARGETHANDLE Handle)
	{
		TargetPtr Target = NetworkManager.GetTarget(Handle);
		if (!Target)
		{
			return 0;
		}

		return Target->UDPClient.GetIP();
	}
	
	/**
	 * Return whether or not this console Intel byte order (required to implement)
	 *
	 * @return	True if the console is Intel byte order
	 */
	virtual bool GetIntelByteOrder()
	{
		return true;
	}

	/**
	 * @return the number of known targets
	 */
	virtual int GetNumTargets()
	{
		return NetworkManager.GetNumberOfTargets();
	}

	/**
	 * Get the name of the specified target
	 *
	 * @param	Handle The handle of the console to retrieve the information from.
	 * @return Name of the target, or NULL if the Index is out of bounds
	 */
	virtual const wchar_t* GetTargetName(TARGETHANDLE Handle)
	{
		TargetPtr Target = NetworkManager.GetTarget(Handle);
		
		if(!Target)
		{
			return NULL;
		}

		return Target->Name.c_str();
	}

	const wchar_t* GetTargetComputerName(TARGETHANDLE Handle)
	{
		TargetPtr Target = NetworkManager.GetTarget(Handle);
		
		if(!Target)
		{
			return NULL;
		}

		return Target->ComputerName.c_str();
	}

	const wchar_t* GetTargetGameName(TARGETHANDLE Handle)
	{
		TargetPtr Target = NetworkManager.GetTarget(Handle);
		
		if(!Target)
		{
			return NULL;
		}

		return Target->GameName.c_str();
	}

	const wchar_t* GetTargetGameType(TARGETHANDLE Handle)
	{
		TargetPtr Target = NetworkManager.GetTarget(Handle);
		
		if(!Target)
		{
			return NULL;
		}

		return Target->GameTypeName.c_str();
	}

	/**
	 * @return true if this platform needs to have files copied from PC->target (required to implement)
	 */
	virtual bool PlatformNeedsToCopyFiles()
	{
		return false;
	}

	/**
	 * Open an internal connection to a target. This is used so that each operation is "atomic" in 
	 * that connection failures can quickly be 'remembered' for one "sync", etc operation, so
	 * that a connection is attempted for each file in the sync operation...
	 * For the next operation, the connection can try to be opened again.
	 *
	 * @param TargetName Name of target
	 *
	 * @return INVALID_TARGETHANDLE for failure, or the handle used in the other functions (MakeDirectory, etc)
	 */
	virtual TARGETHANDLE ConnectToTarget(const wchar_t* TargetName)
	{
		return NetworkManager.ConnectToTarget(TargetName);
	}

	/**
	 * Open an internal connection to a target. This is used so that each operation is "atomic" in 
	 * that connection failures can quickly be 'remembered' for one "sync", etc operation, so
	 * that a connection is attempted for each file in the sync operation...
	 * For the next operation, the connection can try to be opened again.
	 *
	 * @param Handle The handle of the console to connect to.
	 *
	 * @return false for failure.
	 */
	virtual bool ConnectToTarget(TARGETHANDLE Handle)
	{
		return NetworkManager.ConnectToTarget(Handle);
	}

	/**
	 * Called after an atomic operation to close any open connections
	 *
	 * @param Handle The handle of the console to disconnect.
	 */
	virtual void DisconnectFromTarget(TARGETHANDLE Handle)
	{
		NetworkManager.DisconnectTarget(Handle);
	}

	/**
	 * Reboots the target console. Must be implemented
	 *
	 * @param Handle The handle of the console to retrieve the information from.
	 * 
	 * @return true if successful
	 */
	virtual bool Reboot(TARGETHANDLE /*Handle*/)
	{
		return false;
	}

	/**
	 * Reboots the target console. Must be implemented
	 *
	 * @param Handle			The handle of the console to retrieve the information from.
	 * @param Configuration		Build type to run (Debug, Release, RelaseLTCG, etc)
	 * @param BaseDirectory		Location of the build on the console (can be empty for platforms that don't copy to the console)
	 * @param GameName			Name of the game to run (Example, UT, etc)
	 * @param URL				Optional URL to pass to the executable
	 * @param bForceGameName	Forces the name of the executable to be only what's in GameName instead of being auto-generated
	 * 
	 * @return true if successful
	 */
	virtual bool RebootAndRun(TARGETHANDLE /*Handle*/, const wchar_t* /*Configuration*/, const wchar_t* /*BaseDirectory*/, const wchar_t* /*GameName*/, const wchar_t* /*URL*/, bool /*bForceGameName*/)
	{
		return false;
	}

	/**
	 * Run the game on the target console (required to implement)
	 *
	 * @param	TargetList				The list of handles of consoles to run the game on.
	 * @param	NumTargets				The number of handles in TargetList.
	 * @param	MapName					The name of the map that is going to be loaded.
	 * @param	URL						The map name and options to run the game with
	 * @param	OutputConsoleCommand	A buffer that the menu item can fill out with a console command to be run by the Editor on return from this function
	 * @param	CommandBufferSize		The size of the buffer pointed to by OutputConsoleCommand
	 *
	 * @return	Returns true if the run was successful
	 */
	bool RunGame(TARGETHANDLE* /*TargetList*/, int /*NumTargets*/, const wchar_t* MapName, const wchar_t* /*URL*/, wchar_t* /*OutputConsoleCommand*/, int /*CommandBufferSize*/)
	{
		wstring AppName;
		if( Configuration == L"Debug" )
		{
			AppName = Configuration + L"-" + GameName + L".exe";
		}
		else
		{
			AppName = GameName + L".exe";
		}

		wstring CmdLine = MapName;

		HINSTANCE ResultInstance =
			ShellExecuteW( NULL, L"open", AppName.c_str(), CmdLine.c_str(), NULL, SW_HIDE );

		if( ( INT_PTR )ResultInstance <= 32 )
		{
			// Failed to execute
			return false;
		}

		return true;
	}

	/**
	 * Gets a list of targets that have been selected via menu's in UnrealEd.
	 *
	 * @param	OutTargetList			The list to be filled with target handles.
	 * @param	InOutTargetListSize		Contains the size of OutTargetList. When the function returns it contains the # of target handles copied over.
	 */
	void GetMenuSelectedTargets(TARGETHANDLE* /*OutTargetList*/, int &InOutTargetListSize)
	{
		InOutTargetListSize = 0;
	}


	/**
	 * Retrieve the state of the console (running, not running, crashed, etc)
	 *
	 * @param Handle The handle of the console to retrieve the information from.
	 *
	 * @return the current state
	 */
	virtual FConsoleSupport::EConsoleState GetConsoleState(TARGETHANDLE Handle)
	{
		TargetPtr Target = NetworkManager.GetTarget(Handle);

		if(!Target)
		{
			return FConsoleSupport::CS_NotRunning;
		}

		return FConsoleSupport::CS_Running;
	}

	/**
	 * Turn an address into a symbol for callstack dumping
	 * 
	 * @param Address Code address to be looked up
	 * @param OutString Function name/symbol of the given address
	 * @param OutLength Size of the OutString buffer
	 */
	virtual void ResolveAddressToString(unsigned int Address, wchar_t* OutString, int OutLength)
	{
		swprintf_s(OutString, OutLength, L"%s", ToWString(inet_ntoa(*(in_addr*) &Address)).c_str());
	}

	/**
	 * Send a text command to the target
	 * 
	 * @param Handle The handle of the console to retrieve the information from.
	 * @param Command Command to send to the target
	 */
	virtual void SendConsoleCommand(TARGETHANDLE Handle, const wchar_t* Command)
	{
		if(!Command || wcslen(Command) == 0)
		{
			return;
		}

		NetworkManager.SendToConsole(Handle, EDebugServerMessageType_ClientText, ToString(Command));
	}

#ifndef TESTING_WINDOWS_TOOLS
	FConsoleSoundCooker* GetGlobalSoundCooker()
	{
		return NULL;
	}

	FConsoleTextureCooker* GetGlobalTextureCooker()
	{
		return NULL;
	}
#endif

	/**
	 * Retrieves the handle of the default console.
	 */
	virtual TARGETHANDLE GetDefaultTarget()
	{
		TargetPtr Target = NetworkManager.GetDefaultTarget();

		if(Target)
		{
			return Target.GetHandle();
		}

		return INVALID_TARGETHANDLE;
	}

	/**
	 * This function exists to delete an instance of FConsoleSupport that has been allocated from a *Tools.dll. Do not call this function from the destructor.
	 */
	virtual void Destroy()
	{
		// do nothing
	}

	/**
	 * Returns true if the specified target is connected for debugging and sending commands.
	 * 
	 *  @param Handle			The handle of the console to retrieve the information from.
	 */
	virtual bool GetIsTargetConnected(TARGETHANDLE Handle)
	{
		TargetPtr Target = NetworkManager.GetTarget(Handle);

		if(Target)
		{
			return Target->bConnected;
		}

		return false;
	}

	/**
	 * Retrieves a handle to each available target.
	 *
	 * @param	OutTargetList			An array to copy all of the target handles into.
	 * @param	InOutTargetListSize		This variable needs to contain the size of OutTargetList. When the function returns it will contain the number of target handles copied into OutTargetList.
	 */
	virtual void GetTargets(TARGETHANDLE *OutTargetList, int *InOutTargetListSize)
	{
		NetworkManager.GetTargets(OutTargetList, InOutTargetListSize);
	}

	/**
	 * Sets the callback function for TTY output.
	 *
	 * @param	Callback	Pointer to a callback function or NULL if TTY output is to be disabled.
	 * @param	Handle		The handle to the target that will register the callback.
	 */
	virtual void SetTTYCallback(TARGETHANDLE Handle, TTYEventCallbackPtr Callback)
	{
		TargetPtr Target = NetworkManager.GetTarget(Handle);

		if(Target)
		{
			Target->TxtCallback = Callback;
		}
	}

	/**
	* Sets the callback function for handling crashes.
	*
	* @param	Callback	Pointer to a callback function or NULL if handling crashes is to be disabled.
	* @param	Handle		The handle to the target that will register the callback.
	*/
	virtual void SetCrashCallback(TARGETHANDLE /*Handle*/, CrashCallbackPtr /*Callback*/)
	{

	}

	/**
	 * Retrieves the IP address for the debug channel at the specific target.
	 *
	 * @param	Handle	The handle to the target to retrieve the IP address for.
	 */
	virtual unsigned int GetDebugChannelIPAddress(TARGETHANDLE Handle)
	{
		return GetIPAddress(Handle);
	}

	/**
	 * Sets flags controlling how crash reports will be filtered.
	 *
	 * @param	Handle	The handle to the target to set the filter for.
	 * @param	Filter	Flags controlling how crash reports will be filtered.
	 */
	virtual bool SetCrashReportFilter(TARGETHANDLE /*Handle*/, ECrashReportFilter /*Filter*/)
	{
		return true;
	}

	/**
	 * Gets the name of a console as displayed by the target manager.
	 *
	 * @param	Handle	The handle to the target to set the filter for.
	 */
	virtual const wchar_t* GetTargetManagerNameForConsole(TARGETHANDLE Handle)
	{
		return GetTargetName(Handle);
	}

	/**
	 * Enumerates all available targets for the platform.
	 *
	 * @returns The number of available targets.
	 */
	virtual int EnumerateAvailableTargets()
	{
		// Search for available targets
		NetworkManager.Initialize();
		NetworkManager.DetermineTargets();

		return NetworkManager.GetNumberOfTargets();
	}
};

/// Output used for debugging WindowsTools only
#ifdef TESTING_WINDOWS_TOOLS
void DebugOutput(const char* Buffer)
{
	static FILE* DebugFile = NULL;

	OutputDebugString(Buffer);

	const errno_t Result = fopen_s(&DebugFile, "WindowsTools Debug Log.txt", !DebugFile ? "w" : "a");
	if (Result != 0)
	{
		return;
	}

	fprintf(DebugFile, Buffer);
	fclose(DebugFile);
}
#else
void DebugOutput(const char* Str)
{
#if DEBUG || _DEBUG
	OutputDebugString(Str);
#endif
}
#endif

CWindowsSupport WindowsSupport;
CONSOLETOOLS_API FConsoleSupport* GetConsoleSupport()
{
	return (FConsoleSupport*)(&WindowsSupport);
}
