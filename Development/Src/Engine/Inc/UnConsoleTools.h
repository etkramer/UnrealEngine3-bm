/*=============================================================================
	UnConsoleTools.h: Definition of platform agnostic helper functions
					 implemented in a separate DLL.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// define the strings that are returned by the console .dlls so that the engine can refer to them without risk if 
// the platform-specific support class is ever changed to return a slightly different string
#ifndef CONSOLESUPPORT_NAME_PC
#define CONSOLESUPPORT_NAME_PC	L"PC"
#endif // CONSOLESUPPORT_NAME_PC

#ifndef CONSOLESUPPORT_NAME_360
#define CONSOLESUPPORT_NAME_360	L"Xenon"
#endif // CONSOLESUPPORT_NAME_360

#ifndef CONSOLESUPPORT_NAME_PS3
#define CONSOLESUPPORT_NAME_PS3	L"PS3"
#endif // CONSOLESUPPORT_NAME_PS3

// this is only defined when being included by File.h in the UnrealEngine solution.
#ifndef SUPPORT_NAMES_ONLY

#ifndef __UNCONSOLETOOLS_H__
#define __UNCONSOLETOOLS_H__

#ifdef CONSOLETOOLS_EXPORTS
#define CONSOLETOOLS_API __declspec(dllexport)
#else
#define CONSOLETOOLS_API 
#endif

// allow for Managed C++ to include this file and do whatever it needs to use
// the structs
#ifndef MANAGED_CPP_MAGIC 
#define MANAGED_CPP_MAGIC 
#endif

/**
 * Abstract sound cooker interface.
 *
 * Expected usage is:
 *
 * Cook(...)
 * BYTE* CookedData = appMalloc( GetCookedDataSize() );
 * GetCookedData( BYTE* CookedData );
 *
 * and repeat.
 */
class FConsoleSoundCooker
{
public:
	/**
	 * Constructor
	 */
	FConsoleSoundCooker( void ) {}

	/**
	 * Virtual destructor
	 */
	virtual ~FConsoleSoundCooker( void ) {}

	/**
	 * Cooks the source data for the platform and stores the cooked data internally.
	 *
	 * @param	SrcBuffer		Pointer to source buffer
	 * @param	SrcBufferSize	Size in bytes of source buffer
	 * @param	WaveFormat		Pointer to platform specific wave format description
	 * @param	Compression		Quality value ranging from 1 [poor] to 100 [very good]
	 *
	 * @return	TRUE if succeeded, FALSE otherwise
	 */
	virtual bool Cook( short * SrcBuffer, DWORD SrcBufferSize, void * WaveFormat, INT Quality, const TCHAR* Name ) = 0;
	
	/**
	 * Cooks upto 8 mono files into a multichannel file (eg. 5.1). The front left channel is required, the rest are optional.
	 *
	 * @param	SrcBuffers		Pointers to source buffers
	 * @param	SrcBufferSize	Size in bytes of source buffer
	 * @param	WaveFormat		Pointer to platform specific wave format description
	 * @param	Compression		Quality value ranging from 1 [poor] to 100 [very good]
	 *
	 * @return	TRUE if succeeded, FALSE otherwise
	 */
	virtual bool CookSurround( short * SrcBuffers[8], DWORD SrcBufferSize, void * WaveFormat, INT Quality, const TCHAR* Name ) = 0;

	/**
	 * Returns the size of the cooked data in bytes.
	 *
	 * @return The size in bytes of the cooked data including any potential header information.
	 */
	virtual UINT GetCookedDataSize( void ) = 0;

	/**
	 * Copies the cooked ata into the passed in buffer of at least size GetCookedDataSize()
	 *
	 * @param CookedData	Buffer of at least GetCookedDataSize() bytes to copy data to.
	 */
	virtual void GetCookedData( BYTE * CookedData ) = 0;
	
