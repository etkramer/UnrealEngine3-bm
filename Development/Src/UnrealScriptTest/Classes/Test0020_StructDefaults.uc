/**
 * Performs various tests for struct defaults
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/
class Test0020_StructDefaults extends TestClassBase
	editinlinenew;

struct TestInnerNestedStruct
{
	var		int		InnerNestedStructInt;

structdefaultproperties
{
	InnerNestedStructInt=777
}
};

struct TestNestedStruct
{
	var()	int		NestedStructInt;
	var		TestInnerNestedStruct	InnerStructNoOverride, InnerStructOverrideValue;

structdefaultproperties
{
	NestedStructInt=100
	InnerStructOverrideValue=(InnerNestedStructInt=1000)
}

};

struct TestStruct
{
	var() int TestInt;
	var() float TestFloat;
	var() TestNestedStruct TestMemberStruct;
	var() TestNestedStruct TestMemberStruct2;

structdefaultproperties
{
	TestInt=8
	TestFloat=2.0f
	TestMemberStruct2=(NestedStructInt=300,InnerStructNoOverride=(InnerNestedStructInt=1111),InnerStructOverrideValue=(InnerNestedStructInt=2222))
}
};

`define IncludeMemberStructs RemoveThisWordToDisable

`if(`IncludeMemberStructs)
var() TestStruct	StructVar_NoDefaults;
var() TestStruct	StructVar_ClassDefaults;

struct MyAtomicStruct
{
	var() int foo;

structdefaultproperties
{
	foo=10
}
};

struct MyCompoundStruct
{
	var MyAtomicStruct atomicStruct;
};

var() array<MyCompoundStruct> CompoundStructs;

`endif

var() int			ClassIntVar;
var() string		ClassStringVar;

final function LogPropertyValues( optional TestStruct ParameterStruct )
{
//	local int x;
//	ff
`if(`notdefined(FINAL_RELEASE))
	local TestStruct LocalTestStruct;

	`log("Time: " $ `Time @ "Date:" @ `Date);
	`log(`showvar(ClassIntVar));
	`log(`showvar(ClassStringVar));
	`log("");
	`log("");
	`log(`showvar(LocalTestStruct.TestInt));
	`log(`showvar(LocalTestStruct.TestFloat));
	`log("");
	`log("");
	`log(`showvar(LocalTestStruct.TestMemberStruct.NestedStructInt));
	`log(`showvar(LocalTestStruct.TestMemberStruct.InnerStructNoOverride.InnerNestedStructInt));
	`log(`showvar(LocalTestStruct.TestMemberStruct.InnerStructOverrideValue.InnerNestedStructInt));
	`log("");
	`log(`showvar(LocalTestStruct.TestMemberStruct2.NestedStructInt));
	`log(`showvar(LocalTestStruct.TestMemberStruct2.InnerStructNoOverride.InnerNestedStructInt));
	`log(`showvar(LocalTestStruct.TestMemberStruct2.InnerStructOverrideValue.InnerNestedStructInt));
	`log("");
	`log("");
	`log(`showvar(ParameterStruct.TestInt));
	`log(`showvar(ParameterStruct.TestFloat));
	`log("");
	`log(`showvar(ParameterStruct.TestMemberStruct.NestedStructInt));
	`log(`showvar(ParameterStruct.TestMemberStruct.InnerStructNoOverride.InnerNestedStructInt));
	`log(`showvar(ParameterStruct.TestMemberStruct.InnerStructOverrideValue.InnerNestedStructInt));
	`log("");
	`log(`showvar(ParameterStruct.TestMemberStruct2.NestedStructInt));
	`log(`showvar(ParameterStruct.TestMemberStruct2.InnerStructNoOverride.InnerNestedStructInt));
	`log(`showvar(ParameterStruct.TestMemberStruct2.InnerStructOverrideValue.InnerNestedStructInt));

`if(`IncludeMemberStructs)
	`log("");
	`log("");
	`log(`showvar(StructVar_NoDefaults.TestInt));
	`log(`showvar(StructVar_NoDefaults.TestFloat));
	`log("");
	`log(`showvar(StructVar_NoDefaults.TestMemberStruct.NestedStructInt));
	`log(`showvar(StructVar_NoDefaults.TestMemberStruct.InnerStructNoOverride.InnerNestedStructInt));
	`log(`showvar(StructVar_NoDefaults.TestMemberStruct.InnerStructOverrideValue.InnerNestedStructInt));
	`log("");
	`log(`showvar(StructVar_NoDefaults.TestMemberStruct2.NestedStructInt));
	`log(`showvar(StructVar_NoDefaults.TestMemberStruct2.InnerStructNoOverride.InnerNestedStructInt));
	`log(`showvar(StructVar_NoDefaults.TestMemberStruct2.InnerStructOverrideValue.InnerNestedStructInt));
	`log("");
	`log("");
	`log(`showvar(StructVar_ClassDefaults.TestInt));
	`log(`showvar(StructVar_ClassDefaults.TestFloat));
	`log("");
	`log(`showvar(StructVar_ClassDefaults.TestMemberStruct.NestedStructInt));
	`log(`showvar(StructVar_ClassDefaults.TestMemberStruct.InnerStructNoOverride.InnerNestedStructInt));
	`log(`showvar(StructVar_ClassDefaults.TestMemberStruct.InnerStructOverrideValue.InnerNestedStructInt));
	`log("");
	`log(`showvar(StructVar_ClassDefaults.TestMemberStruct2.NestedStructInt));
	`log(`showvar(StructVar_ClassDefaults.TestMemberStruct2.InnerStructNoOverride.InnerNestedStructInt));
	`log(`showvar(StructVar_ClassDefaults.TestMemberStruct2.InnerStructOverrideValue.InnerNestedStructInt));

	CompoundStructs.Length = CompoundStructs.Length + 1;
	`log(`showvar(CompoundStructs[0].atomicStruct.foo));
	`log(`showvar(CompoundStructs[1].atomicStruct.foo));
	`log(`showvar(CompoundStructs[2].atomicStruct.foo));
`endif
`endif
}

DefaultProperties
{
	ClassIntVar=16
	ClassStringVar="ClassStringDefault"
`if(`IncludeMemberStructs)
	StructVar_ClassDefaults=(TestInt=16,TestFloat=10.f,TestMemberStruct2=(InnerStructOverrideValue=(InnerNestedStructInt=3333)))
	CompoundStructs(0)=(atomicStruct=(foo=12))
	CompoundStructs(1)=()
`endif
}
