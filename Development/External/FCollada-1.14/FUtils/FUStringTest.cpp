/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FUtils/FUTestBed.h"
#include "FUtils/FUString.h"

TESTSUITE_START(FUString)

TESTSUITE_TEST(Equivalency)
	// Just verify some of the string<->const char* equivalency
	// in both Unicode and UTF-8 versions
	PassIf(IsEquivalent("Tetrahedron", string("Tetrahedron")));
	PassIf(IsEquivalent(string("Tetrahedron"), "Tetrahedron"));
	PassIf(IsEquivalent("My alter-ego", "My alter-ego"));
	
	FailIf(IsEquivalent("MY ALTER-EGO", "My alter-ego")); // should not be case-sensitive.
	FailIf(IsEquivalent("Utopia", "Utopian"));
	FailIf(IsEquivalent(string("Greatness"), "Great"));
	FailIf(IsEquivalent("Il est", "Il  est"));
	PassIf(IsEquivalent("Prometheian", "Prometheian\0\0\0")); // This is the only different allowed between two strings.

	string sz1 = "Test1";
	string sz2("Test1");
	PassIf(IsEquivalent(sz1, sz2)); // Extra verification in case the compiler optimizes out the string differences above.

	RUN_TESTSUITE(FUStringBuilder);
	RUN_TESTSUITE(FUStringConversion);
	RUN_TESTSUITE(FUUniqueStringMap);

TESTSUITE_END