	/** 
	 * Decompresses the platform dependent format to raw PCM. Used for quality previewing.
	 *
	 * @param	CompressedData		Compressed data
	 * @param	CompressedDataSize	Size of compressed data
	 * @param	AdditionalInfo		Any additonal info required to decompress the sound
	 */
	virtual bool Decompress( const BYTE * CompressedData, DWORD CompressedDataSize, void * AdditionalInfo ) = 0;

	/**
	 * Returns the size of the decompressed data in bytes.
	 *
	 * @return The size in bytes of the raw PCM data
	 */
	virtual DWORD GetRawDataSize( void ) = 0;

	/**
	 * Copies the raw data into the passed in buffer of at least size GetRawDataSize()
	 *
	 * @param DstBuffer	Buffer of at least GetRawDataSize() bytes to copy data to.
	 */
	virtual void GetRawData( BYTE * DstBuffer ) = 0;

	/**
	 * Queries for any warning or error messages resulting from the cooking phase
	 *
	 * @return Warning or error message string, or NULL if nothing to report
	 */
	virtual const char* GetCookErrorMessages() const { return NULL; }
};

/**
 * Abstract 2D texture cooker interface.
 * 
 * Expected usage is:
 * Init(...)
 * for( MipLevels )
 * {
 *     Dst = appMalloc( GetMipSize( MipLevel ) );
 *     CookMip( ... )
 * }
 *
 * and repeat.
 */
struct CONSOLETOOLS_API FConsoleTextureCooker
{
	/**
	 * Virtual destructor
	 */
	virtual ~FConsoleTextureCooker(){}

	/**
	 * Associates texture parameters with cooker.
	 *
	 * @param UnrealFormat	Unreal pixel format
	 * @param Width			Width of texture (in pixels)
	 * @param Height		Height of texture (in pixels)
	 * @param NumMips		Number of miplevels
	 * @param CreateFlags	Platform-specific creation flags
	 */
	virtual void Init( DWORD UnrealFormat, UINT Width, UINT Height, UINT NumMips, DWORD CreateFlags ) = 0;

	/**
	 * Returns the platform specific size of a miplevel.
	 *
	 * @param	Level		Miplevel to query size for
	 * @return	Returns	the size in bytes of Miplevel 'Level'
	 */
	virtual UINT GetMipSize( UINT Level ) = 0;

	/**
	 * Cooks the specified miplevel, and puts the result in Dst which is assumed to
	 * be of at least GetMipSize size.
	 *
	 * @param Level			Miplevel to cook
	 * @param Src			Src pointer
	 * @param Dst			Dst pointer, needs to be of at least GetMipSize size
	 * @param SrcRowPitch	Row pitch of source data
	 */
	virtual void CookMip( UINT Level, void* Src, void* Dst, UINT SrcRowPitch ) = 0;

	/**
	* Returns the index of the first mip level that resides in the packed miptail
	* 
	* @return index of first level of miptail 
	*/
	virtual INT GetMipTailBase() = 0;

	/**
	* Cooks all mip levels that reside in the packed miptail. Dst is assumed to 
	* be of size GetMipSize (size of the miptail level).
	*
	* @param Src - ptrs to mip data for each source mip level
	* @param SrcRowPitch - array of row pitch entries for each source mip level
	* @param Dst - ptr to miptail destination
	*/
	virtual void CookMipTail( void** Src, UINT* SrcRowPitch, void* Dst ) = 0;

};

/** The information about a mesh element that is needed to cook it. */
struct FMeshElementCookInfo
{
	const WORD* Indices;
	UINT NumTriangles;
};

/**
 * Abstract skeletal mesh cooker interface.
 * 
 * Expected usage is:
 * Init(...)
 * CookMeshElement( ... )
 *
 * and repeat.
 */
struct CONSOLETOOLS_API FConsoleSkeletalMeshCooker
{
	/**
	 * Virtual destructor
	 */
	virtual ~FConsoleSkeletalMeshCooker(){}

	virtual void Init( void ) = 0;

