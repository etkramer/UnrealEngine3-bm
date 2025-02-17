#pragma once

namespace StandaloneSymbolParser
{
	using namespace System;

	/**
	 * Base class for platform specific symbol parsers.
	 */
	public ref class SymbolParser abstract
	{
	public:
		/**
		 * Converted to virtual void Dispose().
		 */
		virtual ~SymbolParser() {} // Dispose()

		/**
		 * Loads symbols for an executable.
		 *
		 * @param	ExePath		The path to the executable whose symbols are going to be loaded.
		 */
		virtual bool LoadSymbols(String ^ExePath) = 0;

		/**
		 * Unloads any currently loaded symbols.
		 */
		virtual void UnloadSymbols() = 0;

		/**
		 * Retrieves the symbol info for an address.
		 *
		 * @param	Address			The address to retrieve the symbol information for.
		 * @param	OutFileName		The file that the instruction at Address belongs to.
		 * @param	OutFunction		The name of the function that owns the instruction pointed to by Address.
		 * @param	OutLineNumber	The line number within OutFileName that is represented by Address.
		 * @return	True if the function succeeds.
		 */
		virtual bool ResolveAddressToSymboInfo(unsigned int Address, String ^%OutFileName, String ^%OutFunction, int %OutLineNumber) = 0;
	};
}