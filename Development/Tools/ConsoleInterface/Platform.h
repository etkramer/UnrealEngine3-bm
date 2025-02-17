#pragma once

#include "PlatformTarget.h"

using namespace System;
using namespace System::IO;
using namespace System::Collections::Generic;
using namespace System::Security::Cryptography;
using namespace System::Text;
using namespace System::Text::RegularExpressions;
using namespace System::Windows::Forms;

namespace ConsoleInterface
{
	//forward declarations
	ref class SharedSettings;
	ref class GameSettings;
	ref class TOCSettings;
	ref class TOCInfo;
	ref class Tag;
	ref class FileGroup;
	ref class FileFilter;
	ref class FileSet;

	[Flags]
	public enum class PlatformType
	{
		None = 0,
		PC = 1,
		PS3 = 1 << 1,
		Xbox360 = 1 << 2,
		All = PC | PS3 | Xbox360
	};

	public ref class Platform
	{
	private:
		ref class HwndWrapper : IWin32Window
		{
		private:
			IntPtr mHandle;

		public:
			property IntPtr Handle
			{
				virtual IntPtr get() { return mHandle; }
			}

			HwndWrapper(HWND hWnd) : mHandle(hWnd) {}
			HwndWrapper(IntPtr hWnd) : mHandle(hWnd) {}
		};

	private:
		literal String ^MANIFEST_EXTENSION = L".uncompressed_size";

		FConsoleSupport *mConsoleSupport;
		Dictionary<String^, PlatformTarget^> ^mTargets;
		SharedSettings ^mSharedSettings;
		PlatformType mType;

	public:
		property array<PlatformTarget^>^ Targets
		{
			array<PlatformTarget^>^ get();
		}

		property bool NeedsToSync
		{
			bool get();
		}

		property bool IsIntelByteOrder
		{
			bool get();
		}

		property String^ Name
		{
			String^ get();
		}

		property PlatformType Type
		{
			PlatformType get();
		}

		property PlatformTarget^ DefaultTarget
		{
			PlatformTarget^ get();
		}

	internal:
		property FConsoleSupport* ConsoleSupport
		{
			FConsoleSupport* get();
		}

	internal:
		Platform(PlatformType PlatType, FConsoleSupport *Support, SharedSettings ^Shared);

	private:
		void GetPathAndPattern( String^ FullName, String^& Path, String^& Pattern );
		List<TOCInfo^>^ TocBuild(String ^TOCName, TOCSettings ^BuildSettings, array<Tag^> ^SyncTags, String ^Language);
		void ProcessFileGroups(List<TOCInfo^> ^TOC, array<FileGroup^> ^FileGroups, TOCSettings ^BuildSettings, array<Tag^> ^SyncTags, List<String^> ^NotPlatforms, List<String^> ^NotLanguages, String ^Language);
		void TocAddFile(List<TOCInfo^> ^TOC, TOCSettings ^BuildSettings, FileInfo ^SrcFile, bool bIsForSync, bool bIsForTOC);
		int TocFindStartSector(String ^Directory, String ^Filename, TOCSettings ^BuildSettings);
		void TocIncludeFiles(List<TOCInfo^> ^TOC, TOCSettings ^BuildSettings, String ^SourceFolder, String ^SourcePattern, FileSet ^CurSet, FileGroup ^Group, List<String^> ^NotPlatforms, List<String^> ^NotLanguages, String ^Language);
		void FixString(String ^%Str, TOCSettings ^BuildSettings, String ^Language);
		void TocMerge(List<TOCInfo^> ^CurrentTOC, List<TOCInfo^> ^OldTOC, DateTime TocLastWriteTime);
		List<TOCInfo^>^ TocRead(String ^TOCPath);
		void TocGenerateCRC(List<TOCInfo^> ^TOC, String ^TOCFilename, TOCSettings ^BuildSettings);
		void TocWrite(List<TOCInfo^> ^TOC, String ^TOCPath);
		TOCInfo^ TocFindInfoFromConsoleName(List<TOCInfo^> ^TOC, String ^ConsoleName);
		bool PcInit(TOCSettings ^BuildSettings, List<String^> ^TargetPaths);
		bool PcSync(List<List<TOCInfo^>^> ^TOCList, TOCSettings ^BuildSettings);
		bool PcRequireCopy(TOCSettings ^BuildSettings, FileInfo ^SrcFile, FileInfo ^DstFile);
		bool PcCopyFile(TOCSettings ^BuildSettings, List<String^> ^CRCMismatchedFiles, String ^SourcePath, String ^DestPath, String ^SrcCRC);
		String^ CreateCRC(FileInfo ^SrcFile);
		bool FilterFileName(String ^FileName, String ^Filter);

	protected:
		!Platform();

	public:
		~Platform();
		
		virtual String^ ToString() override;
		List<TOCInfo^>^ GenerateTOC(String ^TagSetName, TOCSettings ^BuildSettings, String ^Language);
		bool TargetSync(TOCSettings ^BuildSettings, String ^TagSetName, bool bReboot, bool bNonInteractive);
		bool TargetSync(TOCSettings ^BuildSettings, bool bReboot, bool bNonInteractive);
		bool TargetSync(TOCSettings ^BuildSettings, List<List<TOCInfo^>^> ^TOCList, bool bReboot, bool bNonInteractive);
		int EnumerateAvailableTargets();

		static WORD GetApplicationSubsystem();
	};
}