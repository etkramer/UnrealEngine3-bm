/**
 * if( ENUM == var )
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class Test0013_IfEnumEqualEqualVar extends Object;


enum Test0013_SomeEnumFoo
{
	Test0013SEF_ValOne,
	Test0013SEF_ValTwo
};

enum Test0013_AnotherEnumFoo
{
	Test0013AEF_ValOne,
	Test0013AEF_ValTwo,
};
enum ENetRole
{
	ROLE_None,              // No role at all.
	ROLE_SimulatedProxy,	// Locally simulated proxy of this actor.
	ROLE_AutonomousProxy,	// Locally autonomous proxy of this actor.
	ROLE_Authority,			// Authoritative control over the actor.
};

/** enum for LDs to select collision options - sets Actor flags and that of our CollisionComponent via PostEditChange() */
enum ECollisionType
{
	COLLIDE_CustomDefault, // custom programmer set collison (PostEditChange() will restore collision to defaults when this is selected)
	COLLIDE_NoCollision, // doesn't collide
	COLLIDE_BlockAll, // blocks everything
};


var	Test0013_SomeEnumFoo	SomeFoo;
var	Test0013_AnotherEnumFoo	AnotherFoo;

var	ENetRole MirrorEnumTest;
var ECollisionType InvalidMirrorEnumTest;

function EnumEqualEqualVar()
{
`if(`notdefined(FINAL_RELEASE))
    local Test0013_SomeEnumFoo AnEnum;

    AnEnum = Test0013SEF_ValOne;

 //   if( Test0013SEF_ValOne == AnEnum )  // does not compile
 //   {
 //      `log( "" );
 //   }

      `log( "AnEnum: " $ AnEnum );
`endif
}



function EnumTypeEnumEqualEqualVar()
{
    local Test0013_SomeEnumFoo AnEnum;

    AnEnum = Test0013SEF_ValOne;

    if( Test0013_SomeEnumFoo.Test0013SEF_ValOne == AnEnum )
    {
       `log( "" );
    }

      `log( "AnEnum: " $ AnEnum );
}


function CompareDifferentEnumTypes()
{
	local Actor TestActor;

	TestActor = None;

	if ( MirrorEnumTest != TestActor.Role )
	{
		`log("Mirrored enum wasn't handled correctly!");
	}

//	if ( InvalidMirrorEnumTest != TestActor.CollisionType )		// doesn't compile (correct behavior)
//	{
//		`log("Invalid mirrored comparison detected!");
//	}
//
	SomeFoo = Test0013SEF_ValTwo;
	AnotherFoo = Test0013AEF_ValTwo;
//	if ( SomeFoo != AnotherFoo )								// doesn't compile (correct behavior)
//	{
//		`log("Invalid comparison occurred!");
//	}

//	`log(`showvar(SomeFoo) @ `showvar(AnotherFoo) @ `showvar(Test0013_SomeEnumFoo(AnotherFoo)));

	if ( SomeFoo != Test0013_SomeEnumFoo(AnotherFoo) )
	{
		`log("Valid comparison due to casting - FALSE!!!!");
	}
	else
	{
		`log("Valid comparison due to casing - TRUE!!!!!");
	}
}



















