#include "Stdafx.h"
#include "Platform.h"
#include "Tag.h"
#include "TOCSettings.h"
#include "TOCInfo.h"
#include "FileGroup.h"
#include "FileFilter.h"
#include "TagSet.h"
#include "SharedSettings.h"
#include "GameSettings.h"
#include "FileSet.h"
#include "SyncFileSocket.h"
#include "ConnectToConsoleRetryForm.h"
#include <stdio.h>
#include <vector>

using namespace System::Xml;
using namespace System::Xml::Serialization;
using namespace System::Reflection;
using namespace System::Net;
using namespace System::Net::Sockets;
using namespace System::Diagnostics;

#define VALIDATE_SUPPORT(x) if(!(x)) throw gcnew InvalidOperationException(L"Invalid platform!")
#undef GetCurrentDirectory

namespace ConsoleInterface
{
	array<PlatformTarget^>^ Platform::Targets::get()
	{
		Dictionary<String^, PlatformTarget^>::ValueCollection ^ValCollection = mTargets->Values;

		array<PlatformTarget^> ^Result = gcnew array<PlatformTarget^>(ValCollection->Count);
		ValCollection->CopyTo(Result, 0);

		return Result;
	}

	bool Platform::NeedsToSync::get()
	{
		VALIDATE_SUPPORT(mConsoleSupport);

		return mConsoleSupport->PlatformNeedsToCopyFiles();
	}

	bool Platform::IsIntelByteOrder::get()
	{
		VALIDATE_SUPPORT(mConsoleSupport);

		return mConsoleSupport->GetIntelByteOrder();
	}

	FConsoleSupport* Platform::ConsoleSupport::get()
	{
		VALIDATE_SUPPORT(mConsoleSupport);

		return mConsoleSupport;
	}

	String^ Platform::Name::get()
	{
		return mType.ToString();
	}

	PlatformType Platform::Type::get()
	{
		return mType;
	}

	PlatformTarget^ Platform::DefaultTarget::get()
	{
		VALIDATE_SUPPORT(mConsoleSupport);

		PlatformTarget ^DefaultTarget = nullptr;

		for each(PlatformTarget ^CurTarget in mTargets->Values)
		{
			if(CurTarget->IsDefault)
			{
				DefaultTarget = CurTarget;
				break;
			}
		}

		return DefaultTarget;
	}

	Platform::Platform(PlatformType PlatType, FConsoleSupport *Support, SharedSettings ^Shared)
	{
		if(Support == NULL)
		{
			throw gcnew ArgumentNullException(L"Support");
		}

		if(Shared == nullptr)
		{
			throw gcnew ArgumentNullException(L"Shared");
		}

		mSharedSettings = Shared;
		mType = PlatType;
		mConsoleSupport = Support;
		mTargets = gcnew Dictionary<String^, PlatformTarget^>();
	}

	Platform::~Platform()
	{
		this->!Platform();
		GC::SuppressFinalize(this);
	}

	Platform::!Platform()
	{
		mConsoleSupport->Destroy();
		mConsoleSupport = NULL;
	}

	String^ Platform::ToString()
	{
		return mType.ToString();
	}

	List<TOCInfo^>^ Platform::GenerateTOC(String ^TagSetName, TOCSettings ^BuildSettings, String ^Language)
	{
		if(BuildSettings == nullptr)
		{
			throw gcnew ArgumentNullException(L"BuildSettings");
		}

		if(Language == nullptr)
		{
			throw gcnew ArgumentNullException(L"Language");
		}

		if(BuildSettings->Languages == nullptr || BuildSettings->Languages->Length == 0)
		{
			BuildSettings->Languages = gcnew array<String^> { L"INT" };
		}

		if(Language->Length != 3)
		{
			throw gcnew System::ArgumentException("Languages must be 3 characters long i.e. \"INT\".", "Language");
		}

		Language = Language->ToUpper();
		String ^TOCFilename = Language->Equals(L"INT") ? String::Format(L"{0}TOC.txt", mType.ToString()) : String::Format(L"{0}TOC_{1}.txt", mType.ToString(), Language);
		String ^TOCPath = Path::Combine(Directory::GetCurrentDirectory(), String::Format(L"..\\{0}\\{1}", BuildSettings->GameName, TOCFilename));
		array<Tag^> ^SyncTags = nullptr;

		if(TagSetName != nullptr)
		{
			for each(TagSet ^CurSet in mSharedSettings->TagSets)
			{
				if(TagSetName->Equals(CurSet->Name,StringComparison::OrdinalIgnoreCase))
				{
					SyncTags = CurSet->Tags;

					// Display the tagset we're using
					BuildSettings->WriteLine(Color::Black, L"Using tagset: {0}", CurSet->Name);
					break;
				}
			}

			if( SyncTags == nullptr )
			{
				BuildSettings->WriteLine(Color::Black, L"No tagset found matching: {0}", TagSetName);
			}
		}

		// Build up the file-list (we always need to do this)
		List<TOCInfo^> ^TOC = TocBuild(TOCFilename, BuildSettings, SyncTags, Language);

		// Merge the CRCs from the existing TOC file (if there is one)

		if(BuildSettings->MergeExistingCRC)
		{
			FileInfo ^TocInfo = gcnew FileInfo(TOCPath);

			if(TocInfo->Exists)
			{
				List<TOCInfo^> ^PreviousTOC = TocRead(TOCPath);
				TocMerge(TOC, PreviousTOC, TocInfo->LastWriteTimeUtc);
			}
		}

		if(BuildSettings->ComputeCRC)
		{
			TocGenerateCRC(TOC, TOCFilename, BuildSettings);
		}

		if(BuildSettings->GenerateTOC)
		{
			TocWrite(TOC, TOCPath);
		}

		return TOC;
	}

	void Platform::GetPathAndPattern( String^ FullName, String^& Path, String^& Pattern )
	{
		// find the last slash in the path
		int LastSlash = FullName->LastIndexOfAny( gcnew array<Char> { L'/', L'\\' } );

		// if no slash, use the current directory
		if( LastSlash == -1 )
		{
			Pattern = FullName;
		}
		else
		{
			// get the first bit including the slash
			Path = FullName->Substring( 0, LastSlash + 1 );
			Pattern = FullName->Substring( LastSlash + 1 );
		}
	}

