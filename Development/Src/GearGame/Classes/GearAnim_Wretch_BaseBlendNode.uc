/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class GearAnim_Wretch_BaseBlendNode extends GearAnim_BaseBlendNode
	native(Anim);

cpptext
{
	virtual void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight );
}

defaultproperties
{
	NodeName="WretchBaseBlendNode"

	Children.Empty
	Children(0)=(Name="Normal")
	Children(1)=(Name="Wall")
	Children(2)=(Name="Ceiling")
	Children(3)=(Name="Cover")
	Children(4)=(Name="UNUSED")
	Children(5)=(Name="UNUSED")


	ChildBlendInTime.Empty
	ChildBlendInTime(0)=0.20
	ChildBlendInTime(1)=0.20
	ChildBlendInTime(2)=0.20
	ChildBlendInTime(3)=0.20
	ChildBlendInTime(4)=0.20
	ChildBlendInTime(5)=0.20
}


