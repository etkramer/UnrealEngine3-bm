/**
*
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

#pragma once

#include <atlbase.h>
#include <atlcom.h>

#include "XeTools.h"

#include <OleCtl.h>
#include "..\XeCOMClasses\XeCOMExports.h"

#pragma warning(push)
#pragma warning(disable : 4100) // unreferenced formal parameter										
#include "tinyxml.h"
#pragma warning(pop)

typedef IFXenonConsole* TARGET;
typedef map<wstring, TARGET> TargetMap;

class FXenonSupport : public FConsoleSupport
{
private:
	/** Structure represents a menu item. */
	struct FMenuItem
	{
		TARGETHANDLE Target;
		bool bChecked;

		FMenuItem(){}
		FMenuItem(TARGETHANDLE Handle, bool bIsChecked) : Target(Handle), bChecked(bIsChecked) {}
	};

	/** Cache the gamename coming from the editor */
	wstring GameName;

	/** Used to persist xenon names past a function call. */
	wstring XenonNameCache;

	/** Cache the configuration (debug/release) of the editor */
	wstring Configuration;

	/** Holds cached xbox connections */
	set<TARGET> CachedConnections;

	/** Holds targets using their target manager name. */
	TargetMap ConnectionNames;

	/** DVD layout file interface */
	TiXmlDocument DVDLayout;

	/** true if FXenonSupport is setup and working properly */
	bool bIsProperlySetup;

	HMODULE XbdmLibrary;

	/** The local pdb file */
	wstring LocalPDBFile;

	/** List of available menu items. */
	vector<FMenuItem> MenuItems;

private:
	bool Setup();
	void Cleanup();
	/**
	 * Retrieves the target with the specified handle if it exists.
	 *
	 * @param	Handle	The handle of the target to retrieve.
	 */
	TARGET GetTarget(TARGETHANDLE Handle);

	/**
	 * Retrieves the target with the specified name if it exists.
	 *
	 * @param	Handle	The handle of the target to retrieve.
	 */
	TARGET GetTarget(const wchar_t *Name);

	/**
	 * This handler creates a multi-threaded apartment from which to create all of the target COM objects so we can sync in multiple worker threads.
	 *
	 * @param	lpParameter		Pointer to extra thread data.
	 */
	static DWORD WINAPI SyncThreadWorkerMain(LPVOID lpParameter);
	static DWORD WINAPI SyncThreadMain(LPVOID lpParameter);
	static HRESULT MakeDirectoryHierarchy(CComPtr<IXboxConsole> &Target, wstring DirectoryHierarchy);
	static HRESULT NeedsToCopyFileInternal(CComPtr<IXboxConsole> Target, BSTR SourceFilename, BSTR DestFilename, VARIANT_BOOL bReverse, VARIANT_BOOL *bOutShouldCopy);