	List<TOCInfo^>^ Platform::TocBuild(String ^TOCName, TOCSettings ^BuildSettings, array<Tag^> ^SyncTags, String ^Language)
	{
		if(BuildSettings == nullptr)
		{
			throw gcnew ArgumentNullException(L"BuildSettings");
		}

		BuildSettings->WriteLine(Color::Black, L"Generating {0}...", TOCName);

		List<TOCInfo^> ^TOC = gcnew List<TOCInfo^>();
		List<String^> ^NotPlatforms = gcnew List<String^>();
		List<String^> ^NotLanguages = gcnew List<String^>();
		String ^PlatformStr = mType.ToString();
		int CurTime = Environment::TickCount;

		// add all platforms not this one
		for each(Tag ^PlatformTag in mSharedSettings->KnownPlatforms)
		{
			// check this platform to see if it doesn't match the one we are syncing
			if(!PlatformTag->Name->Equals(PlatformStr, StringComparison::OrdinalIgnoreCase) && (mType != PlatformType::Xbox360 || !PlatformTag->Name->Equals(L"Xenon", StringComparison::OrdinalIgnoreCase)))
			{
				NotPlatforms->Add(PlatformTag->Name);

				// also use the alternate if there is one
				if(PlatformTag->Alt != nullptr && PlatformTag->Alt->Length > 0)
				{
					NotPlatforms->Add(PlatformTag->Alt);
				}
			}
		}

		// add all languages not this one
		for each(Tag ^LangTag in mSharedSettings->KnownLanguages)
		{
			// check this platform to see if it doesn't matches the one we are syncing
			if(String::Compare(LangTag->Name, Language, StringComparison::OrdinalIgnoreCase) != 0)
			{
				NotLanguages->Add(LangTag->Name);
			}
		}

		// process all the file groups
		if(mSharedSettings != nullptr && mSharedSettings->FileGroups != nullptr)
		{
			ProcessFileGroups(TOC, mSharedSettings->FileGroups, BuildSettings, SyncTags, NotPlatforms, NotLanguages, Language);
		}

		if(BuildSettings->GameOptions != nullptr && BuildSettings->GameOptions->FileGroups != nullptr)
		{
			ProcessFileGroups(TOC, BuildSettings->GameOptions->FileGroups, BuildSettings, SyncTags, NotPlatforms, NotLanguages, Language);
		}

		// *ALWAYS* include TOC
		String ^DestinationFolder = String::Format(L"{0}\\", BuildSettings->GameName);
		String ^SourceFolder = Path::Combine( Path::GetFullPath( ".." ), DestinationFolder );

		try
		{
			if(TOCName->Length > 0)
			{
				String ^TOCPath = Path::Combine( SourceFolder, TOCName );
				TocAddFile(TOC, BuildSettings, gcnew FileInfo( TOCPath ), true, true);
			}
		}
		catch(Exception ^Ex) //Note: an ArgumentException will be thrown for an improperly formatted path name
		{
			String ^Error = Ex->ToString();
			System::Diagnostics::Debug::WriteLine(Error);
			Console::WriteLine(Error);
		}

		float Elapsed = (Environment::TickCount - CurTime) / 1000.0f;
		BuildSettings->WriteLine(Color::Black, L"Generating {0} took {1} seconds.", TOCName, Elapsed.ToString());

		return TOC;
	}

	void Platform::ProcessFileGroups(List<TOCInfo^> ^TOC, array<FileGroup^> ^FileGroups, TOCSettings ^BuildSettings, array<Tag^> ^SyncTags, List<String^> ^NotPlatforms, List<String^> ^NotLanguages, String ^Language)
	{
		for each(FileGroup ^Group in FileGroups)
		{
			// if we only want files for ship, filter on the bIsForShip flag on the group
			if(!BuildSettings->IsForShip || Group->bIsForTOC || Group->bIsForSync)
			{
				if (Group->Platform == nullptr ||
					Group->Platform->Equals(L"*") ||
					Group->Platform->Equals(mType.ToString(), StringComparison::OrdinalIgnoreCase) ||
					(mType == PlatformType::Xbox360 && Group->Platform->Equals(L"Xenon", StringComparison::OrdinalIgnoreCase)) ||
					(mType != PlatformType::PC && Group->Platform->Equals(L"Console", StringComparison::OrdinalIgnoreCase)))
				{
					// is this group used for this sync based on tags?
					bool bTagsMatch = false;

					// if the SyncTags array is null, then we will sync all possible tags
					if(SyncTags == nullptr)
					{
						bTagsMatch = true;
					}
					else
					{
						// process tags
						for each(Tag ^CurTag in SyncTags)
						{
							// case-insensitive comparison of group's tag with the list of tags to sync
							if(String::Compare(Group->Tag, CurTag->Name, true) == 0)
							{
								bTagsMatch = true;
							}
						}
					}

					// at this point, property filtering passed, platform matched, and one or more tags matched,
					// so add it to the TOC
					if(bTagsMatch)
					{
						for each (FileSet ^CurSet in Group->Files)
						{
							String ^Path = String::Empty;
							String ^Wildcard = String::Empty;

							GetPathAndPattern( CurSet->Path, Path, Wildcard );

							// add the files to the TOC
#if _DEBUG
							BuildSettings->WriteLine( Color::Purple, L"Including: " + Path + L" " + Wildcard );
#endif
							TocIncludeFiles(TOC, BuildSettings, Path, Wildcard, CurSet, Group, NotPlatforms, NotLanguages, Language);
						}
					}
				}
			}
		}
	}

	void Platform::TocAddFile(List<TOCInfo^> ^TOC, TOCSettings ^BuildSettings, FileInfo ^SrcFile, bool bIsForSync, bool bIsForTOC)
	{
		// Full source and destination filenames.
		String^ SourceFile = SrcFile->FullName;
		String^ CurrentDir = Path::GetFullPath( ".\\.." );
		String^ TargetFile = ".." + SourceFile->Replace( CurrentDir, "" );

		// Check for duplicates - this is not efficient =(
		for each( TOCInfo^ TOCEntry in TOC )
		{
			if( !String::Compare( TOCEntry->FileName, TargetFile, true ) )
			{
				BuildSettings->WriteLine( Color::Orange, L"WARNING: Attempted to add duplicate file to TOC '" + TargetFile + L"'" );
				return;
			}
		}

		// by default, this file wasn't compressed fully
		int UncompressedSize = 0;

		String ^ManifestFilename = String::Concat(SourceFile, MANIFEST_EXTENSION);

		// look to see if there is an .uncompressed_size file 
		if(File::Exists(ManifestFilename))
		{
			// if so, open it and read the size out of it
			StreamReader ^Reader = File::OpenText(ManifestFilename);
			String ^Line = Reader->ReadLine();
			UncompressedSize = int::Parse(Line);
			Reader->Close();
			delete Reader;
		}

		int StartSector = TocFindStartSector(SrcFile->Directory->ToString(), SrcFile->Name, BuildSettings);

		if(SrcFile->Exists)
		{
			// add the file size, and optional uncompressed size
			TOC->Add(gcnew TOCInfo(TargetFile, L"0", SrcFile->LastWriteTimeUtc, (int)SrcFile->Length, UncompressedSize, StartSector, bIsForSync, bIsForTOC));
		}
		else
		{
			// In case the file is force added and doesn't exist - eg. the TOC file
			TOC->Add(gcnew TOCInfo(TargetFile, L"0", DateTime::UtcNow, 0, UncompressedSize, StartSector, bIsForSync, bIsForTOC));
		}
	}

	int Platform::TocFindStartSector(String ^Directory, String ^Filename, TOCSettings ^BuildSettings)
	{
		VALIDATE_SUPPORT(mConsoleSupport);

		if(BuildSettings->HasDVDLayout)
		{
			unsigned int LBAHi = 0;
			unsigned int LBALo = 0;
			
			pin_ptr<const wchar_t> NativePath = PtrToStringChars(Path::Combine(Directory, Filename));
			// ask the DLL for the start sector of the file
			mConsoleSupport->GetDVDFileStartSector(NativePath, LBAHi, LBALo);

			return (int)LBALo;
		}

		// return 0 if we have no layout
		return 0;
	}

