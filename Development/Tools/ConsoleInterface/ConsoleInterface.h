/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#pragma once

#include "Platform.h"

#define NUM_PLATFORMS 4

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;

namespace ConsoleInterface
{
	//forward declarations
	ref class SharedSettings;

	//delegates
	public delegate void TTYEventCallbackDelegate(IntPtr Data, int Length);
	public delegate void SetTTYEventCallbackDelegate([MarshalAs(UnmanagedType::FunctionPtr)]TTYEventCallbackDelegate ^Callback);

	public delegate void ProfilerEventCallbackDelegate(IntPtr Data, int Length);
	public delegate void SetProfilerEventCallbackDelegate([MarshalAs(UnmanagedType::FunctionPtr)]ProfilerEventCallbackDelegate ^Callback);

	public ref class DLLInterface
	{
	private:
		static Dictionary<PlatformType, Platform^> ^mPlatforms = gcnew Dictionary<PlatformType, Platform^>();
		static SharedSettings ^mSharedSettings;

	private:
		static FConsoleSupport* LoadPlatformDLL(String ^DllPath);
		static void EnumeratingPlatformsUIThread(Object ^State);

	public:
		static property ICollection<Platform^>^ Platforms
		{
			ICollection<Platform^>^ get();
		}

		static property int NumberOfPlatforms
		{
			int get();
		}

		static bool HasPlatform(PlatformType PlatformToCheck);
		static PlatformType LoadPlatforms(PlatformType PlatformsToLoad);
		static bool TryGetPlatform(PlatformType PlatformToGet, Platform ^%OutPlatform);
	};
}
