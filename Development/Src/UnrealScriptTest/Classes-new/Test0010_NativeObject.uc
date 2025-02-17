/**
 * Part of the unrealscript execution and compilation regression framework.
 * Native class for testing and regressing issues with native unrealscript classes.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Test0010_NativeObject extends Object
	native
	implements(Test0002_InterfaceNative);

var Test0002_InterfaceNative InterfaceMember;

var array<Test0002_InterfaceNative> InterfaceArray;

struct native export ConstructorStructString
{
	var		string		StringNeedCtor;
	var		int			IntNoCtor;
	var 	Test0002_InterfaceNative InterfaceVar;
};

struct native export ConstructorStructArray
{
	var		int				IntNoCtor;
	var		array<string>	StringArrayNeedCtor;
};


struct native export ConstructorStructCombo
{
	var float			FloatNoCtor;
	var array<string>	StringArrayNeedCtor;
	var string			StringNeedCtor;
	var int				IntNoCtor;
};

struct native NoCtorProps
{
	var int Foo1;
	var bool Foo2;
	var float Foo3;
};

/**
 * This struct is for testing a bug with struct inheritance
 */
struct native MyFirstStruct
{
	var int MyFirstInt;
	var float MyFirstFloat;
	var string MyFirstString;
};

struct native MyStruct extends MyFirstStruct
{
	var	int MyInt;
	var float MyFloat;
	var string MyStrings[5];
};

var ConstructorStructString DefaultStringStruct;
var ConstructorStructArray DefaultArrayStruct;
var ConstructorStructCombo DefaultComboStruct;

var array<MyStruct> MyArray;

cpptext
{
	void TestDoubleLinkedList();
}

native final function PerformNativeTest( int TestNumber );


event Test01_CallEventWithStruct( NoCtorProps NoCtor, ConstructorStructString StringParm, ConstructorStructArray ArrayParm, ConstructorStructCombo ComboParm, bool PaddingBool )
{
	`log(`showvar(StringParm.StringNeedCtor)@`showvar(StringParm.IntNoCtor));
	`log(`showvar(ArrayParm.StringArrayNeedCtor[0])@`showvar(ArrayParm.IntNoCtor));
	`log(`showvar(ComboParm.StringArrayNeedCtor[0])@`showvar(ComboParm.StringNeedCtor)@`showvar(ComboParm.IntNoCtor));
}

native final function Test02_PassNativeInterfaceToNativeFunction( Test0002_InterfaceNative InterfaceParm );