	void Platform::TocIncludeFiles(List<TOCInfo^> ^TOC, TOCSettings ^BuildSettings, String ^SourceFolder, String ^SourcePattern, FileSet ^CurSet, FileGroup ^Group, List<String^> ^NotPlatforms, List<String^> ^NotLanguages, String ^Language)
	{
		// the 360 is the only thing w/ an alt platform
		if(mType == PlatformType::Xbox360)
		{
			// Handle expanding %PLATFORM% into  Platform and AltPlatform
			// @todo: not needed if we remove Alt platform.
			if(SourceFolder->Contains("%PLATFORM%"))
			{
				TocIncludeFiles(TOC, BuildSettings, SourceFolder->Replace("%PLATFORM%", L"Xbox360"), SourcePattern, CurSet, Group, NotPlatforms, NotLanguages, Language);
				TocIncludeFiles(TOC, BuildSettings, SourceFolder->Replace("%PLATFORM%", L"Xenon"), SourcePattern, CurSet, Group, NotPlatforms, NotLanguages, Language);
				return;
			}
			if(SourcePattern->Contains("%PLATFORM%"))
			{
				TocIncludeFiles(TOC, BuildSettings, SourceFolder, SourcePattern->Replace("%PLATFORM%", L"Xbox360"), CurSet, Group, NotPlatforms, NotLanguages, Language);
				TocIncludeFiles(TOC, BuildSettings, SourceFolder, SourcePattern->Replace("%PLATFORM%", L"Xenon"), CurSet, Group, NotPlatforms, NotLanguages, Language);
				return;
			}
		}

		// replace special tags
		FixString(SourceFolder, BuildSettings, Language);
		FixString(SourcePattern, BuildSettings, Language);

		// Convert relative paths to platform specific ones.
		SourceFolder = Path::Combine(Path::GetFullPath( ".." ), SourceFolder);

		// make sure destination folder exists
		try
		{
			// Find files matching pattern.
			DirectoryInfo ^SourceDirectory = gcnew DirectoryInfo(SourceFolder);

			if(!SourceDirectory->Exists)
			{
				return;
			}

			array<FileInfo^> ^Files = SourceDirectory->GetFiles(SourcePattern, CurSet->bIsRecursive ? SearchOption::AllDirectories : SearchOption::TopDirectoryOnly);
			String ^PlatformStr = mType.ToString();
			List<String^> ^ExpandedFilters = nullptr;

			if(CurSet->FileFilters != nullptr)
			{
				ExpandedFilters = gcnew List<String^>();

				// handle replacement of some trickier variables (that don't have a 1 to 1 replacement)
				for each(FileFilter ^Filter in CurSet->FileFilters)
				{
					bool bSwitchPlatform = Filter->Name->Contains( L"%PLATFORM%" );
					bool bSwitchNotPlatform = Filter->Name->Contains( L"%NOTPLATFORM%" );
					bool bSwitchLanguage = Filter->Name->Contains( L"%LANGUAGE%" );
					bool bSwitchNotLanguage = Filter->Name->Contains( L"%NOTLANGUAGE%" );

					if( !bSwitchPlatform && !bSwitchNotPlatform && !bSwitchLanguage && !bSwitchNotLanguage )
					{
						// Raw string filter
						ExpandedFilters->Add( Filter->Name );
					}
					else
					{
						String ^Fmt;

						if( bSwitchNotLanguage && bSwitchNotPlatform )
						{
							Fmt = Filter->Name->Replace( L"%NOTPLATFORM%", L"{0}" )->Replace( L"%NOTLANGUAGE%", L"{1}" );

							for each( String ^CurPlatform in NotPlatforms )
							{
								for each( String ^CurLang in NotLanguages )
								{
									ExpandedFilters->Add( String::Format( Fmt, CurPlatform, CurLang ) );
								}
							}
						}
						else if( bSwitchNotLanguage && bSwitchPlatform )
						{
							Fmt = Filter->Name->Replace( L"%PLATFORM%", L"{0}" )->Replace( L"%NOTLANGUAGE%", L"{1}" );

							for each( String ^CurLang in NotLanguages )
							{
								ExpandedFilters->Add( String::Format( Fmt, mType.ToString(), CurLang ) );
							}
						}
						else if( bSwitchLanguage && bSwitchNotPlatform )
						{
							Fmt = Filter->Name->Replace( L"%NOTPLATFORM%", L"{0}" )->Replace( L"%LANGUAGE%", L"{1}" );

							for each( String ^CurPlatform in NotPlatforms )
							{
								ExpandedFilters->Add( String::Format( Fmt, CurPlatform, Language ) );
							}
						}
						else if( bSwitchLanguage && bSwitchPlatform )
						{
							Fmt = Filter->Name->Replace( L"%PLATFORM%", L"{0}" )->Replace( L"%LANGUAGE%", L"{1}" );

							ExpandedFilters->Add( String::Format( Fmt, mType.ToString(), Language ) );
						}
						else if( bSwitchNotLanguage )
						{
							Fmt = Filter->Name->Replace( L"%NOTLANGUAGE%", L"{0}" );

							for each( String ^CurLang in NotLanguages )
							{
								ExpandedFilters->Add( String::Format( Fmt, CurLang ) );
							}
						}
						else if( bSwitchLanguage )
						{
							Fmt = Filter->Name->Replace( L"%LANGUAGE%", L"{0}" );
							ExpandedFilters->Add( String::Format( Fmt, Language ) );
						}
						else if( bSwitchNotPlatform )
						{
							Fmt = Filter->Name->Replace( L"%NOTPLATFORM%", L"{0}" );

							for each( String ^CurPlatform in NotPlatforms )
							{
								ExpandedFilters->Add( String::Format( Fmt, CurPlatform ) );
							}
						}
						else if( bSwitchPlatform )
						{
							Fmt = Filter->Name->Replace( L"%PLATFORM%", L"{0}" );
							ExpandedFilters->Add( String::Format( Fmt, mType.ToString() ) );
						}
					}
				}
			}

			for each(FileInfo ^SrcFile in Files)
			{
				bool bWasFilteredOut = false;
				int CurFileTime = Environment::TickCount;

				// look to see if this file should not be copied
				if(ExpandedFilters != nullptr)
				{
					for each(String ^Filter in ExpandedFilters)
					{
						if(FilterFileName(SrcFile->Name, Filter))
						{
							bWasFilteredOut = true;
							break;
						}
					}
				}

				// only deal with this file if it wasn't filtered out
				if (!bWasFilteredOut)
				{
					TocAddFile(TOC, BuildSettings, SrcFile, Group->bIsForSync, Group->bIsForTOC);
#if _DEBUG
					BuildSettings->WriteLine( Color::Green, L"    Added:     " + SrcFile->ToString() );
				}
				else
				{
					BuildSettings->WriteLine( Color::Red, L"    Filtered: " + SrcFile->ToString() );
#endif
				}
			}
		}
		// gracefully handle copying from non existent folder.
		catch(System::IO::DirectoryNotFoundException^)
		{
		}
	}

