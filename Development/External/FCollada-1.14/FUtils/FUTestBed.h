/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#ifndef _FU_TESTBED_H_
#define _FU_TESTBED_H_

#ifndef _FU_LOG_FILE_H_
#include "FUtils/FULogFile.h"
#endif // _FU_LOG_FILE_H_

class FUTestSuite;

///////////////////////////////////////////////////////////////////////////////
// QTestBed class
//
class FUTestBed
{
private:
	size_t testPassed, testFailed;
	FULogFile fileOut;
	fstring filename;
	bool isVerbose;

public:
	FUTestBed(const fchar* filename, bool isVerbose);

	inline bool IsVerbose() const { return isVerbose; }
	FULogFile& GetLogFile() { return fileOut; }

	bool RunTestbed(FUTestSuite* headTestSuite);
	void RunTestSuite(FUTestSuite* testSuite);
};

///////////////////////////////////////////////////////////////////////////////
// QTestSuite class
//
class FUTestSuite
{
public:
	virtual size_t GetTestCount() = 0;
	virtual bool RunTest(FUTestBed& testBed, FULogFile& fileOut, size_t testIndex) = 0;
};

///////////////////////////////////////////////////////////////////////////////
// Helpful Macros
//
#if defined(_FU_ASSERT_H_) && defined(UE3_DEBUG)
#define FailIf(a) \
	if ((a)) { \
		fileOut.WriteLine(__FILE__, __LINE__, " Test('%s') failed: %s.", szTestName, #a); \
		extern bool skipAsserts; \
		if (!skipAsserts) { \
			__DEBUGGER_BREAK; \
			skipAsserts = ignoreAssert; } \
		return false; }

#else // _FU_ASSERT_H_ && UE3_DEBUG

#define FailIf(a) \
	if ((a)) { \
		fileOut.WriteLine(__FILE__, __LINE__, " Test('%s') failed: %s.", szTestName, #a); \
		return false; }

#endif // _FU_ASSERT_H_

#define PassIf(a) FailIf(!(a))

#define Fail { bool b = true; FailIf(b); }

///////////////////////////////////////////////////////////////////////////////
// TestSuite Generation Macros.
// Do use the following macros, instead of writing your own.
//
#ifdef ENABLE_TEST
#define TESTSUITE_START(suiteName) \
FUTestSuite* _test##suiteName; \
class FUTestSuite##suiteName : public FUTestSuite \
{ \
public: \
	FUTestSuite##suiteName() : FUTestSuite() { _test##suiteName = this; } \
	bool RunTest(FUTestBed& testBed, FULogFile& fileOut, size_t testIndex) \
	{ \
		switch (testIndex) { \
			case ~0u : { \
				if (testBed.IsVerbose()) { \
					fileOut.WriteLine("Running %s...", #suiteName); \
				}

#define TESTSUITE_TEST(testName) \
			} return true; \
		case (__COUNTER__): { \
			static const char* szTestName = #testName;

#define TESTSUITE_END \
			} return true; \
		} \
		fileOut.WriteLine(__FILE__, __LINE__, " Test suite implementation error."); \
		return false; \
	} \
	size_t GetTestCount() { return __COUNTER__; } \
} __testSuite;

#define RUN_TESTSUITE(suiteName) \
	extern FUTestSuite* _test##suiteName; \
	testBed.RunTestSuite(_test##suiteName);

#define RUN_TESTBED(testBedObject, szTestSuiteHead, testPassed) { \
	extern FUTestSuite* _test##szTestSuiteHead; \
	testPassed = testBedObject.RunTestbed(_test##szTestSuiteHead); }

#else // ENABLE_TEST

// The code will still be compiled, but the linker should take it out.
#define TESTSUITE_START(suiteName) \
	FUTestSuite* _test##suiteName = NULL; \
	bool __testCode##suiteName(FULogFile& fileOut, const char* szTestName) { { fileOut; szTestName;
#define TESTSUITE_TEST(testName) } {
#define TESTSUITE_END } return true; }
#define RUN_TESTSUITE(suiteName)
#define RUN_TESTBED(testBedObject, szTestSuiteHead, testPassed) testPassed = true;

#endif

#endif // _FU_TESTBED_H_