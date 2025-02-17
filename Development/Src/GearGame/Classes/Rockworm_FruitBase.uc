/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class Rockworm_FruitBase extends Actor
	abstract;

/** particle system to play just before we destroy ourselves */
var ParticleSystem FruitEatPS;

var SpawnedGearEmitter FruitEatEmitter;

/** rockworm who is eating me */
var repnotify GearPawn_RockwormBase RockwormEater;

/** delay after we start to be eaten to destroy ourself */
var() float EatDestroyDelay;

var class<DamageType> LastDT;
var int				  LastDamageAmt;
var actor			  LastDamager;


replication
{
	if(ROLE==ROLE_Authority && bNetDirty)
		RockwormEater;
}

simulated event ReplicatedEvent(name VarName)
{
	if (VarName == 'RockwormEater')
	{
		if(RockwormEater != none)
		{
			NotifyBeingEaten(RockwormEater);
		}
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

simulated function NotifyBeingEaten(GearPawn_RockwormBase Eater)
{
	RockwormEater = Eater;
	GotoState('BeingEaten');	
}


auto state Undisturbed
{
	event TakeDamage(int DamageAmount, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
	{
		LastDT = DamageType;
		LastDamageAmt = DamageAmount;
		LastDamager = EventInstigator;
		GotoState('Disturbed');
	}
}

event Touch( Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal )
{
	local GearPawn_RockWormBase RW;
	RW = GearPawn_RockWormBase(Other);
	if(RW != none)
	{
		RW.ArrivedAtFruit(self);
	}
}

// if we are shot/disturbed break fall from where we are to the ground
simulated state Disturbed
{
	ignores TakeDamage;

	function TriggerDamageEvents()
	{
		local int idx;
		local SeqEvent_TakeDamage dmgEvent;
		// search for any damage events
		for (idx = 0; idx < GeneratedEvents.Length; idx++)
		{
			dmgEvent = SeqEvent_TakeDamage(GeneratedEvents[idx]);
			if (dmgEvent != None)
			{
				// notify the event of the damage received
				//`log(dmgEvent@LastDamager@LastDT@LastDamageAmt);
				dmgEvent.HandleDamage(self, LastDamager, LastDT, dmgEvent.DamageThreshold+1);
			}
		}
	}

	event Landed( vector HitNormal, actor FloorActor )
	{
		TriggerDamageEvents();
	}


Begin:
	//`log("OPEN DISTURBED.. FALLING!");
	SetPhysics(PHYS_Falling);
	DesiredRotation.Pitch = 16384;
}

simulated state BeingEaten
{
	ignores TakeDamage;

Begin:
	FruitEatEmitter = Spawn(class'SpawnedGearEmitter',,,Location,,,true);
	FruitEatEmitter.SetTemplate(FruitEatPS,true);
	FruitEatEmitter.ParticleSystemComponent.ActivateSystem();
	FruitEatEmitter.LifeSpan=RandRange(EatDestroyDelay,EatDestroyDelay+0.25f);
	SetHidden(true);
	Sleep(EatDestroyDelay);
	Destroy();
};

defaultproperties
{
	EatDestroyDelay=3.0f
	bStatic=false
	bMovable=true
	bNoDelete=false

	Begin Object Class=CylinderComponent Name=CollisionCylinder
		CollisionRadius=+0032.000000
		CollisionHeight=+010.000000
		BlockNonZeroExtent=true
		BlockZeroExtent=true
		BlockActors=false
		CollideActors=true
		Translation=(z=10)
	End Object
	Components.Add(CollisionCylinder)
	CollisionComponent=CollisionCylinder
	bPathColliding=false
	bCollideActors=true
	bNoEncroachCheck=true
	bCollideWorld=true
	bProjTarget=true
	Physics=PHYS_None
	RotationRate=(Pitch=32768)
	RemoteRole=ROLE_SimulatedProxy
	bUpdateSimulatedPosition=true
}