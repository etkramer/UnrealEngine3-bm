/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

// This is the main DLL file.

#include "stdafx.h"
#include "ConsoleInterface.h"
#include "EnumeratingPlatformsForm.h"
#include "SharedSettings.h"

using namespace System::IO;
using namespace System::Xml;
using namespace System::Xml::Serialization;
using namespace System::Threading;

// freacken macro's clash with .net types
#undef GetCurrentDirectory
#undef SetCurrentDirectory

namespace ConsoleInterface
{
	ICollection<Platform^>^ DLLInterface::Platforms::get()
	{
		return mPlatforms->Values;
	}

	int DLLInterface::NumberOfPlatforms::get()
	{
		return mPlatforms->Count;
	}

	void DLLInterface::EnumeratingPlatformsUIThread(Object ^State)
	{
		ManualResetEvent ^Event = (ManualResetEvent^)State;

		if(Event != nullptr)
		{
			Application::Run(gcnew EnumeratingPlatformsForm(Event));
		}
	}

	FConsoleSupport* DLLInterface::LoadPlatformDLL(String ^DllPath)
	{
		if(DllPath == nullptr)
		{
			throw gcnew ArgumentNullException(L"Path");
		}

		//NOTE: We have to do this in case the target dll links to any dll's at load time.
		String^ CurDir = Directory::GetCurrentDirectory();
		DllPath = Path::GetFullPath(DllPath);
		String^ NewDir = Path::GetDirectoryName(DllPath);

		if(Directory::Exists(NewDir))
		{
			Directory::SetCurrentDirectory(NewDir);

			pin_ptr<const wchar_t> NativePath = PtrToStringChars(Path::GetFileName(DllPath));

			//NOTE: we'll leave this in our address space and let it be unloaded when the process is destroyed
			HMODULE Module = LoadLibraryW(NativePath);

			// restore our working directory
			Directory::SetCurrentDirectory(CurDir);

			if(Module)
			{
				FuncGetConsoleSupport SupportFunc = (FuncGetConsoleSupport)GetProcAddress(Module, "GetConsoleSupport");

				if(SupportFunc)
				{
					return SupportFunc();
				}
			}
		}

		return NULL;
	}

	bool DLLInterface::HasPlatform(PlatformType PlatformToCheck)
	{
		return mPlatforms->ContainsKey(PlatformToCheck);
	}

	PlatformType DLLInterface::LoadPlatforms(PlatformType PlatformsToLoad)
	{
		if(mSharedSettings == nullptr)
		{
			try
			{
				System::String ^EntryAssemblyLocation = System::Reflection::Assembly::GetEntryAssembly()->Location;
				System::String ^EntryAssemblyDirectory = Path::GetDirectoryName( EntryAssemblyLocation );

				XmlReader ^Reader = XmlReader::Create( Path::Combine( EntryAssemblyDirectory, L"CookerSync.xml") );
				XmlSerializer ^Serializer = gcnew XmlSerializer(SharedSettings::typeid);

				mSharedSettings = (SharedSettings^)Serializer->Deserialize(Reader);

				delete Serializer;
				delete Reader;
			}
			catch(Exception ^ex)
			{
				String ^Error = ex->ToString();
				Console::WriteLine(Error);
				System::Diagnostics::Debug::WriteLine(Error);
				mSharedSettings = gcnew SharedSettings();
			}
		}

		bool bSuccess = true;

		if((PlatformsToLoad & PlatformType::PC) == PlatformType::PC && !mPlatforms->ContainsKey(PlatformType::PC))
		{
			FConsoleSupport *Support = LoadPlatformDLL(L"Windows\\WindowsTools.dll");

			if(Support)
			{
				mPlatforms->Add(PlatformType::PC, gcnew Platform(PlatformType::PC, Support, mSharedSettings));
			}
			else
			{
				PlatformsToLoad = (PlatformType)(PlatformsToLoad & ~PlatformType::PC);
			}
		}

		if((PlatformsToLoad & PlatformType::Xbox360) == PlatformType::Xbox360 && !mPlatforms->ContainsKey(PlatformType::Xbox360))
		{
			FConsoleSupport *Support = LoadPlatformDLL(L"Xenon\\XeTools.dll");

			if(Support)
			{
				mPlatforms->Add(PlatformType::Xbox360, gcnew Platform(PlatformType::Xbox360, Support, mSharedSettings));
			}
			else
			{
				PlatformsToLoad = (PlatformType)(PlatformsToLoad & ~PlatformType::Xbox360);
			}
		}

		if((PlatformsToLoad & PlatformType::PS3) == PlatformType::PS3 && !mPlatforms->ContainsKey(PlatformType::PS3))
		{
			FConsoleSupport *Support = LoadPlatformDLL(L"PS3\\PS3Tools.dll");

			if(Support)
			{
				mPlatforms->Add(PlatformType::PS3, gcnew Platform(PlatformType::PS3, Support, mSharedSettings));
			}
			else
			{
				PlatformsToLoad = (PlatformType)(PlatformsToLoad & ~PlatformType::PS3);
			}
		}

		return PlatformsToLoad;
	}

	bool DLLInterface::TryGetPlatform(PlatformType PlatformToGet, Platform ^%OutPlatform)
	{
		return mPlatforms->TryGetValue(PlatformToGet, OutPlatform);
	}
}