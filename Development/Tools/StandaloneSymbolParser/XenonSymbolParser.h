#pragma once

#include "SymbolParser.h"

// specify the following is native code
#pragma managed(push, off)

#include <Windows.h>
#include <ImageHlp.h>

// Function pointer for initializing symbols for a process
typedef BOOL ( WINAPI *PFNSymInitialize )(
	IN HANDLE		hProcess,
	IN PWSTR		UserSearchPath,
	IN BOOL			fInvadeProcess
	);

// Function pointer for cleaning up the symbols in a process
typedef BOOL ( WINAPI *PFNSymCleanup )(IN HANDLE hProcess);

// Function Pointer for setting symbol options
typedef DWORD ( WINAPI * PFNSymSetOptions )(IN DWORD SymOptions);

// Function pointer for loading module symbols
typedef DWORD64 ( WINAPI *PFNSymLoadModule )(
	IN  HANDLE      hProcess,
	IN  HANDLE      hFile,
	IN  PCSTR        ImageName,
	IN  PCSTR        ModuleName,
	IN  DWORD64     BaseOfDll,
	IN  DWORD       SizeOfDll
	);

// Function pointer for unloading module symbols
typedef BOOL ( WINAPI *PFNSymUnloadModule )(
	HANDLE			hProcess,
	DWORD64			BaseOfDll
	);

// Function pointer for resolving symbols
typedef BOOL ( WINAPI *PFNSymGetSymFromAddr )(
	IN  HANDLE				hProcess,
	IN  DWORD64             qwAddr,
	OUT PDWORD64            pdwDisplacement,
	OUT PIMAGEHLP_SYMBOL64  Symbol
	);

// Function pointer for resolving symbols
typedef BOOL ( WINAPI *PFNSymGetLineFromAddr )(
	IN  HANDLE              hProcess,
	IN  DWORD64				qwAddr,
	OUT PDWORD				pdwDisplacement,
	OUT PIMAGEHLP_LINE64	Line
	);

#pragma managed(pop)

namespace StandaloneSymbolParser
{
	/**
	 * Class for loading xbox 360 symbols.
	 */
	public ref class XenonSymbolParser : public SymbolParser
	{
	private:
		static HMODULE DbgLibraryHandle;
		static PFNSymInitialize mSymInitialize;
		static PFNSymCleanup mSymCleanup;
		static PFNSymSetOptions mSymSetOptions;
		static PFNSymLoadModule mSymLoadModule;
		static PFNSymUnloadModule mSymUnloadModule;
		static PFNSymGetSymFromAddr mSymGetSymFromAddr;
		static PFNSymGetLineFromAddr mSymGetLineFromAddr;

		/**
		 * Loads global debug information for xbox 360's.
		 */
		static bool LoadDbgHelp();

	public:
		/**
		 * Loads symbols for an executable.
		 *
		 * @param	ExePath		The path to the executable whose symbols are going to be loaded.
		 */
		virtual bool LoadSymbols(String ^ExePath) override;

		/**
		 * Unloads any currently loaded symbols.
		 */
		virtual void UnloadSymbols() override;

		/**
		 * Retrieves the symbol info for an address.
		 *
		 * @param	Address			The address to retrieve the symbol information for.
		 * @param	OutFileName		The file that the instruction at Address belongs to.
		 * @param	OutFunction		The name of the function that owns the instruction pointed to by Address.
		 * @param	OutLineNumber	The line number within OutFileName that is represented by Address.
		 * @return	True if the function succeeds.
		 */
		virtual bool ResolveAddressToSymboInfo(unsigned int Address, String ^%OutFileName, String ^%OutFunction, int %OutLineNumber) override;
	};
}