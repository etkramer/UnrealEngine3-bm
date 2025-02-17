/**
 * Class description
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Test0022_StructMemberAccess extends TestClassBase;


struct InnerStruct
{
	var int InnerStructInt;
};

struct OuterStruct
{
	var float 				OuterStruct_Float;
	var InnerStruct			OuterStruct_NestedStruct;
};

struct ExtendedOuterStruct extends OuterStruct
{
	var array<InnerStruct>	OuterStruct_StructArray;
};

struct CtorStruct
{
	var string				CtorStruct_String;
};

var		OuterStruct			m_OuterStruct;
var		ExtendedOuterStruct	m_DerivedOuterStruct;
var		CtorStruct			m_CtorStruct;
var		array<InnerStruct>	m_StructArray;

var	const	InnerStruct		m_ConstStruct;


var		delegate<ReturnStructDelegate>	m_StructDelegateProp;
var		delegate<ReturnStructDelegate>	m_ConstStructDelegateProp;


delegate OuterStruct ReturnStructDelegate()
{
}

// demonstrates simple member access for a struct that does not contain any properties requiring construction
function SimpleStructAccess_NonCtorTest()
{
	local float ValueFloat;
	local int ValueInt;

	// test normal r-value access
	ValueFloat = m_OuterStruct.OuterStruct_Float;

	// test normal l-value access
	m_OuterStruct.OuterStruct_Float = ValueFloat;

	// test nested r-value access
	ValueInt = m_OuterStruct.OuterStruct_NestedStruct.InnerStructInt;

	// test nested l-value access
	m_OuterStruct.OuterStruct_NestedStruct.InnerStructInt = ValueInt;

	// test normal r-value access through const member struct
	ValueInt = m_ConstStruct.InnerStructInt;
}

// demonstrates simple member access for a struct that contains properties requiring construction
function SimpleStructAccess_CtorTest()
{
	local string ValueString;

	// test normal r-value access
	ValueString = m_CtorStruct.CtorStruct_String;

	// test normal l-value access
	m_CtorStruct.CtorStruct_String = ValueString;
}

// demonstrates member access through array element
function ArrayStructAccessTest()
{
	local int ArrayInt;

	// test r-value access through array element
	ArrayInt = m_StructArray[0].InnerStructInt;

	// test l-value access through array element
	m_StructArray[0].InnerStructInt = ArrayInt;

	// test nested r-value access through array element
	ArrayInt = m_DerivedOuterStruct.OuterStruct_StructArray[0].InnerStructInt;

	// test nested l-value access through array element
	m_DerivedOuterStruct.OuterStruct_StructArray[0].InnerStructInt = ArrayInt;
}

// demonstrates member access through struct return value
function ReturnValueAccessTest()
{
	local OuterStruct LocalStruct;
	local float LocalFloat;

	// test without member access
	LocalStruct = ReturnMemberStruct();

	// test simple member access through return value
	LocalFloat = ReturnMemberStruct().OuterStruct_Float;

	// test simple member access through return value from inline struct member access
	LocalFloat = ReturnInlineStruct().X;

	// to avoid script compiler warnings.
	m_OuterStruct = LocalStruct;
	m_OuterStruct.OuterStruct_Float = LocalFloat;
}

function DelegateReturnValueAccessTest()
{
`if(`notdefined(FINAL_RELEASE))
	local InnerStruct LocalStruct;
`endif

    m_StructDelegateProp = ReturnMemberStruct;

`if(`notdefined(FINAL_RELEASE))
	LocalStruct = m_StructDelegateProp().OuterStruct_NestedStruct;
	LocalStruct = m_ConstStructDelegateProp().OuterStruct_NestedStruct;

	// avoid the compiler warning
	`log(LocalStruct.InnerStructInt);
`endif
}

// demonstrates member access through literal struct
function InlineStructAccessTest()
{
	local float VectorX;

	VectorX = vect(100,200,300).X;
	m_OuterStruct.OuterStruct_Float = VectorX;
}


// returns a member struct
private function OuterStruct ReturnMemberStruct()
{
	return m_OuterStruct;
}

// returns an inline struct
private function vector ReturnInlineStruct()
{
	return vect(1000,2000,3000);
}

// verifies that accessing a struct member through an out-of-bounds array element does not expand the array.
function TestInvalidArrayExpansion()
{
	`log(`location@"BEFORE:"@`showvar(m_StructArray.Length));

	m_StructArray[3].InnerStructInt = 5;

	`log(`location@"AFTER:"@`showvar(m_StructArray.Length));
}

DefaultProperties
{
	m_OuterStruct=(OuterStruct_Float=50.f,OuterStruct_NestedStruct=(InnerStructInt=1000))
	m_DerivedOuterStruct=(OuterStruct_Float=100.f,OuterStruct_NestedStruct=(InnerStructInt=2000),OuterStruct_StructArray=((InnerStructInt=1),(InnerStructInt=2)))
	m_CtorStruct=(CtorStruct_String="DefaultValue")
	m_StructArray=((InnerStructInt=10),(InnerStructInt=20))
	m_ConstStruct=(InnerStructInt=100)
	m_ConstStructDelegateProp=ReturnMemberStruct
}