event Test03_CallEventWithNativeInterface( Test0002_InterfaceNative InterfaceParm )
{
	`log(`location@`showvar(InterfaceParm));
}

native function TestNativeFunction( bool bBoolParm );

//======================================================================================================================
event TestInterfaceEvent( object ObjParam )
{
	TestObjectToInterfaceConversions();

	`log("InterfaceRef:");
	`log( DefaultStringStruct.InterfaceVar );

	InterfaceArray.Insert(0,1);
	InterfaceArray[0] = Test0002_InterfaceNative(ObjParam);
}

function TestObjectToInterfaceConversions()
{
	local Test0002_InterfaceNative InterfaceLocal;

	if ( InterfaceMember == Self )
	{
		`log("InterfaceMember is Self!");
	}
	if ( Self == InterfaceMember )
	{
		`log("InterfaceMember is Self!");
	}

	`log("InterfaceMember:");

	/**
	using primitiveCast instead of InterfaceCast, resolving to InterfaceToBool, rather than InterfaceToString
		execInterfaceToBool
		execInstanceVariable
	*/
	`log( InterfaceMember );

	InterfaceLocal = None;
	if ( InterfaceMember != None )
	{
		InterfaceLocal = InterfaceMember;
		InterfaceMember = None;
	}

	if ( None == InterfaceMember )
	{
		`log("Successful!");
	}

	if ( InterfaceMember == Self )
	{
		`log("InterfaceMember is Self!");
	}

	if ( InterfaceLocal == Self )
	{
		`log("InterfaceLocal is Self, Interface => Obj");
	}

	if ( Self == InterfaceLocal )
	{
		`log("InterfaceLocal is Self, Obj => Interface");
	}

	`log(`showvar(InterfaceArray[0]));
	`log(`showvar(InterfaceLocal));
}

//======================================================================================================================

event Test05_StructInheritance()
{
`if(`notdefined(FINAL_RELEASE))
	local int test;

	test = MyArray[0].MyInt;
`endif

	`log( "test:" @ test );

	`log( "MyArray[0].MyInt:" @ MyArray[0].MyInt );
	`log( "MyArray[0].MyFloat" @ MyArray[0].MyFloat );
	`log( "MyArray[0].MyStrings[0]:" @ MyArray[0].MyStrings[0] );
	`log( "MyArray[0].MyStrings[1]:" @ MyArray[0].MyStrings[1] );
	`log( "MyArray[0].MyStrings[2]:" @ MyArray[0].MyStrings[2] );
	`log( "MyArray[0].MyFirstInt:" @ MyArray[0].MyFirstInt ); // dies here
	`log( "MyArray[0].MyFirstFloat:" @ MyArray[0].MyFirstFloat );
	`log( "MyArray[0].MyFirstString:" @ MyArray[0].MyFirstString );
}

//======================================================================================================================

event Test06_InterfaceToObjectConversions()
{
	local Object ObjectLocal;
	local Test0002_InterfaceNative InterfaceLocal;
`if(`notdefined(FINAL_RELEASE))
    local int ValidateMemoryInt;

	// give this variable a non-zero value so we can detect if its value is being overwritten
	ValidateMemoryInt = 10;
`endif

	ObjectLocal = Self;
	InterfaceLocal = Self;

	// verify that assigning an interface variable to an object variable works correctly
	ObjectLocal = InterfaceLocal;

	// output should be "10", but will be a pointer value if interface => object assignment isn't working correctly
	`log(`showvar(ValidateMemoryInt) @ `showvar(InterfaceLocal) @ `showvar(ObjectLocal));


	// now verify that the script compiler disallows conversions in out parms
	TestInterfaceObject_OutParmCompatibility(ObjectLocal, InterfaceLocal);	// this should work.
	//TestInterfaceObject_OutParmCompatibility(InterfaceLocal, ObjectLocal);	// this should not work.

	VerifyConversionFromInterfaceToObjectAsNativeParm( InterfaceLocal, 10 );
}

function TestInterfaceObject_OutParmCompatibility( out Object out_Object, out Test0002_InterfaceNative out_Interface )
{
	if ( out_Object == out_Interface )
	{
		// nothing - just here to make sure the function isn't removed by optimization
	}
}

/**
 * this method exists to verify that passing an interface variable as the value for an object parameter of a native function
 * works correctly.
 * The value of DummyInt will be a pointer value if this test fails
 */
native final function VerifyConversionFromInterfaceToObjectAsNativeParm( Object InObject, int DummyInt );

//======================================================================================================================
event TestMultipleOptionalParms()
{
	`log("RUNNING TEST WITH NO VALUES");
	FunctionWithMultipleOptionalParms();

	`log("RUNNING TEST WITH VALUE FOR FIRST PARM");
	FunctionWithMultipleOptionalParms("Specified value");

	`log("RUNNING TEST WITH VALUE FOR SECOND PARM");
	FunctionWithMultipleOptionalParms(,"Specified Value");
}

function FunctionWithMultipleOptionalParms( optional string FirstString="First DefaultValue", optional string SecondString="Second Default Value" )
{
	`log(`location @ `showvar(FirstString) @ `showvar(SecondString));
}

event TestOptionalEventStringParm( optional string FirstString, optional string SecondString="Default Value" )
{
	`log(`location @ `showvar(FirstString) @ `showvar(SecondString));
}

defaultproperties
{
	DefaultStringStruct=(StringNeedCtor="StringNeedCtorValue",IntNoCtor=5)
	DefaultArrayStruct=(StringArrayNeedCtor=("Value1","Value2"),IntNoCtor=10)
	DefaultComboStruct=(StringArrayNeedCtor=("Value3","Value4"),StringNeedCtor="StringValue"),IntNoCtor=2)
}

