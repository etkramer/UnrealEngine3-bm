#pragma once

using namespace System;
using namespace System::Drawing;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;

namespace ConsoleInterface
{
	//forward declarations
	ref class GameSettings;
	ref class OutputEventArgs;

	public delegate void OutputHandlerDelegate(Object ^sender, OutputEventArgs ^e);
	public delegate void WriteLineNativeDelegate(unsigned int TxtColor, [MarshalAs(UnmanagedType::LPWStr)] String ^Message);

	public ref class TOCSettings
	{
	private:
		OutputHandlerDelegate ^mOutputHandler;

	public:
		array<String^> ^Languages;
		String ^GameName;
		String ^TargetBaseDirectory;
		Dictionary<String^, String^> ^GenericVars;
		List<String^> ^DestinationPaths;
		GameSettings ^GameOptions;
		List<String^> ^TargetsToSync;
		bool IsForShip;
		bool HasDVDLayout;
		bool MergeExistingCRC;
		bool ComputeCRC;
		bool NoSync;
		bool VerifyCopy;
		bool Force;
		bool SyncToHost;
		bool GenerateTOC;
		int SleepDelay;

	public:
		TOCSettings(OutputHandlerDelegate ^OutputHandler);

		void Write(Color TxtColor, String ^Message, ... array<Object^> ^Parms);
		void WriteLine(Color TxtColor, String ^Message, ... array<Object^> ^Parms);
		void WriteLine(Color TxtColor, String ^Message);
		void WriteLine(unsigned int TxtColor, String ^Message);
	};
}