	bool Platform::FilterFileName(String ^FileName, String ^Filter)
	{
		if(!Filter->Contains(L"*"))
		{
			return FileName->Equals(Filter, StringComparison::OrdinalIgnoreCase);
		}

		array<String^> ^FilterSegments = Filter->Split(gcnew array<Char> { L'*' }, StringSplitOptions::None);
		bool bHasVariableBeginning = FilterSegments[0]->Length == 0;
		bool bHasVariableEnd = FilterSegments[FilterSegments->Length - 1]->Length == 0;
		int NumSegments = FilterSegments[FilterSegments->Length - 1]->Length == 0 ? FilterSegments->Length - 1 : FilterSegments->Length;
		int CurSegment = bHasVariableBeginning ? 1 : 0;
		int FileNameIndex = 0;

		for(; FileNameIndex < FileName->Length && CurSegment < NumSegments; ++FileNameIndex)
		{
			if(Char::ToLowerInvariant(FileName[FileNameIndex]) == Char::ToLowerInvariant(FilterSegments[CurSegment][0]) && FileName->IndexOf(FilterSegments[CurSegment], FileNameIndex, StringComparison::OrdinalIgnoreCase) == FileNameIndex)
			{
				// -1 because the loop does a + 1
				FileNameIndex += FilterSegments[CurSegment]->Length - 1;
				++CurSegment;
			}
		}

		// if the filter doesn't have a variable length end (ends with a *) and there are characters remaining the filter fails
		return CurSegment == NumSegments && (bHasVariableEnd || FileNameIndex == FileName->Length);
	}

	void Platform::FixString(String ^%Str, TOCSettings ^BuildSettings, String ^Language)
	{
		if(Str != nullptr)
		{
			Str = Str->Replace("%GAME%", BuildSettings->GameName);
			Str = Str->Replace("%PLATFORM%", mType.ToString());
			Str = Str->Replace("%LANGUAGE%", Language);

			// do any extra var replacement
			if(BuildSettings->GenericVars != nullptr)
			{
				for each (KeyValuePair<String^, String^> Pair in BuildSettings->GenericVars)
				{
					String ^Var = String::Format(L"%{0}%",  Pair.Key->ToUpper());
					Str = Str->Replace(Var, Pair.Value);
				}
			}
		}
	}

	void Platform::TocMerge(List<TOCInfo^> ^CurrentTOC, List<TOCInfo^> ^OldTOC, DateTime TocLastWriteTime)
	{
		for each (TOCInfo ^CurrentEntry in CurrentTOC)
		{
			TOCInfo ^OldEntry = TocFindInfoFromConsoleName(OldTOC, CurrentEntry->FileName);
			
			if(OldEntry != nullptr)
			{
				if(DateTime::Compare(TocLastWriteTime, CurrentEntry->LastWriteTime) < 0)
				{
					continue;
				}

				if(CurrentEntry->Size == OldEntry->Size && CurrentEntry->CompressedSize == OldEntry->CompressedSize)
				{
					CurrentEntry->CRC = OldEntry->CRC;
				}
			}
		}
	}

	List<TOCInfo^>^ Platform::TocRead(String ^TOCPath)
	{
		List<TOCInfo^> ^TOC = gcnew List<TOCInfo^>();

		try
		{
			array<String^> ^Lines = File::ReadAllLines(TOCPath);

			for each(String ^CurLine in Lines)
			{
				if(CurLine->Length == 0)
				{
					continue;
				}

				array<String^> ^Words = CurLine->Split(gcnew array<Char> { L' ' }, StringSplitOptions::RemoveEmptyEntries);

				if(Words->Length < 3 )
				{
					continue;
				}

				int Size = Int32::Parse(Words[0]);
				int CompressedSize = Int32::Parse(Words[1]);
				int StartSector = Int32::Parse(Words[2]);
				String ^ConsoleFilename = Words[3];
				String ^CRC = (Words->Length > 4) ? Words[4] : L"0";

				TOC->Add(gcnew TOCInfo(ConsoleFilename, CRC, DateTime(0), Size, CompressedSize, StartSector, true, true));
			}

			return TOC;
		}
		catch(Exception^)
		{
			return gcnew List<TOCInfo^>();
		}
	}

	void Platform::TocGenerateCRC(List<TOCInfo^> ^TOC, String ^TOCFilename, TOCSettings ^BuildSettings)
	{
		BuildSettings->WriteLine(Color::Green, L"[GENERATING CRC STARTED]");
		DateTime StartTime = DateTime::UtcNow;

		//TODO: Make this multi-threaded
		StringBuilder ^Bldr = gcnew StringBuilder();

		for each(TOCInfo ^Entry in TOC)
		{
			if(!Entry->bIsForTOC || !Entry->CRC->Equals(L"0"))
			{
				continue;
			}

			FileInfo ^FilePtr = gcnew FileInfo(Entry->FileName);
			
			if(FilePtr->Name->Equals(TOCFilename))
			{
				// ignore the TOC file
				Entry->CRC = L"0";
			}
			else
			{
				BuildSettings->WriteLine( Color::Black, Entry->FileName );

				if( FilePtr->Exists )
				{
					Entry->CRC = CreateCRC( FilePtr );
				}
				else
				{
					Entry->CRC = L"0";
				}

				BuildSettings->WriteLine( Color::Black, L"\t... {0}", Entry->CRC );
			}
		}

		TimeSpan Duration = DateTime::UtcNow.Subtract(StartTime);
		BuildSettings->WriteLine(Color::Green, L"Operation took {0}:{1}", Duration.Minutes.ToString(), Duration.Seconds.ToString(L"D2"));
		BuildSettings->WriteLine(Color::Green, L"[GENERATING CRC FINISHED]");

		//LogTimeTaken( "CookerSyncCRCCreation", Duration );
	}

	/**
	 * Writes the table of contents to disk.
	 * 
	 * @param	TOC			Table of contents
	 * @param	TOCPath		Destination path for table of contents
	 */
	void Platform::TocWrite(List<TOCInfo^> ^TOC, String ^TOCPath)
	{
		try
		{
			// Make sure the file is not read only
			FileInfo ^Info = gcnew FileInfo(TOCPath);
			if(Info->Exists)
			{
				Info->Attributes = FileAttributes::Normal;
			}

			// delete it first because sometimes it appends, not overwrites, unlike the false says in new StreamWriter!!
			File::Delete(TOCPath);

			// Write out each element of the TOC file
			StreamWriter ^Writer = gcnew StreamWriter(TOCPath, false);
			for each(TOCInfo ^Entry in TOC)
			{
				// skip files that don't need to be in the TOC file
				if(!Entry->bIsForTOC)
				{
					continue;
				}

				if (Entry->CRC != nullptr)
				{
					Writer->WriteLine(L"{0} {1} {2} {3} {4}", Entry->Size, Entry->CompressedSize, Entry->StartSector, Entry->FileName, Entry->CRC);
				}
				else
				{
					Writer->WriteLine(L"{0} {1} {2} {3} 0", Entry->Size, Entry->CompressedSize, Entry->StartSector, Entry->FileName);
				}
			}

			Writer->Close();
			delete Writer;
		}
		catch(Exception ^e)
		{
			String ^ErrMsg = e->ToString();
			Console::WriteLine(ErrMsg);
			System::Diagnostics::Debug::WriteLine(ErrMsg);
		}
	}

