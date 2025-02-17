/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearExplosionActorReplicated extends GearExplosionActor
	native(Weapon);

/** Should be a serializable object, like in the level or something (as is the case for kismet actions). */
var transient private repnotify GearExplosion					ExploTemplateRef;

/** Which type of projectile explosion we will emulate. */
var transient private repnotify class<GearProj_ExplosiveBase>	ProjExploToEmulate;

replication
{
	if (bNetDirty)
		ExploTemplateRef, ProjExploToEmulate;
}

simulated event ReplicatedEvent(Name VarName)
{
	if (VarName == 'ExploTemplateRef')
	{
		Explode(ExploTemplateRef);
	}
	else if (VarName == 'ProjExploToEmulate')
	{
		EmulateProjectileExplosion(ProjExploToEmulate);
	}
	else
	{
		super.ReplicatedEvent(VarName);
	}
}

simulated event Explode(GearExplosion NewExplosionTemplate)
{
	if (Role == ROLE_Authority)
	{
		ExploTemplateRef = NewExplosionTemplate;
	}

	super.Explode(NewExplosionTemplate);

	LifeSpan = FMax(LifeSpan, 1.0);
}

/** Special functionality for Kismet. */
simulated event EmulateProjectileExplosion(class<GearProj_ExplosiveBase> ProjToEmulate)
{
	local GearExplosion ExploTemplate;		// hmm, cannot do this...  something for next week

	ExploTemplate = new(self) class'GearExplosion' (ProjToEmulate.default.ExplosionTemplate);

	ProjToEmulate.static.PrepareDefaultExplosionTemplate(ExploTemplate);
	Explode(ExploTemplate);

	ExploTemplateRef = None; // so it doesn't try to replicate the generated template

	if (Role == ROLE_Authority)
	{
		ProjExploToEmulate = ProjToEmulate;
	}
}

defaultproperties
{
	bNetTemporary=true
	bAlwaysRelevant=TRUE
	RemoteRole=ROLE_SimulatedProxy
	bNetInitialRotation=TRUE
}
