/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class SeqAct_GrabCarryCrate extends SequenceAction;

event Activated()
{
	local array<Object> OutObjs;
	local GearPawn MarcusPawn, DomPawn;
	local GearPawn_CarryCrate_Base CratePawn;

	// Get marcus, dom and crate pawns
	GetObjectVars(OutObjs, "Marcus");
	MarcusPawn = GearPawn(Controller(OutObjs[0]).Pawn);
	OutObjs.length = 0;

	GetObjectVars(OutObjs, "Dom");
	DomPawn = GearPawn(Controller(OutObjs[0]).Pawn);
	OutObjs.length = 0;

	GetObjectVars(OutObjs, "Crate");
	CratePawn = GearPawn_CarryCrate_Base(OutObjs[0]);
	OutObjs.length = 0;

	// Look at inputs to see if we want to grab or drop crate
	if(InputLinks[0].bHasImpulse)
	{
		CratePawn.BeginCarry(MarcusPawn, DomPawn);
	}
	else if(InputLinks[1].bHasImpulse)
	{
		CratePawn.EndCarry();
	}
}

defaultproperties
{
	ObjName="Grab Carry Crate"
	ObjCategory="Gear"
	bCallHandler=FALSE

	InputLinks(0)=(LinkDesc="Grab")
	InputLinks(1)=(LinkDesc="Drop")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Marcus",MinVars=1,MaxVars=1)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Dom",MinVars=1,MaxVars=1)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Object',LinkDesc="Crate",MinVars=1,MaxVars=1)
}
