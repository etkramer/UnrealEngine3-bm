//------------------------------------------------------------------------------
// Provides a framework for implementing the unit tests for the FaceFx SDK
// Based on CppUnitLite (http://c2.com/cgi/wiki?CppUnitLite).
//
// Owner: John Briggs
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "TestFramework.h"

void TestFramework::AddTest( TestCase* newTestCase )
{
	GetInstance()._testCases.push_back( newTestCase );
}

void TestFramework::RunAllTests( TestResult& testResult )
{
	testResult.StartTests();
	for( size_t i = 0; i < GetInstance()._testCases.size(); ++i )
	{
		GetInstance()._testCases[i]->RunTest( testResult );
		testResult.TestWasRun();
	}
	testResult.EndTests();
}

TestFramework& TestFramework::GetInstance()
{
	static TestFramework instance;
	return instance;
}