	/**
	 * Cooks a mesh element.
	 * @param ElementInfo - Information about the element being cooked
	 * @param OutIndices - Upon return, contains the optimized index buffer.
	 */
	virtual void CookMeshElement(
		const FMeshElementCookInfo& ElementInfo,
		WORD* OutIndices
		) = 0;
};

/**
 * Abstract static mesh cooker interface.
 * 
 * Expected usage is:
 * Init(...)
 * CookMeshElement( ... )
 *
 * and repeat.
 */
struct CONSOLETOOLS_API FConsoleStaticMeshCooker
{
	/**
	 * Virtual destructor
	 */
	virtual ~FConsoleStaticMeshCooker(){}

	virtual void Init( void ) = 0;

	/**
	 * Cooks a mesh element.
	 * @param ElementInfo - Information about the element being cooked
	 * @param OutIndices - Upon return, contains the optimized index buffer.
	 */
	virtual void CookMeshElement(
		const FMeshElementCookInfo& ElementInfo,
		WORD* OutIndices
		) = 0;
};

/**
 * Base class to handle precompiling shaders offline in a per-platform manner
 */
MANAGED_CPP_MAGIC class FConsoleShaderPrecompiler
{
public:
	/**
	 * Virtual destructor
	 */
	virtual ~FConsoleShaderPrecompiler(){}

	/**
	 * Precompile the shader with the given name. Must be implemented
	 *
	 * @param ShaderPath			Pathname to the source shader file ("..\Engine\Shaders\BasePassPixelShader.usf")
	 * @param EntryFunction			Name of the startup function ("pixelShader")
	 * @param bIsVertexShader		True if the vertex shader is being precompiled
	 * @param CompileFlags			Default is 0, otherwise members of the D3DXSHADER enumeration
	 * @param Definitions			Space separated string that contains shader defines ("FRAGMENT_SHADER=1 VERTEX_LIGHTING=0")
	 * @param bDumpingShaderPDBs	True if shader PDB's should be saved to ShaderPDBPath
	 * @param ShaderPDBPath			Path to save shader PDB's, can be on the local machine if not using runtime compiling.
	 * @param BytecodeBuffer		Block of memory to fill in with the compiled bytecode
	 * @param BytecodeSize			Size of the returned bytecode
	 * @param ConstantBuffer		String buffer to return the shader definitions with [Name,RegisterIndex,RegisterCount] ("WorldToLocal,100,4 Scale,101,1"), NULL = Error case
	 * @param Errors				String buffer any output/errors
	 * 
	 * @return true if successful
	 */
	virtual bool PrecompileShader(
		const char* ShaderPath, 
		const char* EntryFunction, 
		bool bIsVertexShader, 
		DWORD CompileFlags, 
		const char* Definitions, 
		bool bDumpingShaderPDBs,
		const char* ShaderPDBPath,
		unsigned char* BytecodeBufer, 
		int& BytecodeSize, 
		char* ConstantBuffer, 
		char* Errors
		) = 0;

	/**
	 * Preprocess the shader with the given name. Must be implemented
	 *
	 * @param ShaderPath		Pathname to the source shader file ("..\Engine\Shaders\BasePassPixelShader.usf")
	 * @param Definitions		Space separated string that contains shader defines ("FRAGMENT_SHADER=1 VERTEX_LIGHTING=0")
	 * @param ShaderText		Block of memory to fill in with the preprocessed shader output
	 * @param ShaderTextSize	Size of the returned text
	 * @param Errors			String buffer any output/errors
	 * 
	 * @return true if successful
	 */
	virtual bool PreprocessShader(const char* ShaderPath, const char* Definitions, unsigned char* ShaderText, int& ShaderTextSize, char* Errors) = 0;

	/**
	 * Disassemble the shader with the given byte code. Must be implemented
	 *
	 * @param ShaderByteCode	The null terminated shader byte code to be disassembled
	 * @param ShaderText		Block of memory to fill in with the preprocessed shader output
	 * @param ShaderTextSize	Size of the returned text
	 * 
	 * @return true if successful
	 */
	virtual bool DisassembleShader(const DWORD *ShaderByteCode, unsigned char* ShaderText, int& ShaderTextSize) = 0;

	/**
	 * Create a command line to compile the shader with the given parameters. Must be implemented
	 *
	 * @param ShaderPath		Pathname to the source shader file ("..\Engine\Shaders\BasePassPixelShader.usf")
	 * @param IncludePath		Pathname to extra include directory (can be NULL)
	 * @param EntryFunction		Name of the startup function ("Main") (can be NULL)
	 * @param bIsVertexShader	True if the vertex shader is being precompiled
	 * @param CompileFlags		Default is 0, otherwise members of the D3DXSHADER enumeration
	 * @param Definitions		Space separated string that contains shader defines ("FRAGMENT_SHADER=1 VERTEX_LIGHTING=0") (can be NULL)
	 * @param CommandStr		Block of memory to fill in with the null terminated command line string
	 * 
	 * @return true if successful
	 */
	virtual bool CreateShaderCompileCommandLine(const char* ShaderPath, const char* IncludePath, const char* EntryFunction, bool bIsVertexShader, DWORD CompileFlags, const char* Definitions, char* CommandStr) = 0;
};

