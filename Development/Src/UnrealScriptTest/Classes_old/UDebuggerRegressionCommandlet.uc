/**
 * Commandlet for testing and regression of UDebugger execution.
 * To run, use: run UnrealScriptTest.UDebuggerRegression
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class UDebuggerRegressionCommandlet extends TestCommandletBase;

event int Main( string Parms )
{
	local array<Actor> T;
	local int i;

	T.Length = 0; // to prevent the "never assigned a value" warning
	i = T.Find(None);

	if ( T.Find(None) == -1 )
	{
		`log("safe");
	}

	if ( -1 == i )
	{
		`log("safe");
	}

	if ( -1 == T.Find(None) )
	{
		`log("crash");
	}
	return 0;
}

/** some PushState/PopState tests for actors

function StopThisState()
{
	`log("Default implementation of StopThisState");
}

exec function DebugTest0()
{
	GotoState('TestState1');
}

exec function EndTest()
{
	StopThisState();
	StopThisState();
}

exec function DebugTest1()
{
	GotoState('TestState1');
	SetTimer( 1.f, false, nameof(StopThisState) );
}

exec function DebugTest2()
{
	GotoState('TestState1', 'Test2');
}

exec function DebugTest3()
{
	GotoState('TestState1', 'Test3');
}

exec function DebugTest4()
{
	PushState('TestState1');
}

State TestState1
{
	function StopThisState()
	{
		`log("This is the testState1 version");
	}

Test2:
	PushState('TestState2','Test2');
	Stop;

Test3:
	PushState('TestState2','Test3');
	Stop;

Begin:
	Sleep(0.1f);
	PushState('TestState2');

	`log("This is the rest of TestState1's state code");
}

State TestState2
{
	Function StopThisState()
	{
		`Log("Stopping state");
		PopState();
	}

	function NestedPopState()
	{
		StopThisState();
	}

Begin:
	While (true)
	{
		//wait for external event to finish us!
		Sleep(0.1f);
	}

Test2:
	Sleep(5.f);
	StopThisState();


Test3:
	Sleep(5.f);
	NestedPopState();
}
*/
DefaultProperties
{

}