	TOCInfo^ Platform::TocFindInfoFromConsoleName(List<TOCInfo^> ^TOC, String ^ConsoleName)
	{
		for each(TOCInfo ^Entry in TOC)
		{
			if(Entry->FileName->Equals(ConsoleName,StringComparison::OrdinalIgnoreCase))
			{
				return Entry;
			}
		}

		return nullptr;
	}

	bool Platform::TargetSync(TOCSettings ^BuildSettings, String ^TagSetName, bool bReboot, bool bNonInteractive)
	{
		bool bSuccess = true;

		List<List<TOCInfo^>^> ^TOCList = gcnew List<List<TOCInfo^>^>();

		for each(String ^CurLang in BuildSettings->Languages)
		{
			TOCList->Add(GenerateTOC(TagSetName, BuildSettings, CurLang));
		}

		if(BuildSettings->TargetsToSync->Count > 0)
		{
			if(!TargetSync(BuildSettings, TOCList, bReboot, bNonInteractive))
			{
				bSuccess = false;
			}
		}

		if(BuildSettings->DestinationPaths->Count > 0)
		{
			if(!PcSync(TOCList, BuildSettings))
			{
				bSuccess = false;
			}
		}

		return bSuccess;
	}

	/**
	 * Syncs the console with the PC.
	 * 
	 * @return true if succcessful
	 */
	bool Platform::TargetSync(TOCSettings ^BuildSettings, List<List<TOCInfo^>^> ^TOCList, bool bReboot, bool bNonInteractive)
	{
		if(!this->NeedsToSync)
		{
			return true;
		}

		bool bSuccess = true;

		List<PlatformTarget^> ^Targets = gcnew List<PlatformTarget^>();
		List<PlatformTarget^> ^TargetsToDisconnect = gcnew List<PlatformTarget^>();
		Dictionary<String^, PlatformTarget^> ^TargetCache = gcnew Dictionary<String^, PlatformTarget^>();
		Dictionary<unsigned int, PlatformTarget^> ^TargetIPCache = gcnew Dictionary<unsigned int, PlatformTarget^>();

		for each(PlatformTarget ^CurTarget in mTargets->Values)
		{
			TargetCache[CurTarget->Name->ToLowerInvariant()] = CurTarget;

			// Add title IP address
			unsigned int Addr = BitConverter::ToUInt32(CurTarget->IPAddress->GetAddressBytes(), 0);

			if(Addr != 0)
			{
				TargetIPCache[Addr] = CurTarget;
			}

			// Add debug channel IP address
			Addr = BitConverter::ToUInt32(CurTarget->DebugIPAddress->GetAddressBytes(), 0);

			if(Addr != 0)
			{
				TargetIPCache[Addr] = CurTarget;
			}
		}

		WORD Subsystem = GetApplicationSubsystem();
		IWin32Window ^ParentWindow = nullptr;

		if(Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI)
		{
			// There's high probability if the current application is a console app then it's CookerSync
			// lets see if the parent application is UnrealFrontend because if it is then we'll want to use its
			// main window handle instead of the console application window handle as that will be hidden
			HANDLE SnapshotHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

			if(SnapshotHandle != NULL)
			{
				PROCESSENTRY32 ProcInfo;
				ProcInfo.dwSize = sizeof(PROCESSENTRY32);

				try
				{
					if(Process32First(SnapshotHandle, &ProcInfo))
					{
						Process ^CurProcess = Process::GetCurrentProcess();
						Process ^ParentProcess = nullptr;

						do 
						{
							if(ProcInfo.th32ProcessID == CurProcess->Id)
							{
								ParentProcess = Process::GetProcessById(ProcInfo.th32ParentProcessID);
								
								if(ParentProcess->MainModule->ModuleName->Equals(L"UnrealFrontend.exe", StringComparison::OrdinalIgnoreCase))
								{
									ParentWindow = gcnew HwndWrapper(ParentProcess->MainWindowHandle);
								}

								delete ParentProcess;

								break;
							}
						} while(Process32Next(SnapshotHandle, &ProcInfo));

						delete CurProcess;
					}
				}
				catch(Exception ^ex)
				{
					System::Diagnostics::Debug::WriteLine(ex->ToString());
				}

				CloseHandle(SnapshotHandle);
			}

			// Either the parent application isn't UnrealFrontend or we failed to get a valid parent window from UnrealFrontend
			// default to the console window handle
			if(ParentWindow == nullptr)
			{
				ParentWindow = gcnew HwndWrapper(GetConsoleWindow());
			}
		}
		else if(Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI)
		{
			try
			{
				Process ^CurProcess = Process::GetCurrentProcess();
				ParentWindow = gcnew HwndWrapper(CurProcess->MainWindowHandle);
				delete CurProcess;
			}
			catch(Exception ^ex)
			{
				System::Diagnostics::Debug::WriteLine(ex->ToString());
			}
		}

		ConnectToConsoleRetryForm ^RetryForm = gcnew ConnectToConsoleRetryForm();
		RetryForm->Text = L"CookerSync: Target Connection Error";

		for each(String ^CurTargetName in BuildSettings->TargetsToSync)
		{
			PlatformTarget ^CurTarget = nullptr;
			IPAddress ^TargetAddr = nullptr;

			if(IPAddress::TryParse(CurTargetName, TargetAddr))
			{
				TargetIPCache->TryGetValue(BitConverter::ToUInt32(TargetAddr->GetAddressBytes(), 0), CurTarget);
			}

			if(CurTarget == nullptr)
			{
				TargetCache->TryGetValue(CurTargetName->ToLowerInvariant(), CurTarget);
			}

			if(CurTarget != nullptr)
			{
				bool bConnected = true;

				if(!CurTarget->IsConnected)
				{
					RetryForm->Message = String::Format(L"Could not connect to {0}!", PlatformTarget::ResolveIPFromHostFile(CurTargetName));

					do 
					{
						bConnected = CurTarget->Connect();

						if(!bConnected)
						{

							if(bNonInteractive)
							{
								BuildSettings->WriteLine(Color::Orange, L"Target \'{0}\' skipped!", CurTargetName);
								bSuccess = false;
								break;
							}
							else
							{
								System::Media::SystemSounds::Exclamation->Play();

								if(RetryForm->ShowDialog(ParentWindow) == System::Windows::Forms::DialogResult::Cancel)
								{
									BuildSettings->WriteLine(Color::Orange, L"Target \'{0}\' skipped!", CurTargetName);
									bSuccess = false;
									break;
								}
							}
						}
						else
						{
							TargetsToDisconnect->Add(CurTarget);
						}
					} while(!bConnected);
				}

				if(bConnected)
				{
					Targets->Add(CurTarget);
					CurTarget->InitializeSyncing();
				}
			}
			else
			{
				BuildSettings->WriteLine(Color::Red, L"Target \'{0}\' either does not exist or is currently unavailable!", CurTargetName);
				bSuccess = false;
			}
		}

		delete RetryForm;

		if(Targets->Count == 0)
		{
			BuildSettings->WriteLine(Color::Red, L"List of targets to sync does not contain any valid targets. Ending sync operation.");
			return false;
		}

		DateTime StartTime = DateTime::UtcNow;

		try
		{
			// Should only be Xenon that gets this far
			if(bReboot)
			{
				BuildSettings->WriteLine(Color::Green, L"\r\n[REBOOTING TARGETS]");

				for each(PlatformTarget ^CurTarget in Targets)
				{
					// Grab name before reboot because you can't retrieve it in certain reboot stages
					String ^TargetName = CurTarget->Name;

					if(!CurTarget->Reboot())
					{
						BuildSettings->WriteLine(Color::Red, L"Target \'{0}\' failed to reboot!", TargetName);
					}
					else
					{
						BuildSettings->WriteLine(Color::Black, "Target \'{0}\' has been rebooted.", TargetName);
					}
				}

				MSG Msg;
				bool bAllTargetsRebooted = true;
				int Ticks = Environment::TickCount;
				const int REBOOT_TIMEOUT = 60000;

				// need to wait for xenon's to finish rebooting before we start copying files
				// otherwise bad things happen
				do
				{
					bAllTargetsRebooted = true;

					// pump COM and UI messages
					if(GetMessage(&Msg, NULL, 0, 0) > 0)
					{
						TranslateMessage(&Msg);
						DispatchMessage(&Msg);
					}

					for(int i = 0; i < Targets->Count; ++i)
					{
						if(Targets[i]->State == TargetState::Rebooting)
						{
							bAllTargetsRebooted = false;
							break;
						}
					}
				} while(!bAllTargetsRebooted && Environment::TickCount - Ticks < REBOOT_TIMEOUT);

				if(Environment::TickCount - Ticks >= REBOOT_TIMEOUT)
				{
					BuildSettings->WriteLine(Color::Orange, L"Warning: Waiting for all targets to finish rebooting has timed out. This is probably the result of corrupted state for one or more of the targets.");
				}
			}

			// start copying files
			BuildSettings->WriteLine(Color::Green, L"\r\n[SYNCING DATA STARTED]");

			if(!BuildSettings->NoSync)
			{
				for each(PlatformTarget ^CurTarget in Targets)
				{
					// create the base directory
					if(!CurTarget->MakeDirectory(BuildSettings->TargetBaseDirectory))
					{
						BuildSettings->WriteLine(Color::Orange, L"Failed to create directory \'{0}\'", Path::GetDirectoryName(BuildSettings->TargetBaseDirectory));
					}
				}
			}

			VALIDATE_SUPPORT(mConsoleSupport);

			std::vector<TARGETHANDLE> NativeTargets;
			std::vector<const wchar_t*> NativeSrcFiles;
			std::vector<const wchar_t*> NativeDestFiles;
			std::vector<const wchar_t*> NativeDirectories;
			Dictionary<String^, String^> ^DirectoriesToMake = gcnew Dictionary<String^, String^>();
			Dictionary<String^, String^> ^FilesToCopy = gcnew Dictionary<String^, String^>();
			pin_ptr<const wchar_t> PinTemp;
			wchar_t *Temp = NULL;

			for each(PlatformTarget ^CurTarget in Targets)
			{
				NativeTargets.push_back(CurTarget->Handle);
			}

			for each(List<TOCInfo^> ^TOC in TOCList)
			{
				for each(TOCInfo ^Entry in TOC)
				{
					if (Entry->bIsForSync)
					{
						String^ FullFileName = Entry->FileName;
						if( FullFileName->StartsWith( "..\\" ) )
						{
							FullFileName = FullFileName->Substring( 3 );
						}
				
						String^ DestPath = Path::Combine(BuildSettings->TargetBaseDirectory, FullFileName);
						String^ DestDir = Path::GetDirectoryName(DestPath);

						if(!FilesToCopy->ContainsKey(DestPath))
						{
							PinTemp = PtrToStringChars(Entry->FileName);
							Temp = new wchar_t[Entry->FileName->Length + 1];
							wcscpy_s(Temp, Entry->FileName->Length + 1, PinTemp);
							NativeSrcFiles.push_back(Temp);

							PinTemp = PtrToStringChars(DestPath);
							Temp = new wchar_t[DestPath->Length + 1];
							wcscpy_s(Temp, DestPath->Length + 1, PinTemp);
							NativeDestFiles.push_back(Temp);

							FilesToCopy[DestPath] = DestPath;
						}

						if(!DirectoriesToMake->ContainsKey(DestDir))
						{
							PinTemp = PtrToStringChars(DestDir);
							Temp = new wchar_t[DestDir->Length + 1];
							wcscpy_s(Temp, DestDir->Length + 1, PinTemp);
							NativeDirectories.push_back(Temp);

							DirectoriesToMake[DestDir] = DestDir;
						}
					}
				}
			}

			try
			{
				// NOTE: We need to hold on to this for the duration of SyncFiles() so that the native to managed thunk (__stdcall to __clrcall) doesn't get collected
				WriteLineNativeDelegate ^TTYDelegate = gcnew WriteLineNativeDelegate(BuildSettings, &TOCSettings::WriteLine);

				ColoredTTYEventCallbackPtr TTYCallback = (ColoredTTYEventCallbackPtr)Marshal::GetFunctionPointerForDelegate(TTYDelegate).ToPointer();

				bool bSyncResult = mConsoleSupport->SyncFiles(&NativeTargets[0], (int)NativeTargets.size(), &NativeSrcFiles[0], &NativeDestFiles[0], (int)NativeSrcFiles.size(), &NativeDirectories[0], (int)NativeDirectories.size(), TTYCallback);

				bSuccess = bSuccess && bSyncResult;
			}
			finally
			{
				for(size_t i = 0; i < NativeSrcFiles.size(); ++i)
				{
					delete [] NativeSrcFiles[i];
					delete [] NativeDestFiles[i];
				}

				for(size_t i = 0; i < NativeDirectories.size(); ++i)
				{
					delete [] NativeDirectories[i];
				}
			}
		}
		finally
		{
			for each(PlatformTarget ^CurTarget in TargetsToDisconnect)
			{
				CurTarget->Disconnect();
			}
		}

		TimeSpan Duration = DateTime::UtcNow.Subtract(StartTime);
		BuildSettings->WriteLine(Color::Black, L"\r\nOperation took {0}:{1}", Duration.Minutes.ToString(), Duration.Seconds.ToString(L"D2"));
		BuildSettings->WriteLine(Color::Green, L"[SYNCING WITH TARGETS FINISHED]");

		//LogTimeTaken( "CookerSyncCopy", Duration );

		return bSuccess;
	}