#define MENU_SEPARATOR_LABEL L"--"

// NOTE: Also defined in Core.h for object propagation
#ifndef INVALID_TARGETHANDLE
typedef void* TARGETHANDLE;
#define INVALID_TARGETHANDLE ((TARGETHANDLE)-1)
#endif

// the ps3 gcc compiler does not support calling convention declarations, windows is the only one that needs this anyway
#if WIN32
typedef void (__stdcall *CrashCallbackPtr)(const wchar_t *CallStack);
typedef void (__stdcall *TTYEventCallbackPtr)(const wchar_t *Txt);
typedef void (__stdcall *ColoredTTYEventCallbackPtr)(DWORD Color, const wchar_t *Txt);
typedef void (__stdcall *ProfilerEventCallbackPtr)(const wchar_t *Txt);
#endif

/**
 * Base abstract class for a platform-specific support class. Every platform
 * will override the member functions to provide platform-speciific code.
 * This is so that access to platform-specific code is not given to all
 * licensees of the Unreal Engine. This is to protect the NDAs of the
 * console manufacturers from Unreal Licensees who don't have a license with
 * that manufacturer.
 *
 * The ConsoleSupport subclasses will be defined in DLLs, so Epic can control 
 * the code/proprietary information simply by controlling who gets what DLLs.
 */
