/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"

#include "FUtils/FUTestBed.h"
#include "FUtils/FUFileManager.h"

TESTSUITE_START(FUFileManager)

TESTSUITE_TEST(Substrings)
	const size_t testFilenameCount = 6;
	const fchar* testFilenames[testFilenameCount] = {
		FC("C:\\TestFolder\\TestFile.dae"),
		FC("/UnixFolder/AbsoluteFile.dae"),
		FC("\\\\WindowsUNCPath/NetworkFolder/SomeFile.dae"),
		FC("file:///strangeURI"),
		FC("/UnixFolder/UnixFolder.AnnoyingExtension/SomeFileWithoutExtension"),
		FC("file://www.someweb.com/index")
	};

	for (size_t i = 0; i < testFilenameCount; ++i)
	{
		// Check the ExtractNetworkHostname function
		fstring filename = testFilenames[i];
		fstring hostname = FUFileManager::ExtractNetworkHostname(filename);

		static const fchar* checkFilenameReferences[testFilenameCount] = {
			FC("C:\\TestFolder\\TestFile.dae"),
			FC("/UnixFolder/AbsoluteFile.dae"),
			FC("/NetworkFolder/SomeFile.dae"),
			FC("file:///strangeURI"),
			FC("/UnixFolder/UnixFolder.AnnoyingExtension/SomeFileWithoutExtension"),
			FC("file://www.someweb.com/index")
		};

		static const fchar* checkHostnames[testFilenameCount] =
			{ FC(""), FC(""), FC("WindowsUNCPath"), FC(""), FC(""), FC("") };
		PassIf(IsEquivalent(filename, checkFilenameReferences[i]));
		PassIf(IsEquivalent(hostname, checkHostnames[i]));

		// Check the GetFileExtension function
		fstring extension = FUFileManager::GetFileExtension(testFilenames[i]);

		static const fchar* checkExtensions[testFilenameCount] =
			{ FC("dae"), FC("dae"), FC("dae"), FC(""), FC(""), FC("") };
		PassIf(IsEquivalent(extension, checkExtensions[i]));

		// Check the StripFileFromPath function
		filename = FUFileManager::StripFileFromPath(testFilenames[i]);
		static const fchar* checkStrippedFile[testFilenameCount] =
			{ FC("C:\\TestFolder\\"), FC("/UnixFolder/"), FC("\\\\WindowsUNCPath/NetworkFolder/"),
			FC("file:///"), FC("/UnixFolder/UnixFolder.AnnoyingExtension/"), FC("file://www.someweb.com/") };
		PassIf(IsEquivalent(filename, checkStrippedFile[i]));
	}


TESTSUITE_TEST(BasicFileManagement)
	FUFileManager manager;

	// Verify that the 'localhost' hostname is dropped.
	fstring f = FC("file://localhost/rootFolder/testFile.dae");
	f = manager.MakeFilePathAbsolute(f);
	PassIf(f == FC("\\rootFolder\\testFile.dae"));

TESTSUITE_TEST(RelativePaths)
	// Verify the relative file path generation for local files
	FUFileManager manager;
	manager.PushRootFile(FC("C:\\AFolder\\AFolder.D\\AFile"));
	fstring f = manager.MakeFilePathRelative(FC("D:\\BPath\\BFile"));
	PassIf(f == FC("D:\\BPath\\BFile"));

	f = manager.MakeFilePathRelative(FC("\\BPath\\BFile"));
	PassIf(f == FC("\\BPath\\BFile"));

	f = manager.MakeFilePathRelative(FC("\\AFolder\\AFolder.D\\BFile"));
	PassIf(f == FC(".\\BFile"));

	f = manager.MakeFilePathRelative(FC("\\AFolder\\BFile"));
	PassIf(f == FC("..\\BFile"));

	f = manager.MakeFilePathRelative(FC("..\\AFolder.D\\BFile"));
	// [glaforte 21-07-2006] might be optimization possible, although the current output is not wrong.
	PassIf(f == FC("..\\AFolder.D\\BFile"));

	f = manager.MakeFilePathRelative(FC("\\\\AFolder\\AFile"));
	PassIf(f == FC("\\\\AFolder\\AFile"));

	// Verify the relative file path generation with a network file as the root
	manager.PopRootFile();
	manager.PushRootFile(FC("\\\\ANetwork\\AFolder\\AFolder.D\\AFile"));
	f = manager.MakeFilePathRelative(FC("C:\\BLocal\\BFolder\\AFile"));
	PassIf(f == FC("C:\\BLocal\\BFolder\\AFile"));

	f = manager.MakeFilePathRelative(FC("\\\\ANetwork\\AFolder\\BFile"));
	PassIf(f == FC("..\\BFile"));
	
	// Verify the relative file path generation when the root has no drive letter
	manager.PushRootFile(FC("\\ARootFolder\\BFile"));
	f = manager.MakeFilePathRelative(FC("A:\\ARootFolder\\CFile"));
	PassIf(f == FC("A:\\ARootFolder\\CFile"));

	f = manager.MakeFilePathRelative(FC("BPureRelative\\CFile"));
	PassIf(f == FC(".\\BPureRelative\\CFile"));