	/**
	 * Initializes the target paths.
	 */
	bool Platform::PcInit(TOCSettings ^BuildSettings, List<String^> ^TargetPaths)
	{
		// used to indicate potential existance
		bool bWouldExist = false;
		TargetPaths->Clear();

		// Get each console that was checked
		for each(String ^Path in BuildSettings->DestinationPaths)
		{
			DirectoryInfo ^Info = gcnew DirectoryInfo(Path);
			if (!Info->Exists)
			{
				if(BuildSettings->NoSync)
				{
					BuildSettings->WriteLine(Color::Black, L"Would create directory: \'{0}\'", Path);
					bWouldExist = true;
				}
				else
				{
					BuildSettings->WriteLine(Color::Black, L"Creating directory: \'{0}\'", Path);

					try
					{
						Info->Create();
					}
					catch(Exception ^e)
					{
						BuildSettings->WriteLine(Color::Red, String::Format(L"Error: {0}", e->Message));
					}

					Info->Refresh();
				}
			}

			if(Info->Exists || bWouldExist)
			{
				String ^TargetPath = Path;
				if(!TargetPath->EndsWith(L"\\"))
				{
					TargetPath = String::Concat(TargetPath, L"\\");
				}

				TargetPaths->Add(TargetPath);
			}
		}

		// this was successful if we found at least one target we can use
		if(TargetPaths->Count > 0)
		{
			return true;
		}

		BuildSettings->WriteLine(Color::Red, "Error: Failed to find any valid target paths.");
		return false;
	}

