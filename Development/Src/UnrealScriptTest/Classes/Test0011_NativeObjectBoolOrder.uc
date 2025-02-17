/**
 * Part of the unrealscript execution and compilation regression framework.
 * Native class for testing and regressing issues with native unrealscript classes.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Test0011_NativeObjectBoolOrder extends Object
	native;

var()	bool	bMemberBoolA, bMemberBoolB, bMemberBoolC;

native function PerformBoolOrderTest();

native function bool NativeTestBoolOrderFunction( bool BoolParm );

event NativeTestBoolOrderEvent( bool EventBoolParm )
{
`if(`notdefined(FINAL_RELEASE))
	local bool bLocalBoolLiteral, bLocalBoolParm, bLocalBoolMemberTrue, bLocalBoolMemberFalse;

	`log(`showvar(EventBoolParm,InputBoolValue));
	`log("");

	bLocalBoolLiteral = NativeTestBoolOrderFunction(true);
	`log(`showvar(bLocalBoolLiteral,LiteralBoolResult));
	`log("");

	bLocalBoolParm = NativeTestBoolOrderFunction(EventBoolParm);
	`log(`showvar(bLocalBoolParm,InputBoolResult));
	`log("");

	bLocalBoolMemberTrue = NativeTestBoolOrderFunction(bMemberBoolC);
	`log(`showvar(bLocalBoolMemberTrue));
	`log("");

	bLocalBoolMemberFalse = NativeTestBoolOrderFunction(bMemberBoolB);
	`log(`showvar(bLocalBoolMemberFalse));
	`log("");
`endif
}

DefaultProperties
{
	bMemberBoolA=True
	bMemberBoolB=False
	bMemberBoolC=True
}
