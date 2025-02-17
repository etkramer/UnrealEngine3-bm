/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearScout extends Scout
	DependsOn(GearWallPathNode)
	native
	transient;

cpptext
{
	virtual void InitForPathing( ANavigationPoint* Start, ANavigationPoint* End );
	virtual UClass* GetDefaultReachSpecClass();
	virtual FVector GetSize(FName desc);
	virtual FVector GetDefaultForcedPathSize(UReachSpec* Spec);
	virtual void SetPathColor(UReachSpec* ReachSpec);
	virtual void Exec( const TCHAR* Str );
	virtual void BuildCombatZones( UBOOL bFromDefinePaths = FALSE );
	virtual void AddSpecialPaths( INT NumPaths, UBOOL bOnlyChanged );
	UBOOL CreateLeapPath( ANavigationPoint* Nav, ANavigationPoint* DestNav, FCheckResult Hit, UBOOL bOnlyChanged );
	virtual INT PrunePathsForNav(ANavigationPoint* Nav);
	// called after PrunePathsForNav is called on all pathnodes 
	virtual INT SecondPassPrunePathsForNav(ANavigationPoint* Nav);

};

defaultproperties
{
	Begin Object NAME=CollisionCylinder
		CollisionRadius=+0034.000000
	End Object

	PathSizes.Empty
	PathSizes(0)=(Desc=Wretch,Radius=34,Height=52,PathColor=0)
	PathSizes(1)=(Desc=Common,Radius=34,Height=80,PathColor=1)
	PathSizes(2)=(Desc=Bloodmount,Radius=55,Height=90,PathColor=3)
	PathSizes(3)=(Desc=Boomer,Radius=60,Height=90,PathColor=3)
	PathSizes(4)=(Desc=Max,Radius=72,Height=120,PathColor=2)
	PathSizes(5)=(Desc=Brumak,Radius=200,Height=120,PathColor=4)
}
