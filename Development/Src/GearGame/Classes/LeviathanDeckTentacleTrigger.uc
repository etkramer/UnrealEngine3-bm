/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class LeviathanDeckTentacleTrigger extends Trigger_ChainsawInteraction
	config(Pawn);


/** So this is used to kill off the attached grenade so the kismet can spawn the effects it wants **/
var config float TimeForAttachedGrenadesToLive;

/** Called when you try and grenade tag one of these trigger. */
simulated function bool GrenadeTagged(GearWeap_GrenadeBase GrenadeWeap, vector HitLocation, vector HitNorm, TraceHitInfo HitInfo)
{
	local GearProj_Grenade NadeProj;
	local Actor AttachTo;
	local SequenceEvent Evt;
	local SeqEvt_LeviathanGrenadeTag TagEvt;

	AttachTo = (Base != None) ? Base : self;
	NadeProj = GrenadeWeap.AttachToObject(AttachTo, HitLocation, HitNorm, HitInfo);
	NadeProj.LifeSpan = TimeForAttachedGrenadesToLive;

	// Look over bound events
	foreach GeneratedEvents(Evt)
	{
		TagEvt = SeqEvt_LeviathanGrenadeTag(Evt);
		if (TagEvt != None)
		{
			// fire it off!
			TagEvt.CheckActivate(self, GrenadeWeap.Instigator, FALSE);
		}
	}

	bDisplayIcon = FALSE;
	UpdateDisplayIcon( GrenadeWeap.Instigator );

	return TRUE;
}

defaultproperties
{
	bNoKismet=TRUE

	Begin Object Name=CollisionCylinder
		BlockZeroExtent=TRUE
	End Object

	bProjTarget=TRUE
	RemoteRole=ROLE_SimulatedProxy
	bNoDelete=FALSE

	SupportedEvents.Add(class'GearGame.SeqEvt_LeviathanGrenadeTag')
}
