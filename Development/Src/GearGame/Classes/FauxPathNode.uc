//=============================================================================
// FauxPathNode
//
// Used to create a navigation network that is searchable through 
// normal path finding functions but is separate from the physical
// path network (ie used by Combat Zones)
//
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class FauxPathNode extends NavigationPoint
	hidecategories(Collision,Display,Force)
	notplaceable
	native;

cpptext
{
	// Only connections made through forced paths
	virtual UBOOL CanConnectTo(ANavigationPoint* Nav, UBOOL bCheckDistance) { return FALSE; }
	// No base needed, network is not connected to physical world
	virtual UBOOL ShouldBeBased() { return FALSE; }

	virtual void CheckForErrors() {}
}
	
defaultproperties
{
	bMovable=FALSE
	bHidden=TRUE
	bHiddenEd=TRUE
	bHiddenEdGroup=TRUE
}