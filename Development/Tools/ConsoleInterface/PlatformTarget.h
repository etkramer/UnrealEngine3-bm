#pragma once

#include <string>

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;

namespace ConsoleInterface
{
	// forward declarations
	ref class Platform;
	ref class TOCSettings;
	ref class TOCInfo;

	// NOTE: These enums are mirrored in the FConsoleSupport class (UnConsoleTools.h)
	public enum class EProfileType
	{
		PT_Invalid,
		PT_Script,
		PT_GameTick,
		PT_RenderTick,
	};

	public enum class TargetState
	{
		NotRunning = 0,
		Rebooting,
		Running,
		Crashed,
		Asserted,
		RIP,
	};

	public enum class TargetType
	{
		Unknown = 0,
		TestKit,
		DevKit,
		ReviewerKit
	};

	[Flags]
	public enum class CrashReportFilter
	{
		None = 0,
		Debug = 1,
		Release = 1 << 1,
		ReleaseForShip = 1 << 2,
		All = Debug | Release | ReleaseForShip
	};

	public enum class DumpType
	{
		Normal = 0,
		WithFullMemory
	};

	public value struct TargetHandle
	{
		IntPtr mHandle;

		TargetHandle(TARGETHANDLE Handle) : mHandle(Handle) {}
		TargetHandle(IntPtr Handle) : mHandle(Handle) {}

		static operator TARGETHANDLE(TargetHandle Handle)
		{
			return (TARGETHANDLE)Handle.mHandle.ToPointer();
		}
	};

	public delegate void TTYOutputDelegate(IntPtr Txt);
	public delegate void CrashCallbackDelegate(IntPtr CallStackPtr, IntPtr MiniDumpLocationPtr);

