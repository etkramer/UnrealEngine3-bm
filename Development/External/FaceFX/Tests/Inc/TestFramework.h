//------------------------------------------------------------------------------
// Provides a framework for implementing the unit tests for the FaceFx SDK
// Based on CppUnitLite (http://c2.com/cgi/wiki?CppUnitLite).
//
// Owner: John Briggs
//------------------------------------------------------------------------------

#include <vector>
#include <string>
#include <iostream>
#include <sstream>

class TestCase;

class TestFailure
{
public:
	TestFailure( std::string testName, std::string file, int line, std::string condition )
		: _testName( testName )
	{
		_failure = "Condition not met: ";
		_failure += condition;
		_MakeClickyLink( file, line );
	}

	TestFailure( std::string testName, std::string file, int line, std::string expectedResult, std::string actualResult )
		: _testName( testName )
	{
		_failure = "Expected result ";
		_failure += expectedResult;
		_failure += " was not achieved.  The actual result was ";
		_failure += actualResult;
		_MakeClickyLink( file, line );
	}

	std::string GetTestName() 
	{
		return _testName;
	}

	std::string GetFailure()
	{
		return _failure;
	}

	std::string GetClickyLink()
	{
		return _clickyLink;
	}

private:
	void _MakeClickyLink( std::string file, int line )
	{
		std::ostringstream ostream;
		ostream << line;
		_clickyLink = file;
		_clickyLink += "(";
		_clickyLink += ostream.str();
		_clickyLink += "): ";
	}

	std::string _testName;
	std::string _failure;
	std::string _clickyLink;
};

class TestResult
{
public:
	void StartTests()
	{
		_numFailed = 0;
		_numTests  = 0;
		_numChecks = 0;
	}

	void AddFailure( TestFailure& failure )
	{
		std::cout << failure.GetClickyLink() << " Test failed: " << failure.GetTestName() << std::endl;
		std::cout << "\t" << failure.GetFailure() << std::endl;
		_numFailed++;
	}

	void TestWasRun( )
	{
		_numTests++;
	}

	void CheckWasRun( )
	{
		_numChecks++;
	}

	void EndTests()
	{
		std::cout << "Total number of TESTS PERFORMED: " << _numTests << std::endl
			<< "Total number of CHECKS PERFORMED: " << _numChecks << std::endl
			<< "Total number of FAILURES: " << _numFailed << std::endl;
	}

	bool AnyTestFailed()
	{
		return _numFailed != 0;
	}

private:
	int _numTests;
	int _numFailed;
	int _numChecks;
};

class TestFramework	
{
public:
	static void AddTest( TestCase* newTestCase );
	static void RunAllTests( TestResult& testResult );
private:
	static TestFramework& GetInstance();
	std::vector<TestCase*> _testCases;
};

class TestCase
{
public:
	TestCase( const std::string& testName )
		: _testName(testName)
	{
		TestFramework::AddTest( this );
	}
	virtual ~TestCase() {}
	virtual void RunTest( TestResult& testResult ) = 0;
protected:
	std::string _testName;
};

#define UNITTEST( cls, fn ) \
class cls##fn##Test : public TestCase \
{ \
public: \
cls##fn##Test() : TestCase( #cls "::" #fn ) {} \
void RunTest( TestResult& testResult ); \
} cls##fn##TestInstance;\
void cls##fn##Test::RunTest( TestResult& testResult ) \

#define CHECK( exp ) \
testResult.CheckWasRun(); if( !(exp) ) \
{ \
	TestFailure newFailure( _testName, __FILE__, __LINE__, #exp); \
	testResult.AddFailure( newFailure ); \
    return; \
}

#define CHECK_EQUAL( expected, actual ) \
testResult.CheckWasRun(); if( (expected) != (actual) ) \
{ testResult.AddFailure( TestFailure( _testName, __FILE__, __LINE__, #actual, #expected ) ); return; }