TESTSUITE_TEST(URI_Generation)
	// Verify the URI generation with respect to a local file
	FUFileManager manager;
	manager.PushRootFile(FC("C:\\AFolder\\AFolder.D\\AFile"));
	fstring f = manager.GetFileURL(FC("C:\\AFolder\\AFolder.D\\BFile"), true);
	PassIf(f == FC("./BFile"));
	f = manager.GetFileURL(FC("C:\\AFolder\\AFolder.D\\BFile"), false);
	PassIf(f == FC("file:///C:/AFolder/AFolder.D/BFile"));

	f = manager.GetFileURL(FC("\\\\BNetwork\\BFolder\\BFile"), true);
	PassIf(f == FC("file://BNetwork/BFolder/BFile"));
	f = manager.GetFileURL(FC("\\\\BNetwork\\BFolder\\BFile"), false);
	PassIf(f == FC("file://BNetwork/BFolder/BFile"));

	f = manager.GetFileURL(FC("file:///C:/AFolder/BFolder/BFile"), true);
	PassIf(f == FC("../BFolder/BFile"));
	f = manager.GetFileURL(FC("file:///C:/AFolder/BFolder/BFile"), false);
	PassIf(f == FC("file:///C:/AFolder/BFolder/BFile"));

	// Verify the URI generation with respect to a networked file
	manager.PushRootFile(FC("\\\\BNetwork\\BFolder\\BSubfolder\\BFile"));
	f = manager.GetFileURL(FC("C:\\BFolder\\BSubfolder\\CFile"), true);
	PassIf(f == FC("file:///C:/BFolder/BSubfolder/CFile"));
	f = manager.GetFileURL(FC("C:\\BFolder\\BSubfolder\\CFile"), false);
	PassIf(f == FC("file:///C:/BFolder/BSubfolder/CFile"));

	f = manager.GetFileURL(FC("\\\\BNetwork\\BFolder\\CFile"), true);
	PassIf(f == FC("../CFile"));
	f = manager.GetFileURL(FC("\\\\BNetwork\\BFolder\\CFile"), false);
	PassIf(f == FC("file://BNetwork/BFolder/CFile"));

	f = manager.GetFileURL(FC("file://BNetwork/BFolder/BSubfolder/CFile"), true);
	PassIf(f == FC("./CFile"));
	f = manager.GetFileURL(FC("file://BNetwork/BFolder/BSubfolder/CFile"), false);
	PassIf(f == FC("file://BNetwork/BFolder/BSubfolder/CFile"));

TESTSUITE_TEST(BackwardCompatibility)
	// Verify the handling of the file paths that we used to export.
	FUFileManager manager;
	manager.PushRootFile(FC("C:\\AFolder\\AFolder.D\\AFile"));
	fstring f = manager.GetFilePath(FC("file://C|/AFolder/BFolder/BFile"));
	PassIf(f == FC("C:\\AFolder\\BFolder\\BFile"));

	f = manager.MakeFilePathRelative(FC("file://C|/AFolder/BFolder/BFile"));
	PassIf(f == FC("..\\BFolder\\BFile"));

TESTSUITE_END
