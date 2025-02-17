#include "Stdafx.h"
#include "XenonSymbolParser.h"
#include <string>

using namespace System::Text;

namespace StandaloneSymbolParser
{
	/**
	 * Loads global debug information for xbox 360's.
	 */
	bool XenonSymbolParser::LoadDbgHelp()
	{
		if(!DbgLibraryHandle)
		{
			// Get the XEDK environment variable.
			wchar_t* XedkDir;
			size_t XedkDirSize;
			errno_t Err = _wdupenv_s(&XedkDir, &XedkDirSize, L"xedk");
			if(Err || !XedkDir)
			{
				OutputDebugStringW(L"Couldn't read xedk environment variable.\n");
				return false;
			}

			// Create a fully specified path to the XEDK version of dbghelp.dll
			// This is necessary because symsrv.dll must be in the same directory
			// as dbghelp.dll, and only a fully specified path can guarantee which
			// version of dbghelp.dll we load.
			std::wstring DbgHelpPath = std::wstring(XedkDir) + L"\\bin\\win32\\dbghelp.dll";

			// Free xedkDir
			if(XedkDir)
			{
				free(XedkDir);
			}

			// Call LoadLibrary on DbgHelp.DLL with our fully specified path.
			DbgLibraryHandle = LoadLibrary(DbgHelpPath.c_str());

			// Print an error message and return FALSE if DbgHelp.DLL didn't load.
			if(!DbgLibraryHandle)
			{
				OutputDebugStringW(L"ERROR: Couldn't load DbgHelp.dll from %xedk%\\bin\\win32.\n");
				return false;
			}

			mSymInitialize = (PFNSymInitialize)GetProcAddress(DbgLibraryHandle, "SymInitializeW");
			mSymCleanup = (PFNSymCleanup)GetProcAddress(DbgLibraryHandle, "SymCleanup");
			mSymSetOptions = (PFNSymSetOptions)GetProcAddress(DbgLibraryHandle, "SymSetOptions");
			mSymLoadModule = (PFNSymLoadModule)GetProcAddress(DbgLibraryHandle, "SymLoadModule64");
			mSymUnloadModule = (PFNSymUnloadModule)GetProcAddress(DbgLibraryHandle, "SymUnloadModule64");
			mSymGetSymFromAddr = (PFNSymGetSymFromAddr)GetProcAddress(DbgLibraryHandle, "SymGetSymFromAddr64");
			mSymGetLineFromAddr = (PFNSymGetLineFromAddr)GetProcAddress(DbgLibraryHandle, "SymGetLineFromAddr64");
		}

		return true;
	}

	/**
	 * Loads symbols for an executable.
	 *
	 * @param	ExePath		The path to the executable whose symbols are going to be loaded.
	 */
	bool XenonSymbolParser::LoadSymbols(String ^ExePath)
	{
		if(ExePath == nullptr)
		{
			throw gcnew ArgumentNullException(L"ExePath");
		}

		if(ExePath->Length == 0)
		{
			throw gcnew ArgumentException(L"ExePath has 0 length");
		}

		if(LoadDbgHelp())
		{
			if(!mSymInitialize((HANDLE)GetHashCode(), L"Xenon\\Lib\\XeDebug;Xenon\\Lib\\XeRelease;Xenon\\Lib\\XeReleaseLTCG;Xenon\\Lib\\XeReleaseLTCG-DebugConsole", FALSE))
			{
				UnloadSymbols();
				return false;
			}

			mSymSetOptions(SYMOPT_UNDNAME | SYMOPT_LOAD_LINES | SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_DEFERRED_LOADS | SYMOPT_DEBUG | SYMOPT_ALLOW_ABSOLUTE_SYMBOLS);

			pin_ptr<Byte> AsciiBytes = &Encoding::UTF8->GetBytes(ExePath)[0];
			std::string AsciiPath((char*)AsciiBytes);

			DWORD64 BaseAddress = mSymLoadModule((HANDLE)GetHashCode(), 0, AsciiPath.c_str(), NULL, 0, 0);
			if(!BaseAddress)
			{
				UnloadSymbols();
				return false;
			}
		}
		else
		{
			return false;
		}

		return true;
	}

	/**
	 * Unloads any currently loaded symbols.
	 */
	void XenonSymbolParser::UnloadSymbols()
	{
		mSymCleanup((HANDLE)GetHashCode());
	}

	/**
	 * Retrieves the symbol info for an address.
	 *
	 * @param	Address			The address to retrieve the symbol information for.
	 * @param	OutFileName		The file that the instruction at Address belongs to.
	 * @param	OutFunction		The name of the function that owns the instruction pointed to by Address.
	 * @param	OutLineNumber	The line number within OutFileName that is represented by Address.
	 * @return	True if the function succeeds.
	 */
	bool XenonSymbolParser::ResolveAddressToSymboInfo(unsigned int Address, String ^%OutFileName, String ^%OutFunction, int %OutLineNumber)
	{
		IMAGEHLP_SYMBOL64_PACKAGE	Symbol;
		IMAGEHLP_LINE64		Line = { 0 };

		// Set required IMAGEHLP_SYMBOL fields.
		Symbol.sym.SizeOfStruct = sizeof(Symbol);
		Symbol.sym.MaxNameLength = MAX_SYM_NAME;

		DWORD64 dwSymbolDisplacement = 0;
		// Clear output IMAGEHLP_SYMBOL fields.
		Symbol.sym.Address = 0;
		Symbol.sym.Size = 0;
		Symbol.sym.Flags = 0;
		Symbol.sym.Name[0] = '\0';

		if(!mSymGetSymFromAddr((HANDLE)GetHashCode(), Address, &dwSymbolDisplacement, &Symbol.sym))
		{
			return false;
		}

		System::Diagnostics::Debug::Assert(Address - Symbol.sym.Address == dwSymbolDisplacement );

		DWORD dwLineDisplacement = 0;
		Line.SizeOfStruct = sizeof( Line );
		mSymGetLineFromAddr((HANDLE)GetHashCode(), Address, &dwLineDisplacement, &Line);

		OutFileName = gcnew String(Line.FileName ? Line.FileName : "???");
		OutFunction = gcnew String(Symbol.sym.Name);
		OutLineNumber = Line.LineNumber;

		return true;
	}
}