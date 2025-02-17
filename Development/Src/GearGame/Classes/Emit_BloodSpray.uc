/** 
 * Blood Spray Emitter
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Emit_BloodSpray extends SpawnedGearEmitter;


function Recycle()
{
	SetHidden(True);
}


function Reset()
{
	Recycle();
}


event Destroyed()
{
	Recycle();
}


function ReInit()
{
	SetHidden(False);
}


defaultproperties
{
    // @todo remove this post TGS and post objectPool has been created
	bDestroyOnSystemFinish=false // this will work for TGS as only the Player does damage (dom has been castrated)

	ParticleSystem=ParticleSystem'AW-Particles.Bloodspray'
	RemoteRole=ROLE_SimulatedProxy
}
