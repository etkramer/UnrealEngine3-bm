/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Route extends Info
	placeable
	native;

cpptext
{
	virtual void PostLoad();

	void AutoFillRoute( ERouteFillAction RFA, TArray<ANavigationPoint*>& Points );
	virtual void GetActorReferences(TArray<FActorReference*> &ActorRefs, UBOOL bIsRemovingLevel);
	virtual void CheckForErrors();
}

enum ERouteFillAction
{
	RFA_Overwrite,
	RFA_Add,
	RFA_Remove,
	RFA_Clear,
};
enum ERouteDirection
{
	ERD_Forward,
	ERD_Reverse,
};
enum ERouteType
{
	/** Move from beginning to end, then stop */
	ERT_Linear,
	/** Move from beginning to end and then reverse */
	ERT_Loop,
	/** Move from beginning to end, then start at beginning again */
	ERT_Circle,
};
var() ERouteType RouteType;

var() deprecated array<NavReference> NavList;
/** List of move targets in order */
var() array<ActorReference> RouteList;
/** Fudge factor for adjusting to next route position faster */
var() float	FudgeFactor;

final native function int ResolveRouteIndex( int Idx, ERouteDirection RouteDirection, out byte out_bComplete, out byte out_bReverse );

/**
 *	Find the closest navigation point in the route
 *	(that is also within tether distance)
 */
final native function int MoveOntoRoutePath( Pawn P, optional ERouteDirection RouteDirection = ERD_Forward, optional float DistFudgeFactor = 1.f );


defaultproperties
{
	Begin Object Name=Sprite
		Sprite=Texture2D'EditorResources.S_Route'
	End Object
	Components.Add(Sprite)

	Begin Object Class=RouteRenderingComponent Name=RouteRenderer
		HiddenGame=True
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(RouteRenderer)

	bStatic=TRUE
	FudgeFactor=1.f
}