MANAGED_CPP_MAGIC class FConsoleSupport
{
public:
	enum EConsoleState
	{
		CS_NotRunning = 0,
		CS_Rebooting,
		CS_Running,
		CS_Crashed,
		CS_Asserted,
		CS_RIP,
	};

	enum EProfileType
	{
		PT_Invalid,
		PT_Script,
		PT_GameTick,
		PT_RenderTick,
		PT_Memory,
		PT_UE3Stats,
		PT_MemLeaks,
		PT_FPSCharts,
		PT_BugIt,
		PT_MiscFile,
	};

	enum ETargetType
	{
		TART_Unknown = 0,
		TART_TestKit,
		TART_DevKit,
		TART_ReviewerKit
	};

	enum ECrashReportFilter
	{
		CRF_None = 0,
		CRF_Debug = 1,
		CRF_Release = 1 << 1,
		CRF_ReleaseForShip = 1 << 2,
		CRF_All = CRF_Debug | CRF_Release | CRF_ReleaseForShip
	};

	enum EDumpType
	{
		DMPT_Normal = 0,
		DMPT_WithFullMemory,
	};

	/**
	 * Virtual destructor
	 */
	virtual ~FConsoleSupport(){}

	/** Initialize the DLL with some information about the game/editor
	 *
	 * @param	GameName		The name of the current game ("ExampleGame", "UTGame", etc)
	 * @param	Configuration	The name of the configuration to run ("Debug", "Release", etc)
	 */
	virtual void Initialize(const wchar_t* GameName, const wchar_t* Configuration) = 0;

	/**
	 * Return a string name descriptor for this platform (required to implement)
	 *
	 * @return	The name of the platform
	 */
	virtual const wchar_t* GetConsoleName() = 0;

	/**
	 * Return the default IP address to use when sending data into the game for object propagation
	 * Note that this is probably different than the IP address used to debug (reboot, run executable, etc)
	 * the console. (required to implement)
	 *
	 * @param	Handle The handle of the console to retrieve the information from.
	 *
	 * @return	The in-game IP address of the console, in an Intel byte order 32 bit integer
	 */
	virtual unsigned int GetIPAddress(TARGETHANDLE Handle) = 0;
	
	/**
	 * Return whether or not this console Intel byte order (required to implement)
	 *
	 * @return	True if the console is Intel byte order
	 */
	virtual bool GetIntelByteOrder() = 0;
	
	/**
	 * @return the number of known targets
	 */
	virtual int GetNumTargets()
	{
		return 0;
	}

	/**
	 * Get the name of the specified target
	 *
	 * @param	Handle The handle of the console to retrieve the information from.
	 * @return Name of the target, or NULL if the Index is out of bounds
	 */
	virtual const wchar_t* GetTargetName(TARGETHANDLE /*Handle*/)
	{
		return NULL;
	}

	/**
	 * @return true if this platform needs to have files copied from PC->target (required to implement)
	 */
	virtual bool PlatformNeedsToCopyFiles() = 0;

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
	virtual TARGETHANDLE ConnectToTarget(const wchar_t* /*TargetName*/)
	{
		return 0;
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
	virtual bool ConnectToTarget(TARGETHANDLE /*Handle*/)
	{
		return false;
	}

	/**
	 * Called after an atomic operation to close any open connections
	 *
	 * @param Handle The handle of the console to disconnect.
	 */
	virtual void DisconnectFromTarget(TARGETHANDLE /*Handle*/)
	{

	}

	/**
	 * Creates a directory
	 *
	 * @param Handle The handle of the console to retrieve the information from.
	 * @param SourceFilename Platform-independent directory name (ie UnrealEngine3\Binaries)
	 *
	 * @return true if successful
	 */
	virtual bool MakeDirectory(TARGETHANDLE /*Handle*/, const wchar_t* /*DirectoryName*/)
	{
		return false;
	}

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
	virtual bool NeedsToCopyFile(TARGETHANDLE /*Handle*/, const wchar_t* /*SourceFilename*/, const wchar_t* /*DestFilename*/, bool /*bReverse*/)
	{
		return false;
	}

	/**
	 * Copies a single file from PC to target
	 *
	 * @param Handle		The handle of the console to retrieve the information from.
	 * @param SourceFilename Path of the source file on PC
	 * @param DestFilename Platform-independent destination filename (ie, no xe:\\ for Xbox, etc)
	 *
	 * @return true if successful
	 */
	virtual bool CopyFile(TARGETHANDLE /*Handle*/, const wchar_t* /*SourceFilename*/, const wchar_t* /*DestFilename*/)
	{
		return false;
	}

	/**
	 *	Copies a single file from the target to the PC
	 *
	 *  @param	Handle			The handle of the console to retrieve the information from.
	 *	@param	SourceFilename	Platform-independent source filename (ie, no xe:\\ for Xbox, etc)
	 *	@param	DestFilename	Path of the destination file on PC
	 *	
	 *	@return	bool			true if successful, false otherwise
	 */
	virtual bool RetrieveFile(TARGETHANDLE /*Handle*/, const wchar_t* /*SourceFilename*/, const wchar_t* /*DestFilename*/)
	{
		return false;
	}

	/**
	 * Sets the name of the layout file for the DVD, so that GetDVDFileStartSector can be used
	 * 
	 * @param DVDLayoutFile Name of the layout file
	 *
	 * @return true if successful
	 */
	virtual bool SetDVDLayoutFile(const wchar_t* /*DVDLayoutFile*/)
	{
		return true;
	}
	/**
	 * Gets the starting sector of a file on the DVD (or whatever optical medium)
	 *
	 * @param DVDFilename Path to the file on the DVD
	 * @param SectorHigh High 32 bits of the sector location
	 * @param SectorLow Low 32 bits of the sector location
	 * 
	 * @return true if the start sector was found for the file
	 */
	virtual bool GetDVDFileStartSector(const wchar_t* /*DVDFilename*/, unsigned int& /*SectorHigh*/, unsigned int& /*SectorLow*/)
	{
		return false;
	}

	/**
	 * Reboots the target console. Must be implemented
	 *
	 * @param Handle The handle of the console to retrieve the information from.
	 * 
	 * @return true if successful
	 */
	virtual bool Reboot(TARGETHANDLE Handle) = 0;

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
	virtual bool RebootAndRun(TARGETHANDLE Handle, const wchar_t* Configuration, const wchar_t* BaseDirectory, const wchar_t* GameName, const wchar_t* URL, bool bForceGameName) = 0;

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
	virtual bool RunGame(TARGETHANDLE *TargetList, int NumTargets, const wchar_t* MapName, const wchar_t* URL, wchar_t* OutputConsoleCommand, int CommandBufferSize) = 0;
	
	/**
	 * Gets a list of targets that have been selected via menu's in UnrealEd.
	 *
	 * @param	OutTargetList			The list to be filled with target handles.
	 * @param	InOutTargetListSize		Contains the size of OutTargetList. When the function returns it contains the # of target handles copied over.
	 */
	virtual void GetMenuSelectedTargets(TARGETHANDLE *OutTargetList, int &InOutTargetListSize) = 0;

	/**
	 * Retrieve the state of the console (running, not running, crashed, etc)
	 *
	 * @param Handle The handle of the console to retrieve the information from.
	 *
	 * @return the current state
	 */
	virtual EConsoleState GetConsoleState(TARGETHANDLE Handle) = 0;

	/**
	 * Send a text command to the target
	 * 
	 * @param Handle The handle of the console to retrieve the information from.
	 * @param Command Command to send to the target
	 */
	virtual void SendConsoleCommand(TARGETHANDLE Handle, const wchar_t* Command) = 0;

	/**
	 * Have the console take a screenshot and dump to a file
	 * 
	 * @param Handle The handle of the console to retrieve the information from.
	 * @param Filename Location to place the .bmp file
	 *
	 * @return true if successful
	 */
	virtual bool ScreenshotBMP(TARGETHANDLE /*Handle*/, const wchar_t* /*Filename*/)
	{
		return false;
	}

	/**
	 * Return the number of console-specific menu items this platform wants to add to the main
	 * menu in UnrealEd.
	 *
	 * @return	The number of menu items for this console
	 */
	virtual int GetNumMenuItems() 
	{ 
		return 0; 
	}
	
	/**
	 * Return the string label for the requested menu item
	 * @param	Index		The requested menu item
	 * @param	bIsChecked	Is this menu item checked (or selected if radio menu item)
	 * @param	bIsRadio	Is this menu item part of a radio group?
	 * @param	OutHandle	Receives the handle of the target associated with the menu item.
	 *
	 * @return	Label for the requested menu item
	 */
	virtual const wchar_t* GetMenuItem(int /*Index*/, bool& /*bIsChecked*/, bool& /*bIsRadio*/, TARGETHANDLE& /*OutHandle*/)
	{ 
		return NULL; 
	}
	
	/**
	 * Internally process the given menu item when it is selected in the editor
	 * @param	Index		The selected menu item
	 * @param	OutputConsoleCommand	A buffer that the menu item can fill out with a console command to be run by the Editor on return from this function
	 * @param	CommandBufferSize		The size of the buffer pointed to by OutputConsoleCommand
	 */
	virtual void ProcessMenuItem(int /*Index*/, wchar_t* /*OutputConsoleCommand*/, int /*CommandBufferSize*/)
	{ 
	}

	/**
	 * Handles receiving a value from the application, when ProcessMenuItem returns that the ConsoleSupport needs to get a value
	 * @param	Value		The actual value received from user
	 */
	virtual void SetValueCallback(const wchar_t* /*Value*/) 
	{ 
	}

	/**
	 * Returns the global sound cooker object.
	 *
	 * @return global sound cooker object, or NULL if none exists
	 */
	virtual FConsoleSoundCooker* GetGlobalSoundCooker( void )
	{
		// Default implementations exist for the sake of e.g. WindowsTools, but should never be called.
		// PURE_VIRTUAL and appErrorf unfortunately cannot be used here because it's not defined for WindowsTools.
		return NULL;
	}

	/**
	 * Returns the global texture cooker object.
	 *
	 * @return global sound cooker object, or NULL if none exists
	 */
	virtual FConsoleTextureCooker* GetGlobalTextureCooker( void )
	{
		// Default implementations exist for the sake of e.g. WindowsTools, but should never be called.
		// PURE_VIRTUAL and appErrorf unfortunately cannot be used here because it's not defined for WindowsTools.
		return NULL;
	}

	/**
	 * Returns the global skeletal mesh cooker object.
	 *
	 * @return global skeletal mesh cooker object, or NULL if none exists
	 */
	virtual FConsoleSkeletalMeshCooker* GetGlobalSkeletalMeshCooker( void )
	{
		// Default implementations exist for the sake of e.g. WindowsTools, but should never be called.
		// PURE_VIRTUAL and appErrorf unfortunately cannot be used here because it's not defined for WindowsTools.
		return NULL;
	}

	/**
	 * Returns the global static mesh cooker object.
	 *
	 * @return global static mesh cooker object, or NULL if none exists
	 */
	virtual FConsoleStaticMeshCooker* GetGlobalStaticMeshCooker( void )
	{
		// Default implementations exist for the sake of e.g. WindowsTools, but should never be called.
		// PURE_VIRTUAL and appErrorf unfortunately cannot be used here because it's not defined for WindowsTools.
		return NULL;
	}

	/**
	 * Returns the global shader precompiler object.
	 * @return global shader precompiler object, or NULL if none exists.
	 */
	virtual FConsoleShaderPrecompiler* GetGlobalShaderPrecompiler()
	{
		// Default implementations exist for the sake of e.g. WindowsTools, but should never be called.
		// PURE_VIRTUAL and appErrorf unfortunately cannot be used here because it's not defined for WindowsTools.
		return NULL;
	}

	/**
	 * Frees memory allocated and returned by the console support module.
	 * @param Pointer - A pointer to the memory to be freed.
	 */
	virtual void FreeReturnedMemory(void* /*Pointer*/)
	{
	}

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
	virtual INT GetPlatformTextureSize(DWORD /*UnrealFormat*/, UINT /*Width*/, UINT /*Height*/, UINT /*NumMips*/, DWORD /*CreateFlags*/)
	{
		return 0;
	}

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
	virtual INT GetPlatformCubeTextureSize(DWORD /*UnrealFormat*/, UINT /*Size*/, UINT /*NumMips*/, DWORD /*CreateFlags*/)
	{
		return 0;
	}

	/**
	 * Retrieves the handle of the default console.
	 */
	virtual TARGETHANDLE GetDefaultTarget() = 0;

	/**
	 * This function exists to delete an instance of FConsoleSupport that has been allocated from a *Tools.dll. Do not call this function from the destructor.
	 */
	virtual void Destroy() = 0;

	/**
	 * Returns true if the specified target is connected for debugging and sending commands.
	 * 
	 *  @param Handle			The handle of the console to retrieve the information from.
	 */
	virtual bool GetIsTargetConnected(TARGETHANDLE Handle) = 0;

	/**
	 * Retrieves a handle to each available target.
	 *
	 * @param	OutTargetList			An array to copy all of the target handles into.
	 * @param	InOutTargetListSize		This variable needs to contain the size of OutTargetList. When the function returns it will contain the number of target handles copied into OutTargetList.
	 */
	virtual void GetTargets(TARGETHANDLE *OutTargetList, int *InOutTargetListSize) = 0;

	/**
	  * Retrieves the IP address for the debug channel at the specific target.
	  *
	  * @param	Handle	The handle to the target to retrieve the IP address for.
	  */
	virtual unsigned int GetDebugChannelIPAddress(TARGETHANDLE /*Handle*/)
	{
		return 0;
	}

	/**
	 * Returns the type of the specified target.
	 */
	virtual ETargetType GetTargetType(TARGETHANDLE /*Handle*/)
	{
		return TART_Unknown;
	}

	/**
	 * Sets flags controlling how crash reports will be filtered.
	 *
	 * @param	Handle	The handle to the target to set the filter for.
	 * @param	Filter	Flags controlling how crash reports will be filtered.
	 */
	virtual bool SetCrashReportFilter(TARGETHANDLE Handle, ECrashReportFilter Filter) = 0;

	/**
	 * Gets the name of a console as displayed by the target manager.
	 *
	 * @param	Handle	The handle to the target to set the filter for.
	 */
	virtual const wchar_t* GetTargetManagerNameForConsole(TARGETHANDLE Handle) = 0;

	/**
	 * Enumerates all available targets for the platform.
	 *
	 * @returns The number of available targets.
	 */
	virtual int EnumerateAvailableTargets() = 0;

	/**
	 * Gets the dump type the current target is set to.
	 *
	 * @param	Handle	The handle to the target to get the dump type from.
	 */
	virtual EDumpType GetDumpType(TARGETHANDLE /*Handle*/)
	{
		return DMPT_Normal;
	}

	/**
	 * Sets the dump type of the current target.
	 *
	 * @param	DmpType		The new dump type of the target.
	 * @param	Handle		The handle to the target to set the dump type for.
	 */
	virtual void SetDumpType(TARGETHANDLE /*Handle*/, EDumpType /*DmpType*/) {}

#ifdef WIN32
	/**
	 * Sets the callback function for TTY output.
	 *
	 * @param	Callback	Pointer to a callback function or NULL if TTY output is to be disabled.
	 * @param	Handle		The handle to the target that will register the callback.
	 */
	virtual void SetTTYCallback(TARGETHANDLE Handle, TTYEventCallbackPtr Callback) = 0;

	/**
	 * Sets the callback function for handling crashes.
	 *
	 * @param	Callback	Pointer to a callback function or NULL if handling crashes is to be disabled.
	 * @param	Handle		The handle to the target that will register the callback.
	 */
	virtual void SetCrashCallback(TARGETHANDLE Handle, CrashCallbackPtr Callback) = 0;

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
	virtual bool SyncFiles(TARGETHANDLE * /*Handles*/, int /*HandlesSize*/, const wchar_t ** /*SrcFilesToSync*/, const wchar_t ** /*DestFilesToSync*/, int /*FilesToSyncSize*/, const wchar_t ** /*DirectoriesToCreate*/, int /*DirectoriesToCreateSize*/, ColoredTTYEventCallbackPtr /*OutputCallback*/)
	{
		return true;
	}
#endif
};

// Typedef's for easy DLL binding.
typedef FConsoleSupport* (*FuncGetConsoleSupport) (void);

extern "C"
{
	/** 
	 * Returns a pointer to a subclass of FConsoleSupport.
	 *
	 * @return The pointer to the console specific FConsoleSupport
	 */
	CONSOLETOOLS_API FConsoleSupport* GetConsoleSupport();
}

#endif
#endif	// SUPPORT_NAMES_ONLY

