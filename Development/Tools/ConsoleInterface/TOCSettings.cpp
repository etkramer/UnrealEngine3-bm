#include "Stdafx.h"
#include "TOCSettings.h"
#include "GameSettings.h"
#include "OutputEventArgs.h"

using namespace System::IO;

namespace ConsoleInterface
{
	TOCSettings::TOCSettings(OutputHandlerDelegate ^OutputHandler) 
		: Languages(gcnew array<String^> { L"INT" }), GameName(L"ExampleGame"), TargetBaseDirectory(L"UnrealEngine3"), IsForShip(false), HasDVDLayout(false),
		GenericVars(gcnew Dictionary<String^, String^>()), DestinationPaths(gcnew List<String^>()), 
		MergeExistingCRC(false), ComputeCRC(false), GameOptions(gcnew GameSettings()), mOutputHandler(OutputHandler), NoSync(false),
		VerifyCopy(false), Force(false), SleepDelay(0), TargetsToSync(gcnew List<String^>()),
		GenerateTOC(true), SyncToHost(false)
	{
		try
		{
			String ^FullPath = Path::GetFullPath("..");
			FullPath = FullPath->TrimEnd(gcnew array<Char> { '\\' });
			
			int FinalSeparator = FullPath->LastIndexOf('\\');

			if(FinalSeparator != -1)
			{
				TargetBaseDirectory = FullPath->Substring(FinalSeparator + 1);
			}
		}
		catch(Exception ^ex)
		{
			//NOTE: TargetBaseDirectory should still default to UnrealEngine3
			System::Diagnostics::Debug::WriteLine(ex->ToString());
		}
	}

	void TOCSettings::Write(Color TxtColor, String ^Message, ... array<Object^> ^Parms)
	{
		if(mOutputHandler)
		{
			mOutputHandler(this, gcnew OutputEventArgs(TxtColor, String::Format(Message, Parms)));
		}
	}

	void TOCSettings::WriteLine(Color TxtColor, String ^Message, ... array<Object^> ^Parms)
	{
		if(mOutputHandler)
		{
			mOutputHandler(this, gcnew OutputEventArgs(TxtColor, String::Concat(String::Format(Message, Parms), Environment::NewLine)));
		}
	}

	void TOCSettings::WriteLine(Color TxtColor, String ^Message)
	{
		if(mOutputHandler)
		{
			mOutputHandler(this, gcnew OutputEventArgs(TxtColor, String::Concat(Message, Environment::NewLine)));
		}
	}

	void TOCSettings::WriteLine(unsigned int TxtColor, String ^Message)
	{
		if(mOutputHandler)
		{
			mOutputHandler(this, gcnew OutputEventArgs(Color::FromArgb(TxtColor), String::Concat(Message, Environment::NewLine)));
		}
	}
}