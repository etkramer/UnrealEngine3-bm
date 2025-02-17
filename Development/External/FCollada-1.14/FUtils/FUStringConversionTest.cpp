/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"

#include "FUtils/FUTestBed.h"
#include "FUtils/FUString.h"

TESTSUITE_START(FUStringConversion)

TESTSUITE_TEST(UnicodeUTF8)
	string sz = TO_STRING(FS("Grotesque"));
	PassIf(IsEquivalent(sz, "Grotesque"));
	fstring fsz = TO_FSTRING(sz);
	PassIf(IsEquivalent(fsz, FC("Grotesque")));

TESTSUITE_TEST(ConvertPrimitives)
	int32 i = FUStringConversion::ToInt32(FC("17"));
	PassIf(i == 17);
	i = FUStringConversion::ToInt32(FC("-91"));
	PassIf(i == -91);
	uint32 k = FUStringConversion::ToUInt32(FC("47"));
	PassIf(k == 47);
	k = FUStringConversion::ToUInt32("482156");
	PassIf(k == 482156);
	float f = FUStringConversion::ToFloat("485.551");
	PassIf(IsEquivalent(f, 485.551f));
	float g = FUStringConversion::ToFloat("-241.022e-2");
	PassIf(IsEquivalent(g, -241.022e-2f));
	k = FUStringConversion::HexToUInt32(FC("69AbFF"));
	PassIf(k == 0x69ABFF);
	k = FUStringConversion::HexToUInt32(string("0x78291"));
	PassIf(k == 0x78291);
	bool b = FUStringConversion::ToBoolean("true");
	PassIf(b);
	b = FUStringConversion::ToBoolean("0");
	FailIf(b);
	b = FUStringConversion::ToBoolean("FALSE");
	FailIf(b);
	b = FUStringConversion::ToBoolean("1");
	PassIf(b);

TESTSUITE_TEST(Advancing)
	const char* sz = "32 -48 128.44 true";
	uint32 u = FUStringConversion::ToUInt32(&sz);
	PassIf(u == 32);
	int32 i = FUStringConversion::ToInt32(&sz);
	PassIf(i == -48);
	float f = FUStringConversion::ToFloat(&sz);
	PassIf(IsEquivalent(f, 128.44f));
	bool b = FUStringConversion::ToBoolean(sz);
	PassIf(b);

TESTSUITE_TEST(UInt32List)
	const char* sz = "32 31 55 128";
	UInt32List values;
	FUStringConversion::ToUInt32List(sz, values);
	const uint32 expected[] = { 32, 31, 55, 128 };
	const size_t expectedCount = sizeof(expected) / sizeof(uint32);
	PassIf(IsEquivalent(values, expected, expectedCount));

TESTSUITE_TEST(Int32List)
	const char* sz = "1 0 -22 912 -161203";
	Int32List values;
	FUStringConversion::ToInt32List(sz, values);
	const int32 expected[] = { 1, 0, -22, 912, -161203 };
	const size_t expectedCount = sizeof(expected) / sizeof(uint32);
	PassIf(IsEquivalent(values, expected, expectedCount));

TESTSUITE_TEST(TokenList)
	const char* sz = "token1\t\t0 parafin  \t\n  __Aloes\n\n";
	StringList values;
	FUStringConversion::ToStringList(sz, values);
	const string expected[] = { "token1", "0", "parafin", "__Aloes" };
	const size_t expectedCount = sizeof(expected) / sizeof(string);
	PassIf(IsEquivalent(values, expected, expectedCount));

TESTSUITE_TEST(FloatList)
	const char* sz = "-15e03 17.2 0291. 312.e2 182 12";
	FloatList values;
	FUStringConversion::ToFloatList(sz, values);
	const float expected[] = { -15e03f, 17.2f, 0291.f, 312.e2f, 182.f, 12.f };
	const size_t expectedCount = sizeof(expected) / sizeof(float);
	PassIf(IsEquivalent(values, expected, expectedCount));

TESTSUITE_END