	public ref class PlatformTarget
	{
	private:
		Platform ^mOwner;
		TTYOutputDelegate ^mTTYCallback;
		CrashCallbackDelegate ^mCrashCallback;
		Dictionary<String^, String^> ^ExistingDirectories;
		TargetHandle mHandle;

		static bool mHasParsedHosts = false;
		static Dictionary<unsigned int, String^> ^mHosts = gcnew Dictionary<unsigned int, String^>();

		// properties
	internal:
		/// <summary>
		/// Gets the native target handle.
		/// </summary>
		property TargetHandle Handle
		{
			TargetHandle get();
		}

	public:
		/// <summary>
		/// Gets the name of the target.
		/// </summary>
		property String^ Name
		{
			String^ get();
		}

		/// <summary>
		/// Gets Title IP Address for the target.
		/// </summary>
		property Net::IPAddress^ IPAddress
		{
			Net::IPAddress^ get();
		}

		/// <summary>
		/// Gets the debug channel IP address.
		/// </summary>
		property Net::IPAddress^ DebugIPAddress
		{
			Net::IPAddress^ get();
		}

		/// <summary>
		/// Gets the current state of the target.
		/// </summary>
		property TargetState State
		{
			TargetState get();
		}

		/// <summary>
		/// Gets whether or not the target has been connected to.
		/// </summary>
		property bool IsConnected
		{
			bool get();
		}

		/// <summary>
		/// Gets whether or not the target is the default target.
		/// </summary>
		property bool IsDefault
		{
			bool get();
		}

		/// <summary>
		/// Gets the platform the target belongs to.
		/// </summary>
		property Platform^ ParentPlatform
		{
			Platform^ get();
		}

		/// <summary>
		/// Gets the type of the target.
		/// </summary>
		property TargetType ConsoleType
		{
			TargetType get();
		}

		/// <summary>
		/// Sets crash report filters for the target.
		/// </summary>
		property CrashReportFilter CrashFilter
		{
			void set(CrashReportFilter Value);
		}

		/// <summary>
		/// Gets the name the target manager associates with the target.
		/// </summary>
		property String^ TargetManagerName
		{
			String^ get();
		}

		/// <summary>
		/// Gets/Sets the type of crash dumps generated by the target.
		/// </summary>
		property DumpType CrashDumpType
		{
			DumpType get();
			void set(DumpType DmpType);
		}

		// functions
	private:
		/// <summary>
		/// Parses the hosts file for resolving IP addresses.
		/// </summary>
		static void ParseHostsFile();

	public:
		/// <summary>
		/// Constructor.
		/// </summary>
		/// <param name="Handle">The handle to the target.</param>
		/// <param name="Owner">The platform the target belongs to.</param>
		PlatformTarget(TargetHandle Handle, Platform ^Owner);
		
		/// <summary>
		/// Establishes a debug connection with the target.
		/// </summary>
		/// <returns>True if a connection has been made with the target.</returns>
		bool Connect();

		/// <summary>
		/// Disconnects from the target.
		/// </summary>
		void Disconnect();

		/// <summary>
		/// Creates a directory on the target.
		/// </summary>
		/// <param name="DirectoryName">The name of the directory to be created.</param>
		/// <returns>True if the directory was successfully created.</returns>
		bool MakeDirectory(String ^DirectoryName);

		/// <summary>
		/// Creates a directory hierarchy on the target.
		/// </summary>
		/// <param name="DirectoryHierarchy">The directory hierarchy to be created.</param>
		/// <returns>True if the directory hierarchy was successfully was created.</returns>
		bool MakeDirectoryHierarchy(String ^DirectoryHierarchy);

		/// <summary>
		/// Checks if a file is up to date on the target.
		/// </summary>
		/// <param name="SourceFilename">The file to be copied.</param>
		/// <param name="DestFileName">The file to be overwritten.</param>
		/// <param name="bReverse">True if the source file and dest file are to be switched.</param>
		/// <returns>True if the source file needs to be copied.</returns>
		bool NeedsToCopyFile(String ^SourceFilename, String ^DestFileName, bool bReverse);

		/// <summary>
		/// Copies a file to the target.
		/// </summary>
		/// <param name="SourceFilename">The file to be copied.</param>
		/// <param name="DestFileName">The file to be written to.</param>
		/// <returns>True if the source file was successfully copied.</returns>
		bool CopyFileToTarget(String ^SourceFilename, String ^DestFileName);

		/// <summary>
		/// Copies a file from the target.
		/// </summary>
		/// <param name="SourceFilename">The file to be copied.</param>
		/// <param name="DestFileName">The file to be written to.</param>
		/// <returns>True if the source file was successfully copied.</returns>
		bool RetrieveFileFromTarget(String ^SourceFilename, String ^DestFileName);

		/// <summary>
		/// Reboots the target.
		/// </summary>
		/// <returns>True if the target was successfully rebooted.</returns>
		bool Reboot();

		/// <summary>
		/// Reboots the target and tells it to start loading the specified game.
		/// </summary>
		/// <param name="Configuration">The configuration the game will run on.</param>
		/// <param name="BaseDirectory">The base directory of the game on the target HD.</param>
		/// <param name="GameName">The name of the game to run.</param>
		/// <param name="URL">The URL containing extra command line parameters.</param>
		/// <param name="bForceGameName">If true then GameName must reference the full name of the executable within BaseDirectory (i.e. ExampleGame-XeDebug.xex).</param>
		/// <returns>True the target was rebooted and has started loading the game.</returns>
		bool RebootAndRun(String ^Configuration, String ^BaseDirectory, String ^GameName, String ^URL, bool bForceGameName);

		/// <summary>
		/// Takes a screen shot.
		/// </summary>
		/// <param name="FileName">The local file name of the screen shot (.bmp).</param>
		/// <returns>True if the screen shot was taken and saved to disk.</returns>
		bool ScreenShot(String ^FileName);

		/// <summary>
		/// Copies files needed to run a game to the target.
		/// </summary>
		/// <param name="BuildSettings">Information about the files to copy.</param>
		/// <param name="TOCInfo">Table of contents.</param>
		void Sync(TOCSettings ^BuildSettings, TOCInfo ^Entry);

		/// <summary>
		/// Prepares the target for syncing.
		/// </summary>
		void InitializeSyncing();

		/// <summary>
		/// Sends a debug command to the target.
		/// </summary>
		/// <param name="Cmd">The command to send.</param>
		void SendCommand(String ^Cmd);

		/// <summary>
		/// Sets the callback function for TTY output.
		/// </summary>
		/// <param name="TTYCallback">The callback.</param>
		void SetTTYCallback(TTYOutputDelegate ^TTYCallback);

		/// <summary>
		/// Sets the callback function for handling crashes.
		/// </summary>
		/// <param name="CrashCallback">The callback.</param>
		void SetCrashCallback(CrashCallbackDelegate ^CrashCallback);

		/// <summary>
		/// Generates a string representation of the target.
		/// </summary>
		/// <returns>The name of the target.</returns>
		virtual String^ ToString() override;

		/// <summary>
		/// Attempts to find the supplied IP address in the host file and resolve its name.
		/// </summary>
		/// <param name="IpAddr>A string containing the IP address to resolve.</param>
		/// <returns>The resolved name if it exists otherwise IpAddr.</returns>
		static String^ ResolveIPFromHostFile(String ^IpAddr);
	};
}