#include "Stdafx.h"
#include "PS3SymbolParser.h"
#include <stdio.h>

using namespace System::IO;
using namespace System::Text;

namespace StandaloneSymbolParser
{
	/**
	 * Constructor.
	 */
	PS3SymbolParser::PS3SymbolParser() : CmdLineProcess(new FCommandLineWrapper())
	{

	}

	/**
	 * Converted to virtual void Dispose().
	 */
	PS3SymbolParser::~PS3SymbolParser()
	{
		this->!PS3SymbolParser();
		GC::SuppressFinalize(this);
	}

	/**
	 * Finalizer frees native memory.
	 */
	PS3SymbolParser::!PS3SymbolParser()
	{
		if(CmdLineProcess)
		{
			delete CmdLineProcess;
			CmdLineProcess = NULL;
		}
	}

	/**
	 * Loads symbols for an executable.
	 *
	 * @param	ExePath		The path to the executable whose symbols are going to be loaded.
	 */
	bool PS3SymbolParser::LoadSymbols(String ^ExePath)
	{
		// convert .elf to .xelf
		if(ExePath->EndsWith(L".elf"))
		{
			ExePath = ExePath->Replace(L".elf", ".xelf");
		}

		// make sure the file exists
		if(!File::Exists(ExePath))
		{
			System::Windows::Forms::OpenFileDialog ^OpenFileDlg = gcnew System::Windows::Forms::OpenFileDialog();

			OpenFileDlg->Title = L"Find .xelf file for the running PS3 game";
			OpenFileDlg->Filter = L".xelf Files|*.xelf";
			OpenFileDlg->RestoreDirectory = true;
			OpenFileDlg->DefaultExt = "xelf";
			OpenFileDlg->AddExtension = true;

			if(OpenFileDlg->ShowDialog() == System::Windows::Forms::DialogResult::OK)
			{
				ExePath = OpenFileDlg->FileName;
			}
			else
			{
				return false;
			}
		}

		// start an addr2line process
		String ^CmdLine = String::Format(L"ppu-lv2-addr2line -f -C -e {0}", ExePath);
		pin_ptr<Byte> NativeCmdLine = &Encoding::UTF8->GetBytes(CmdLine)[0];

		if(!CmdLineProcess->Create((char*)NativeCmdLine))
		{
			System::Windows::Forms::MessageBox::Show(nullptr, L"Failed to create ppu-lv2-addrline process. Check your path env variables.", L"Process creation error", System::Windows::Forms::MessageBoxButtons::OK, System::Windows::Forms::MessageBoxIcon::Error);
			return false;
		}

		// if we got here, we are good to go
		return true;
	}

	/**
	 * Unloads any currently loaded symbols.
	 */
	void PS3SymbolParser::UnloadSymbols()
	{
		CmdLineProcess->Terminate();
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
	bool PS3SymbolParser::ResolveAddressToSymboInfo(unsigned int Address, String ^%OutFileName, String ^%OutFunction, int %OutLineNumber)
	{
		// If there's no process running, just return an unknown string
		if(!CmdLineProcess->IsCreated())
		{
			OutFileName = L"Unknown";
			OutFunction = L"Unknown";
			OutLineNumber = 0;
		}
		else
		{
			// send a command to addr2line
			char Temp[128];
			sprintf_s(Temp, 128, "0x%x", Address);

			CmdLineProcess->Write(Temp);
			CmdLineProcess->Write("\n");

			// get answer back
			char *Function = CmdLineProcess->ReadLine(5000);
			char *Filename = CmdLineProcess->ReadLine(5000);

			// Find ':" separating file from line.
			char *Line	 = strstr( Filename, ":" );
			// NULL terminate filename by replacing ':' if there was one.
			if(Line)
			{
				*Line = 0;
				Line++;
				// Remainder of string is now line number.
				OutLineNumber = atoi(Line);
			}
			else
			{
				OutLineNumber = 0;
			}

			OutFileName = gcnew String(Filename);
			OutFunction = gcnew String(Function);
		}

		return true;
	}
}