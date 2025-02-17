class MantlePlaceholderCylinder extends Actor
	native;

cpptext
{
	virtual UBOOL IgnoreBlockingBy( const AActor *Other) const;
}

var Pawn PawnToIgnore;

defaultproperties
{
	RemoteRole=ROLE_None

	bCollideActors=TRUE
	bBlockActors=TRUE
	Begin Object Class=CylinderComponent Name=CollisionCylinder
		CollisionRadius=+0021.000000
		CollisionHeight=+0044.000000
		BlockNonZeroExtent=TRUE
		BlockZeroExtent=FALSE
		BlockActors=TRUE
		CollideActors=TRUE
	End Object
	Components.Add(CollisionCylinder)
	CollisionComponent=CollisionCylinder
}