	/**
	 * Syncs the build to a PC directory (for any platform)
	 * 
	 * @return true if succcessful
	 */
	bool Platform::PcSync(List<List<TOCInfo^>^> ^TOCList, TOCSettings ^BuildSettings)
	{
		List<String^> ^TargetPaths = gcnew List<String^>();

		// Initialize the target paths.
		if (PcInit(BuildSettings, TargetPaths) == false)
		{
			return false;
		}

		BuildSettings->WriteLine(Color::Green, L"[SYNCING WITH FOLDERS STARTED]");
		DateTime StartTime = DateTime::UtcNow;

		List<String^> ^CRCMismatchedFiles = gcnew List<String^>();

		if(BuildSettings->SyncToHost)
		{
			List<SyncFileSocket^> ^SocketList = gcnew List<SyncFileSocket^>();

			for each(String ^CurHost in TargetPaths)
			{
				if(CurHost->StartsWith(L"\\\\"))
				{
					array<String^> ^Tokens = CurHost->Split(L'\\');
					String ^ServerName = Tokens[2];

					SyncFileSocket ^Sock = gcnew SyncFileSocket();
					
					if(Sock->Connect(ServerName, BuildSettings))
					{
						SocketList->Add(Sock);
						BuildSettings->WriteLine(Color::Green, L"Connected to remote host \'{0}\'", CurHost);

						Sock->SendString(L"ROOT");
						Sock->SendString(String::Format(L"{0}\\{1}", CurHost, BuildSettings->TargetBaseDirectory));
					}
					else
					{
						BuildSettings->WriteLine(Color::Orange, L"Could not connect to remote host \'{0}\'", CurHost);
					}
				}

				for each(List<TOCInfo^> ^TOC in TOCList)
				{
					SyncFileSocket::SyncFiles(TOC, SocketList->ToArray());
				}

				for each(SyncFileSocket ^Sock in SocketList)
				{
					Sock->SendString(L"GETBADCRCS");
					int NumBadCRC = 0;

					if(Sock->ReceiveInt32(NumBadCRC))
					{
						for(int i = 0; i < NumBadCRC; ++i)
						{
							String ^BadFile;
							if(Sock->ReceiveString(BadFile))
							{
								CRCMismatchedFiles->Add(BadFile);
							}
						}
					}
				}
			}
		}
		else
		{
			Dictionary<String^, String^> ^FilesCopied = gcnew Dictionary<String^, String^>();
			//WORD AppSubsystem = GetApplicationSubsystem();
			//MSG Msg;

			// Do the action on each path
			for each(String^ CurPath in TargetPaths)
			{
				for each(List<TOCInfo^> ^TOC in TOCList)
				{
					// Copy each file from the table of contents
					for each (TOCInfo ^Entry in TOC)
					{
						String^ FullFileName = Entry->FileName;
						if( FullFileName->StartsWith( "..\\" ) )
						{
							FullFileName = FullFileName->Substring( 3 );
						}
						String^ DestFileName = Path::Combine( CurPath, Path::Combine( BuildSettings->TargetBaseDirectory, FullFileName ) );

						if(!FilesCopied->ContainsKey(DestFileName))
						{
							FilesCopied[DestFileName] = DestFileName;

							if(!PcCopyFile(BuildSettings, CRCMismatchedFiles, Entry->FileName, DestFileName, Entry->CRC))
							{
								return false;
							}
						}

						//NOTE: This would allow input to be displayed while copying files or allow the window to be closed.
						// the problem is that the form is destroyed on a WM_QUIT before this handler receives the WM_QUIT so
						// when it returns false and propagates back up the calls tack the form tries to access invalid state
						// since it's already been destroyed. wtf.

						// pump COM and UI messages so that the UI updates
						//if(AppSubsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI)
						//{
						//	for(int i = 0; i < 20; ++i)
						//	{
						//		GetMessage(&Msg, NULL, 0, 0);

						//		if(Msg.message == WM_QUIT || Msg.message == WM_CLOSE)
						//		{
						//			// add the message back to the queue
						//			//PostMessage(Msg.hwnd, Msg.message, Msg.wParam, Msg.lParam);
						//			return false;
						//		}

						//		TranslateMessage(&Msg);
						//		DispatchMessage(&Msg);
						//	}
						//}
					}
				}
			}
		}

		if(CRCMismatchedFiles->Count > 0)
		{
			BuildSettings->WriteLine(Color::Red, L"[CRC MISMATCHED FILES]");

			for each(String ^File in CRCMismatchedFiles)
			{
				BuildSettings->WriteLine(Color::Red, File);
			}
		}

		TimeSpan Duration = DateTime::UtcNow.Subtract(StartTime);
		BuildSettings->WriteLine(Color::Green, L"Operation took {0}: {1}", Duration.Minutes.ToString(), Duration.Seconds.ToString(L"D2"));
		BuildSettings->WriteLine(Color::Green, L"[SYNCING WITH FOLDERS FINISHED]");

		//LogTimeTaken( "CookerSyncCopy", Duration );

		return CRCMismatchedFiles->Count == 0;
	}

