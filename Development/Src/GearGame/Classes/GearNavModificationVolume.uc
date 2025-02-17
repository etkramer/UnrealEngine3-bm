class GearNavModificationVolume extends Volume
	native
	placeable;

/** whether the volume is currently affecting NavigationPoints */
var() bool bEnabled;
/** when this volume is enabled NavigationPoints within have this amount of extra cost added to them */
var() int ExtraCost;
/** if set and an AI is found taking cover on an affected navigation point, it will be told to move ASAP */
var() bool bInvalidateAICover;

/** list of NavigationPoints currently affected */
var array<NavigationPoint> AffectedPoints;

cpptext
{
protected:
	virtual void UpdateComponentsInternal(UBOOL bCollisionUpdate = FALSE);
public:
}

/** updates what NavigationPoints are within us and applies our effects to them */
native final function UpdateAffectedNavPoints();

event PostBeginPlay()
{
	Super.PostBeginPlay();

	// delay a bit since if we are part of a streaming level we have likely spent a lot of time this tick doing that
	SetTimer( 0.5, false, nameof(UpdateAffectedNavPoints) );
}

simulated function OnToggle(SeqAct_Toggle InAction)
{
	local bool bNewEnabled;

	if (InAction.InputLinks[0].bHasImpulse)
	{
		bNewEnabled = true;
	}
	else if (InAction.InputLinks[1].bHasImpulse)
	{
		bNewEnabled = false;
	}
	else if (InAction.InputLinks[2].bHasImpulse)
	{
		bNewEnabled = !bEnabled;
	}

	if (bNewEnabled != bEnabled)
	{
		bEnabled = bNewEnabled;
		UpdateAffectedNavPoints();
	}
}

defaultproperties
{
	bEnabled=true
	bCollideActors=false
	bStatic=false
	bNoDelete=true
	ExtraCost=5000
	bInvalidateAICover=true
}
