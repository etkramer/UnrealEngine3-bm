/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FUtils/FUTestBed.h"
#include <fstream>

bool skipAsserts = false;

//
// FUTestbed
//

FUTestBed::FUTestBed(const fchar* _filename, bool _isVerbose)
:	testPassed(0), testFailed(0)
,	filename(_filename), fileOut(_filename), isVerbose(_isVerbose)
{
}

bool FUTestBed::RunTestbed(FUTestSuite* headTestSuite)
{
	if (headTestSuite == NULL) return false;

	testPassed = testFailed = 0;

	RunTestSuite(headTestSuite);

	if (isVerbose)
	{
		fileOut.WriteLine("---------------------------------");
		fileOut.WriteLine("Tests passed: %u.", (uint32) testPassed);
		fileOut.WriteLine("Tests failed: %u.", (uint32) testFailed);
		fileOut.WriteLine("");
		fileOut.Flush();

#ifdef _WIN32
		char sz[1024];
		snprintf(sz, 1024, "Testbed score: [%u/%u]", testPassed, testFailed + testPassed);
		sz[1023] = 0;

		size_t returnCode = IDOK;
		returnCode = MessageBoxA(NULL, sz, "Testbed", MB_OKCANCEL);
		if (returnCode == IDCANCEL)
		{
			snprintf(sz, 1024, "write %s ", filename.c_str());
			sz[1023] = 0;
			system(sz);
			return false;
		}
#endif
	}
	return true;
}

void FUTestBed::RunTestSuite(FUTestSuite* testSuite)
{
	if (testSuite == NULL) return;

	size_t testCount = testSuite->GetTestCount();

	testSuite->RunTest(*this, fileOut, ~0u);
	for (size_t j = 0; j < testCount; ++j)
	{
		bool passed = testSuite->RunTest(*this, fileOut, j);
		if (passed) testPassed++;
		else testFailed++;
	}
}
