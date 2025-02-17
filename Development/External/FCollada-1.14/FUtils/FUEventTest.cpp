/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"

#include "FUtils/FUTestBed.h"
#include "FUtils/FUEvent.h"

///////////////////////////////////////////////////////////////////////////////
class FUTCaller
{
public:
	FUEvent0* event0;
	FUEvent2<long, long>* event2;

	FUTCaller()
	{
		event0 = new FUEvent0();
		event2 = new FUEvent2<long, long>();
	}

	~FUTCaller()
	{
		SAFE_DELETE(event0);
		SAFE_DELETE(event2);
	}
};

class FUTCallee
{
public:
	bool isCallback0_1Called;
	bool isCallback0_2Called;
	bool isCallback2Called;

	long callback2Data1;
	long callback2Data2;

	FUTCallee()
	{
		isCallback0_1Called = false;
		isCallback0_2Called = false;
		isCallback2Called = false;

		callback2Data1 = -1;
		callback2Data2 = -1;
	}

	void Callback0_1()
	{
		isCallback0_1Called = true;
	}

	void Callback0_2()
	{
		isCallback0_2Called = true;
	}

	void Callback2(long data1, long data2)
	{
		isCallback2Called = true;
		callback2Data1 = data1;
		callback2Data2 = data2;
	}
};

///////////////////////////////////////////////////////////////////////////////
TESTSUITE_START(FUEvent)

TESTSUITE_TEST(Callback0)
	FUTCaller caller;
	FUTCallee callee;
	FailIf(caller.event0->GetHandlerCount() != 0);
	caller.event0->InsertHandler(new FUFunctor0<FUTCallee, void>(&callee, &FUTCallee::Callback0_1));
	FailIf(caller.event0->GetHandlerCount() != 1);

	(*(caller.event0))();

	FailIf(!callee.isCallback0_1Called);
	caller.event0->RemoveHandler(&callee, &FUTCallee::Callback0_1);

TESTSUITE_TEST(Callback2)
	FUTCaller caller;
	FUTCallee callee;
	caller.event2->InsertHandler(new FUFunctor2<FUTCallee, long, long, void>(&callee, &FUTCallee::Callback2));

	(*(caller.event2))(72, 55);
	FailIf(!callee.isCallback2Called);
	FailIf(callee.callback2Data1 != 72);
	FailIf(callee.callback2Data2 != 55);

	callee.isCallback2Called = false;
	(*(caller.event2))(44, 79);
	FailIf(!callee.isCallback2Called);
	FailIf(callee.callback2Data1 != 44);
	FailIf(callee.callback2Data2 != 79);
	caller.event2->RemoveHandler(&callee, &FUTCallee::Callback2);

TESTSUITE_TEST(MultipleCallback0)
	FUTCaller caller;
	FUTCallee callee;
	caller.event0->InsertHandler(new FUFunctor0<FUTCallee, void>(&callee, &FUTCallee::Callback0_1));
	caller.event0->InsertHandler(new FUFunctor0<FUTCallee, void>(&callee, &FUTCallee::Callback0_2));
	FailIf(caller.event0->GetHandlerCount() != 2);

	callee.isCallback0_1Called = false;
	callee.isCallback0_2Called = false;
	(*(caller.event0))();
	FailIf(!callee.isCallback0_1Called);
	FailIf(!callee.isCallback0_2Called);

	FailIf(caller.event0->GetHandlerCount() != 2);
	caller.event0->RemoveHandler(&callee, &FUTCallee::Callback0_1);
	FailIf(caller.event0->GetHandlerCount() != 1);
	caller.event0->RemoveHandler(&callee, &FUTCallee::Callback0_2);
	FailIf(caller.event0->GetHandlerCount() != 0);

TESTSUITE_END