public:

	FXenonSupport();
	virtual ~FXenonSupport();

	inline bool IsProperlySetup()
	{
		return bIsProperlySetup;
	}

	/** Initialize the DLL with some information about the game/editor
	 *
	 * @param	GameName		The name of the current game ("ExampleGame", "UTGame", etc)
	 * @param	Configuration	The name of the configuration to run ("Debug", "Release", etc)
	 */
	virtual void Initialize(const wchar_t* InGameName, const wchar_t* InConfiguration);

	/**
	 * Return a string name descriptor for this platform (required to implement)
	 *
	 * @return	The name of the platform
	 */
	virtual const wchar_t* GetConsoleName();

	/**
	 * Return the default IP address to use when sending data into the game for object propagation
	 * Note that this is probably different than the IP address used to debug (reboot, run executable, etc)
	 * the console. (required to implement)
	 *
	 * @param	Handle The handle of the console to retrieve the information from.
	 *
	 * @return	The in-game IP address of the console, in an Intel byte order 32 bit integer
	 */
	unsigned int GetIPAddress(TARGETHANDLE Handle);

	/**
	 * Return whether or not this console Intel byte order (required to implement)
	 *
	 * @return	True if the console is Intel byte order
	 */
	bool GetIntelByteOrder();

	/**
	 * @return the number of known xbox targets
	 */
	int GetNumTargets();

	/**
	 * Retrieves a handle to each available target.
	 *
	 * @param	OutTargetList			An array to copy all of the target handles into.
	 * @param	InOutTargetListSize		This variable needs to contain the size of OutTargetList. When the function returns it will contain the number of target handles copied into OutTargetList.
	 */
	virtual void GetTargets(TARGETHANDLE *OutTargetList, int *InOutTargetListSize);

	/**
	 * Get the name of the specified target
	 *
	 * @param	Handle The handle of the console to retrieve the information from.
	 * @return Name of the target, or NULL if the Index is out of bounds
	 */
	const wchar_t* GetTargetName(TARGETHANDLE Handle);

	/**
	 * @return true if this platform needs to have files copied from PC->target (required to implement)
	 */
	virtual bool PlatformNeedsToCopyFiles();

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
	virtual TARGETHANDLE ConnectToTarget(const wchar_t* TargetName);

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
	virtual bool ConnectToTarget(TARGETHANDLE Handle);

	/**
	 * Called after an atomic operation to close any open connections
	 *
	 * @param Handle The handle of the console to disconnect.
	 */
	virtual void DisconnectFromTarget(TARGETHANDLE Handle);

	/**
	 * Creates a directory
	 *
	 * @param Handle The handle of the console to retrieve the information from.
	 * @param SourceFilename Platform-independent directory name (ie UnrealEngine3\Binaries)
	 *
	 * @return true if successful
	 */
	virtual bool MakeDirectory(TARGETHANDLE Handle, const wchar_t* DirectoryName);

	/**
	 * Determines if the given file needs to be copied
	 *
	 * @param Handle The handle of the console to retrieve the information from.
	 * @param SourceFilename	Path of the source file on PC
	 * @param DestFilename		Platform-independent destination filename (ie, no xe:\\ for Xbox, etc)
	 * @param bReverse			If true, then copying from platform (dest) to PC (src);
	 *
	 * @return true if successful
	 */
	virtual bool NeedsToCopyFile(TARGETHANDLE Handle, const wchar_t* SourceFilename, const wchar_t* DestFilename, bool bReverse = false);

	/**
	 * Copies a single file from PC to target
	 *
	 * @param Handle The handle of the console to retrieve the information from.
	 * @param SourceFilename Path of the source file on PC
	 * @param DestFilename Platform-independent destination filename (ie, no xe:\\ for Xbox, etc)
	 *
	 * @return true if successful
	 */
	virtual bool CopyFile(TARGETHANDLE Handle, const wchar_t* SourceFilename, const wchar_t* DestFilename);

	/**
	 *	Copies a single file from the target to the PC
	 *
	 *  @param	Handle			The handle of the console to retrieve the information from.
	 *	@param	SourceFilename	Platform-independent source filename (ie, no xe:\\ for Xbox, etc)
	 *	@param	DestFilename	Path of the destination file on PC
	 *	
	 *	@return	bool			true if successful, false otherwise
	 */
	virtual bool RetrieveFile(TARGETHANDLE Handle, const wchar_t* SourceFilename, const wchar_t* DestFilename);

	/**
	 * Sets the name of the layout file for the DVD, so that GetDVDFileStartSector can be used
	 * 
	 * @param DVDLayoutFile Name of the layout file
	 *
	 * @return true if successful
	 */
	virtual bool SetDVDLayoutFile(const wchar_t* DVDLayoutFile);

	/**
	 * Processes a DVD layout file "object" to see if it's the matching file.
	 * 
	 * @param Object XmlElement in question
	 * @param Filename Filename (no path) we are trying to match
	 * @param LBA Output LBA information
	 * 
	 * @return true if successful
	 */
	bool HandleDVDObject(TiXmlElement* Object, const char* Filename, __int64& LBA);

	/**
	 * Gets the starting sector of a file on the DVD (or whatever optical medium)
	 *
	 * @param DVDFilename Path to the file on the DVD
	 * @param SectorHigh High 32 bits of the sector location
	 * @param SectorLow Low 32 bits of the sector location
	 * 
	 * @return true if the start sector was found for the file
	 */
	virtual bool GetDVDFileStartSector(const wchar_t* DVDFilename, unsigned int& SectorHigh, unsigned int& SectorLow);


	/**
	 * Reboots the target console. Must be implemented
	 *
	 * @param Handle The handle of the console to retrieve the information from.
	 * 
	 * @return true if successful
	 */
	virtual bool Reboot(TARGETHANDLE Handle);

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
	virtual bool RebootAndRun(TARGETHANDLE Handle, const wchar_t* Configuration, const wchar_t* BaseDirectory, const wchar_t* GameName, const wchar_t* URL, bool bForceGameName);

	/**
	 * This function is run on a separate thread for cooking, copying, and running an autosaved level on an xbox 360.
	 *
	 * @param	Data	A pointer to data passed into the thread providing it with extra data needed to do its job.
	 */
	static void __cdecl RunGameThread(void *Data);

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
	virtual bool RunGame(TARGETHANDLE *TargetList, int NumTargets, const wchar_t* MapName, const wchar_t* URL, wchar_t* OutputConsoleCommand, int CommandBufferSize);

	/**
	 * Gets a list of targets that have been selected via menu's in UnrealEd.
	 *
	 * @param	OutTargetList			The list to be filled with target handles.
	 * @param	InOutTargetListSize		Contains the size of OutTargetList. When the function returns it contains the # of target handles copied over.
	 */
	virtual void GetMenuSelectedTargets(TARGETHANDLE *OutTargetList, int &InOutTargetListSize);

	/**
	 * Send a text command to the target
	 * 
	 * @param Handle The handle of the console to retrieve the information from.
	 * @param Command Command to send to the target
	 */
	virtual void SendConsoleCommand(TARGETHANDLE Handle, const wchar_t* Command);

	/**
	 * Retrieve the state of the console (running, not running, crashed, etc)
	 *
	 * @param Handle The handle of the console to retrieve the information from.
	 *
	 * @return the current state
	 */
	virtual EConsoleState GetConsoleState(TARGETHANDLE Handle);

	/**
	 * Have the console take a screenshot and dump to a file
	 * 
	 * @param Handle The handle of the console to retrieve the information from.
	 * @param Filename Location to place the .bmp file
	 *
	 * @return true if successful
	 */
	virtual bool ScreenshotBMP(TARGETHANDLE Handle, const wchar_t* Filename);

	/**
	 * Return the number of console-specific menu items this platform wants to add to the main
	 * menu in UnrealEd.
	 *
	 * @return	The number of menu items for this console
	 */
	virtual int GetNumMenuItems();

	/**
	 * Return the string label for the requested menu item
	 * @param	Index		The requested menu item
	 * @param	bIsChecked	Is this menu item checked (or selected if radio menu item)
	 * @param	bIsRadio	Is this menu item part of a radio group?
	 * @param	OutHandle	Receives the handle of the target associated with the menu item.
	 *
	 * @return	Label for the requested menu item
	 */
	virtual const wchar_t* GetMenuItem(int Index, bool& bIsChecked, bool& bIsRadio, TARGETHANDLE& OutHandle);

	/**
	 * Internally process the given menu item when it is selected in the editor
	 * @param	Index		The selected menu item
	 * @param	OutputConsoleCommand	A buffer that the menu item can fill out with a console command to be run by the Editor on return from this function
	 * @param	CommandBufferSize		The size of the buffer pointed to by OutputConsoleCommand
	 */
	virtual void ProcessMenuItem(int Index, wchar_t* OutputConsoleCommand, int CommandBufferSize);

	/**
	 * Returns the global sound cooker object.
	 *
	 * @return global sound cooker object, or NULL if none exists
	 */
	virtual FConsoleSoundCooker* GetGlobalSoundCooker();

	/**
	 * Returns the global texture cooker object.
	 *
	 * @return global sound cooker object, or NULL if none exists
	 */
	virtual FConsoleTextureCooker* GetGlobalTextureCooker();

	/**
	 * Returns the global skeletal mesh cooker object.
	 *
	 * @return global skeletal mesh cooker object, or NULL if none exists
	 */
	virtual FConsoleSkeletalMeshCooker* GetGlobalSkeletalMeshCooker();

	/**
	 * Returns the global static mesh cooker object.
	 *
	 * @return global static mesh cooker object, or NULL if none exists
	 */
	virtual FConsoleStaticMeshCooker* GetGlobalStaticMeshCooker();

	/**
	 * Returns the global shader precompiler object.
	 * @return global shader precompiler object, or NULL if none exists.
	 */
	virtual FConsoleShaderPrecompiler* GetGlobalShaderPrecompiler();

	/**
	 * Converts an Unreal Engine texture format to a Xenon texture format.
	 *
	 * @param	UnrealFormat	The unreal format.
	 * @param	Flags			Extra flags describing the format.
	 * @return	The associated Xenon format.
	 */
	D3DFORMAT ConvertToXenonFormat(DWORD UnrealFormat, DWORD Flags);

	/**
	 *  Gets the platform-specific size required for the given texture.
	 *
	 *	@param	UnrealFormat	Unreal pixel format
	 *	@param	Width			Width of texture (in pixels)
	 *	@param	Height			Height of texture (in pixels)
	 *	@param	NumMips			Number of miplevels
	 *	@param	CreateFlags		Platform-specific creation flags
	 *
	 *	@return	INT				The size of the memory allocation needed for the texture.
	 */
	virtual INT GetPlatformTextureSize(DWORD UnrealFormat, UINT Width, UINT Height, UINT NumMips, DWORD CreateFlags);

	/**
	 *  Gets the platform-specific size required for the given cubemap texture.
	 *
	 *	@param	UnrealFormat	Unreal pixel format
	 *	@param	Size			Size of the cube edge (in pixels)
	 *	@param	NumMips			Number of miplevels
	 *	@param	CreateFlags		Platform-specific creation flags
	 *
	 *	@return	INT				The size of the memory allocation needed for the texture.
	 */
	virtual INT GetPlatformCubeTextureSize(DWORD UnrealFormat, UINT Size, UINT NumMips, DWORD CreateFlags);

	/**
	 * Retrieves the handle of the default console.
	 */
	virtual TARGETHANDLE GetDefaultTarget();

	/**
	 * This function exists to delete an instance of FConsoleSupport that has been allocated from a *Tools.dll. Do not call this function from the destructor.
	 */
	virtual void Destroy();

	/**
	 * Returns true if the specified target is connected for debugging and sending commands.
	 * 
	 *  @param Handle			The handle of the console to retrieve the information from.
	 */
	virtual bool GetIsTargetConnected(TARGETHANDLE Handle);

	/**
	 * Sets the callback function for TTY output.
	 *
	 * @param	Callback	Pointer to a callback function or NULL if TTY output is to be disabled.
	 * @param	Handle		The handle to the target that will register the callback.
	 */
	virtual void SetTTYCallback(TARGETHANDLE Handle, TTYEventCallbackPtr Callback);

	/**
	 * Sets the callback function for handling crashes.
	 *
	 * @param	Callback	Pointer to a callback function or NULL if handling crashes is to be disabled.
	 * @param	Handle		The handle to the target that will register the callback.
	 */
	virtual void SetCrashCallback(TARGETHANDLE Handle, CrashCallbackPtr Callback);

	/**
	 * Retrieves the IP address for the debug channel at the specific target.
	 *
	 * @param	Handle	The handle to the target to retrieve the IP address for.
	 */
	virtual unsigned int GetDebugChannelIPAddress(TARGETHANDLE Handle);

	/**
	 * Returns the type of the specified target.
	 */
	virtual FConsoleSupport::ETargetType GetTargetType(TARGETHANDLE Handle);

	/**
	 * Sets flags controlling how crash reports will be filtered.
	 *
	 * @param	Handle	The handle to the target to set the filter for.
	 * @param	Filter	Flags controlling how crash reports will be filtered.
	 */
	virtual bool SetCrashReportFilter(TARGETHANDLE Handle, ECrashReportFilter Filter);

	/**
	 * Gets the name of a console as displayed by the target manager.
	 *
	 * @param	Handle	The handle to the target to set the filter for.
	 */
	virtual const wchar_t* GetTargetManagerNameForConsole(TARGETHANDLE Handle);

	/**
	 * Enumerates all available targets for the platform.
	 *
	 * @returns The number of available targets.
	 */
	virtual int EnumerateAvailableTargets();

	/**
	 * Asynchronously copies a set of files to a set of targets.
	 *
	 * @param	Handles						An array of targets to receive the files.
	 * @param	HandlesSize					The size of the array pointed to by Handles.
	 * @param	SrcFilesToSync				An array of source files to copy. This must be the same size as DestFilesToSync.
	 * @param	DestFilesToSync				An array of destination files to copy to. This must be the same size as SrcFilesToSync.
	 * @param	FilesToSyncSize				The size of the SrcFilesToSync and DestFilesToSync arrays.
	 * @param	DirectoriesToCreate			An array of directories to be created on the targets.
	 * @param	DirectoriesToCreateSize		The size of the DirectoriesToCreate array.
	 * @param	OutputCallback				TTY callback for receiving colored output.
	 */
	virtual bool SyncFiles(TARGETHANDLE *Handles, int HandlesSize, const wchar_t **SrcFilesToSync, const wchar_t **DestFilesToSync, int FilesToSyncSize, const wchar_t **DirectoriesToCreate, int DirectoriesToCreateSize, ColoredTTYEventCallbackPtr OutputCallback);

	/**
	 * Gets the dump type the current target is set to.
	 *
	 * @param	Handle	The handle to the target to get the dump type from.
	 */
	virtual EDumpType GetDumpType(TARGETHANDLE Handle);

	/**
	 * Sets the dump type of the current target.
	 *
	 * @param	DmpType		The new dump type of the target.
	 * @param	Handle		The handle to the target to set the dump type for.
	 */
	virtual void SetDumpType(TARGETHANDLE Handle, EDumpType DmpType);
};