	bool Platform::PcCopyFile(TOCSettings ^BuildSettings, List<String^> ^CRCMismatchedFiles, String ^SourcePath, String ^DestPath, String ^SrcCRC)
	{
		bool bCopySucceeded = true;
		const int MaxCopyRetries = 10;

		FileInfo ^SrcFile = gcnew FileInfo(SourcePath);
		FileInfo ^DstFile = gcnew FileInfo(DestPath);

		if (PcRequireCopy(BuildSettings, SrcFile, DstFile))
		{
			if(!BuildSettings->NoSync)
			{
				BuildSettings->WriteLine(Color::Black, L"Copying {0} to {1}", SourcePath, DestPath);

				int CopyRetries = 0;
				bool bWroteToDestination = false;

				while(!bWroteToDestination && CopyRetries < MaxCopyRetries)
				{
					try
					{
						if(!DstFile->Directory->Exists)
						{
							DstFile->Directory->Create();
							DstFile->Directory->Refresh();
						}

						if(DstFile->Exists)
						{
							DstFile->Attributes = FileAttributes::Normal;
							DstFile->Refresh();
						}

						SrcFile->CopyTo(DestPath, true);

						bWroteToDestination = true;
					}
					catch(Exception ^E)
					{
						bCopySucceeded = false;
						CopyRetries++;

						String ^ErrorDetail = L"Unspecified";
						
						try
						{
							if(E->Message != nullptr)
							{
								ErrorDetail = E->Message;
							}
						}
						catch(Exception^)
						{
						}

						BuildSettings->WriteLine(Color::Orange, L"=> NETWORK WRITE ERROR: {0}", ErrorDetail);

						// Pause for a long time before retrying
						System::Threading::Thread::Sleep(60 * 1000);
					}
				}

				if(!bWroteToDestination)
				{
					BuildSettings->WriteLine(Color::Red, L"==> NETWORK WRITE ERROR: Failed to copy \'{0}\' after {1} retries 60 seconds apart.", SrcFile->Name, CopyRetries.ToString());
				}

				System::Threading::Thread::Sleep(BuildSettings->SleepDelay);
			}
			else
			{
				if(!BuildSettings->VerifyCopy)
				{
					BuildSettings->WriteLine(Color::Black, L"Would copy {0} to {1}", SourcePath, DestPath);
				}
				else
				{
					BuildSettings->WriteLine(Color::Black, L"Verifying {0}", DestPath);
				}
			}

			if(BuildSettings->VerifyCopy && bCopySucceeded)
			{
				DstFile->Refresh();
				bool bFoundFile = false;
				int ReadRetries = 0;

				while(!bFoundFile && ReadRetries < MaxCopyRetries)
				{
					if(DstFile->Exists)
					{
						if(!SrcCRC->Equals(L"0"))
						{
							String ^DstCRC = CreateCRC(DstFile);

							if(!SrcCRC->Equals(DstCRC))
							{
								BuildSettings->WriteLine(Color::Red, L"==> ERROR: CRC Mismatch");
								CRCMismatchedFiles->Add(String::Format(L"Error: CRC Mismatch for \'{0}\'", DestPath));
							}
						}
						else
						{
							BuildSettings->WriteLine(Color::Red, L"Note: No CRC available for \'{0}\'", DestPath);
						}

						bFoundFile = true;
					}
					else
					{
						ReadRetries++;

						if(BuildSettings->NoSync)
						{
							BuildSettings->WriteLine(Color::Orange, L"=> NETWORK READ ERROR\r\n");
						}
					}
				}

				if (!bFoundFile)
				{
					CRCMismatchedFiles->Add(String::Format(L"Error: Missing file \'{0}\'", DestPath));
					BuildSettings->WriteLine(Color::Red, L"==> NETWORK READ ERROR: Failed to read \'{0}\' after {1} retries 60 seconds apart.", DstFile->Name, ReadRetries.ToString());
				}

				System::Threading::Thread::Sleep(BuildSettings->SleepDelay);
			}
		}

		return bCopySucceeded;
	}

	bool Platform::PcRequireCopy(TOCSettings ^BuildSettings, FileInfo ^SrcFile, FileInfo ^DstFile)
	{
		if(BuildSettings->SyncToHost)
		{
			return true;
		}

		if(!SrcFile->Exists)
		{
			return false;
		}

		if(BuildSettings->Force)
		{
			return true;
		}

		if(!DstFile->Exists)
		{
			return true;
		}

		// compare the lengths
		if(DstFile->Length != SrcFile->Length)
		{
			return true;
		}

		// compare the time stamps
		if(DateTime::Compare(DstFile->LastWriteTimeUtc, SrcFile->LastWriteTimeUtc) < 0)
		{
			return true;
		}

		return false;
	}

	String^ Platform::CreateCRC( FileInfo ^SrcFile )
	{
		MD5CryptoServiceProvider^ Hasher = gcnew MD5CryptoServiceProvider();

		FileStream ^Stream = SrcFile->Open( FileMode::Open, FileAccess::Read, FileShare::Read );

		array<Byte> ^HashData = Hasher->ComputeHash( Stream );

		Stream->Close();
		delete Stream;

		StringBuilder ^HashCodeBuilder = gcnew StringBuilder( HashData->Length * 2 );

		for( int Index = 0; Index < HashData->Length; ++Index )
		{
			HashCodeBuilder->Append( HashData[Index].ToString( L"x2" ) );
		}

		return( HashCodeBuilder->ToString() );
	}

	bool Platform::TargetSync(TOCSettings ^BuildSettings, bool bReboot, bool bNonInteractive)
	{
		return TargetSync(BuildSettings, (String^)nullptr, bReboot, bNonInteractive);
	}

	WORD Platform::GetApplicationSubsystem()
	{
		pin_ptr<const wchar_t> AsmPath = PtrToStringChars(Assembly::GetEntryAssembly()->Location);

		FILE *FilePtr = _wfopen(AsmPath, L"rb");

		if(!FilePtr)
		{
			return 0;
		}

		IMAGE_DOS_HEADER DosHeader;

		fread(&DosHeader, sizeof(DosHeader), 1, FilePtr);

		if(DosHeader.e_magic != IMAGE_DOS_SIGNATURE)
		{
			return 0;
		}

		fseek(FilePtr, DosHeader.e_lfanew, SEEK_SET);

		ULONG NtSignature;
		fread(&NtSignature, sizeof(NtSignature), 1, FilePtr);

		if(NtSignature != IMAGE_NT_SIGNATURE)
		{
			return 0;
		}

		IMAGE_FILE_HEADER FileHeader;
		fread(&FileHeader, sizeof(FileHeader), 1, FilePtr);

		IMAGE_OPTIONAL_HEADER OptionalHeader;
		fread(&OptionalHeader, sizeof(OptionalHeader), 1, FilePtr);

		fclose(FilePtr);

		return OptionalHeader.Subsystem;
	}

	int Platform::EnumerateAvailableTargets()
	{
		VALIDATE_SUPPORT(mConsoleSupport);

		int NumTargets = mConsoleSupport->EnumerateAvailableTargets();

		TARGETHANDLE *TargList = new TARGETHANDLE[NumTargets];

		try
		{
			mConsoleSupport->GetTargets(TargList, &NumTargets);

			for(int i = 0; i < NumTargets; ++i)
			{
				String ^TMName = gcnew String(mConsoleSupport->GetTargetManagerNameForConsole(TargList[i]));

				if(TMName->Length > 0 && !mTargets->ContainsKey(TMName))
				{
					mTargets[TMName] = gcnew PlatformTarget(TargetHandle(TargList[i]), this);
				}
			}
		}
		finally
		{
			delete [] TargList;
		}

		return NumTargets;
	}
}