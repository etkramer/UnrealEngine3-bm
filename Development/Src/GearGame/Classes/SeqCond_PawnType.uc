/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqCond_PawnType extends SequenceCondition
	native(Sequence);

cpptext
{
	virtual void DeActivated();
}

var Object CheckPawn;

struct native ArrayHack
{
	var array<name> PawnTypes;
};
var array<ArrayHack > PawnTypes;

defaultproperties
{
	ObjName="Pawn Type"

	OutputLinks(0)=(LinkDesc="Marcus")
	PawnTypes(0)=(PawnTypes=("GearPawn_COGMarcus"))

	OutputLinks(1)=(LinkDesc="Dom")
	PawnTypes(1)=(PawnTypes=("GearPawn_COGDom"))

	OutputLinks(2)=(LinkDesc="COG",bHidden=TRUE)
	PawnTypes(2)=(PawnTypes=("GearPawn_COGGear"))

	OutputLinks(3)=(LinkDesc="Locust",bHidden=TRUE)
	PawnTypes(3)=(PawnTypes=("GearPawn_LocustDrone","GearPawn_LocustWretchBase","GearPawn_LocustBoomer"))

	OutputLinks(4)=(LinkDesc="Drone",bHidden=TRUE)
	PawnTypes(4)=(PawnTypes=("GearPawn_LocustDrone"))

	OutputLinks(5)=(LinkDesc="Wretch",bHidden=TRUE)
	PawnTypes(5)=(PawnTypes=("GearPawn_LocustWretchBase"))

	OutputLinks(6)=(LinkDesc="Boomer",bHidden=TRUE)
	PawnTypes(6)=(PawnTypes=("GearPawn_LocustBoomer"))

	OutputLinks(7)=(LinkDesc="Unknown",bHidden=TRUE)

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Pawn",PropertyName=CheckPawn)
}
