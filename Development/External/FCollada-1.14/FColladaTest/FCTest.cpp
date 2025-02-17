/*
	Copyright (C) 2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include <direct.h>
#include <iostream>

fstring logFilename = FC("FColladaTestLog.txt");
bool isVerbose = true;

void ShowHelp()
{
	std::cerr << "Unknown command line argument(s). " << std::endl;
	std::cerr << "Usage: FColladaTest.exe [-v <is verbose?>] [<filename>]" << std::endl;
	std::cerr << "  -v <is verbose?>: Set the verbosity level." << std::endl;
	std::cerr << "      Level 0 keeps only the error messages." << std::endl;
	std::cerr << "      Level 1 keeps all the messages. " << std::endl;
	std::cerr << " <filename>: Set the filename of the output log file." << std::endl;
	std::cerr << std::endl;
	exit(-1);
}

void ProcessCommandLine(int argc, fchar* argv[])
{
	// Remove the first argument: the current application name...
	--argc; ++argv;
	while (argc > 0)
	{
		if (argv[0][0] == '-' || argv[0][0] == '/')
		{
			++(argv[0]);
			if (fstrcmp(argv[0], FC("-v")) && argc >= 2)
			{
				isVerbose = FUStringConversion::ToBoolean(argv[1]);
				argc -= 2; argv += 2;
			}
			else
			{
				// unknown flag
				ShowHelp(); break;
			}
		}
		else if (argc == 1)
		{
			logFilename = *(argv++);
			--argc;
		}
		else
		{
			ShowHelp(); break;
		}
	}
}

int _tmain(int argc, fchar* argv[])
{
	ProcessCommandLine(argc, argv);

	// In debug mode, output memory leak information and do periodic memory checks
#ifdef PLUG_CRT
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_EVERY_128_DF);
#endif

	FUTestBed testBed(logFilename.c_str(), isVerbose);

#ifndef UE3_DEBUG
	try {
#endif
	// Set the current folder to the folder with the samples DAE files
	_chdir("Samples\\");

	// FMath tests
	RUN_TESTSUITE(FCTestAll)


#ifndef UE3_DEBUG
	}
	catch (const char* sz) { testBed.GetLogFile().WriteLine(sz); }
#ifdef UNICODE
	catch (const fchar* sz) { testBed.GetLogFile().WriteLine(sz); }
#endif
	catch (...) { testBed.GetLogFile().WriteLine("Exception caught!"); }
#endif

	return 0;
}

TESTSUITE_START(FCTestAll)

TESTSUITE_TEST(FColladaAll)

	// FMath tests
	RUN_TESTSUITE(FMArray);
	FUCheckMemory(return false);

	// FUtils tests
	RUN_TESTSUITE(FUObject);
	FUCheckMemory(return false);
	RUN_TESTSUITE(FUCrc32);
	FUCheckMemory(return false);
	RUN_TESTSUITE(FUFunctor);
	FUCheckMemory(return false);
	RUN_TESTSUITE(FUEvent);
	FUCheckMemory(return false);
	RUN_TESTSUITE(FUString);
	FUCheckMemory(return false);
	RUN_TESTSUITE(FUFileManager);
	FUCheckMemory(return false);

	// FCDocument test
	RUN_TESTSUITE(FCDAnimation);
	FUCheckMemory(return false);
	RUN_TESTSUITE(FCDExportReimport);
	FUCheckMemory(return false);
	RUN_TESTSUITE(FCTestXRef);
	FUCheckMemory(return false);
	RUN_TESTSUITE(FCTAssetManagement);
	FUCheckMemory(return false);

TESTSUITE_END