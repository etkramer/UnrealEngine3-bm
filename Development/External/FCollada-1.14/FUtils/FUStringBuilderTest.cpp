/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"

#include "FUtils/FUTestBed.h"
#include "FUtils/FUString.h"

TESTSUITE_START(FUStringBuilder)

TESTSUITE_TEST(Creation_Equivalence)
	FUSStringBuilder builder;
	PassIf(IsEquivalent(builder, ""));
	FUSStringBuilder builder1("55qa");
	PassIf(IsEquivalent(builder1, "55qa"));
	FUSStringBuilder builder2('c', 1);
	PassIf(IsEquivalent(builder2, "c"));
	FUSStringBuilder builder3(5);
	PassIf(IsEquivalent(builder3, ""));

TESTSUITE_TEST(Modifications)
	FUSStringBuilder builder;
	builder.append((int32) 12);
	PassIf(IsEquivalent(builder, "12"));
	builder.append("  aa");
	PassIf(IsEquivalent(builder, "12  aa"));
	builder.remove(4);
	PassIf(IsEquivalent(builder, "12  "));
	builder.append('s');
	PassIf(IsEquivalent(builder, "12  s"));
	builder.append(55.55);
	PassIf(strstr(builder.ToCharPtr(), "12  s55.55") != NULL);
	builder.clear();
	PassIf(IsEquivalent(builder, ""));
	builder.pop_back();
	PassIf(IsEquivalent(builder, ""));
	builder.append(677u);
	PassIf(IsEquivalent(builder, "677"));
	builder.pop_back();
	PassIf(IsEquivalent(builder, "67"));
	builder.remove(0, 1);
	PassIf(IsEquivalent(builder, "7"));

TESTSUITE_END
