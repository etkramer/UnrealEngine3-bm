#pragma once

#include "SymbolParser.h"
#include "..\..\Src\PS3\PS3Tools\CommandLineWrapper.h"

namespace StandaloneSymbolParser
{
	/**
	 * Class for loading PS3 symbols.
	 */
	public ref class PS3SymbolParser : public SymbolParser
	{
	private:
		FCommandLineWrapper *CmdLineProcess;

	protected:
		/**
		 * Finalizer frees native memory.
		 */
		!PS3SymbolParser(); // finalizer

	public:
		/**
		 * Constructor.
		 */
		PS3SymbolParser();

		/**
		 * Converted to virtual void Dispose().
		 */
		virtual ~PS3SymbolParser(); // Dispose()

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