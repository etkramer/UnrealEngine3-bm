/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class UTProj_BioGlobling extends UTProj_BioShot;

simulated function ProcessTouch(Actor Other, Vector HitLocation, Vector HitNormal)
{
	if ( Other.bProjTarget && (UTProj_BioGlob(Other) == None) && !bExploded )
	{
		Other.TakeDamage(Damage, InstigatorController, Location, MomentumTransfer * Normal(Velocity), MyDamageType,, self);
		Explode( HitLocation, HitNormal );
	}
}

auto state Flying
{
	simulated function ProcessTouch(Actor Other, Vector HitLocation, Vector HitNormal)
	{
		Global.ProcessTouch(Other, HitLocation, HitNormal);
   	}
}

state OnGround
{
	simulated function ProcessTouch(Actor Other, Vector HitLocation, Vector HitNormal)
	{
		Global.ProcessTouch(Other, HitLocation, HitNormal);
	}
}