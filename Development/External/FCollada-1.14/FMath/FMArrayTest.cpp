/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	Available only to licensees.
	Distribution of this file or its content is strictly prohibited.
*/

#include "StdAfx.h"

#include "FUtils/FUTestBed.h"
#include "FMath/FMArray.h"

////////////////////////////////////////////////////////////////////////
static const uint32 testValues[] = { 1, 2, 3, 5, 7 };
static const size_t testValueCount = sizeof(testValues) / sizeof(uint32);

#define EXPS { static const uint32 expected[] = 
#define EXPE ; \
	static const size_t expectedCount = sizeof(expected) / sizeof(uint32); \
	PassIf(IsEquivalent(testV, expected, expectedCount)); }

////////////////////////////////////////////////////////////////////////
TESTSUITE_START(FMArray)

TESTSUITE_TEST(Equivalency)
	// Start with checking the equivalency functions,
	// since we'll use these during the testing
	vector<uint32> testV1;
	testV1.push_back(1); testV1.push_back(2); testV1.push_back(3); testV1.push_back(5); testV1.push_back(7);
	PassIf(testV1 == testV1);
	PassIf(IsEquivalent(testV1, testV1));
	vector<uint32> testV2;
	testV2.push_back(1); testV2.push_back(2); testV2.push_back(3); testV2.push_back(5); testV2.push_back(7);
	PassIf(IsEquivalent(testV1, testV2));
	PassIf(IsEquivalent(testV1, testValues, testValueCount));

	// Check the custom constructor
	vector<uint32> testV3(testValues, testValueCount);
	PassIf(IsEquivalent(testV3, testV1));

TESTSUITE_TEST(Contains)
	vector<uint32> testV(testValues, testValueCount);
	PassIf(testV.contains(3));
	PassIf(testV.contains(1));
	PassIf(testV.contains(7));
	FailIf(testV.contains(10));
	FailIf(testV.contains(4));

TESTSUITE_TEST(Erase)
	vector<uint32> testV(testValues, testValueCount);
	FailIf(!testV.erase((uint32) 3));
	EXPS {1, 2, 5, 7} EXPE;
	FailIf(!testV.erase((uint32) 5));
	EXPS {1, 2, 7} EXPE;
	PassIf(!testV.erase((uint32) 6));
	EXPS {1, 2, 7} EXPE;
	testV.erase((size_t) 1);
	EXPS {1, 7} EXPE;
	testV.erase((size_t) 0);
	EXPS {7} EXPE;

TESTSUITE_TEST(Find)
	vector<uint32> testV(testValues, testValueCount);
	PassIf(testV.find(3) == testV.begin() + 2);
	PassIf(testV.find(7) == testV.begin() + 4);
	testV.erase((size_t) 0);
	PassIf(testV.find(3) == testV.begin() + 1);

TESTSUITE_END
