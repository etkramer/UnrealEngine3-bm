/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GDO_FlameDroneTank extends GDO_PropaneTank
	config(Weapon);

var Controller LastDamager;
var vector	   LastDamageLoc;
var GearPawn OwningGearPawn;
var() config float ExplosionDelay;
var() config float Health;
var class<AICommand> CommandClassToPush;
var AIAvoidanceCylinderComponent CreatedComponent;
var() ParticleSystem SmokePS;
var ParticleSystemComponent SmokePSComp;
var() ParticleSystem LeftPersistentFlamePS,RightPersistentFlamePS;

/** Expose the FlameTankMesh so we can set the ShadowParent on it. **/
var StaticMeshComponent FlameTankMesh;


function Init(GearPawn InOwner)
{
	OwningGearPawn = InOwner;
	SubObjects[0].DefaultHealth = Health;
	SubObjectHealths[0] = Health;

	if( SmokePS != none )
	{
		SmokePSComp = new(self) class'ParticleSystemComponent';
		if(SmokePSComp != none)
		{
			SmokePSComp.SetTemplate( SmokePS );
			AttachComponent(SmokePSComp);
			SmokePSComp.SetHidden( FALSE );
			SmokePSComp.ActivateSystem();
		}
	}

}
event TakeDamage(int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	if(EventInstigator == OwningGearPawn.Controller || (EventInstigator != None && EventInstigator.GetTeamNum() == OwningGearPawn.GetTeamNum()))
	{
		return;
	}
	Super.TakeDamage(Damage,EventInstigator,HitLocation,Momentum,DamageType,HitInfo,DamageCauser);
	LastDamager = EventInstigator;
	LastDamageLoc = HitLocation;
}

simulated protected function Explode()
{
	local GearExplosionActor ExplosionActor;
	local GearPRI InstigatorPRI, MyPRI;

	// spawn explosion
	ExplosionActor = Spawn(class'GearExplosionActor', OwningGearPawn,, Location, rot(16384,0,0));
	ExplosionActor.Instigator=LastDamager.Pawn;
	ExplosionActor.InstigatorController = LastDamager;
	ExplosionTemplate.Damage = ExploBaseDamage;
	ExplosionTemplate.DamageRadius = ExploDamageRadius;
	ExplosionTemplate.DamageFalloffExponent = ExploDamageFalloff;
	ExplosionTemplate.ExploShakeOuterRadius = ExploDamageRadius;
	ExplosionActor.Explode(ExplosionTemplate);
	if(CreatedComponent!=none)
	{
		OwningGearPawn.DetachComponent(CreatedComponent);
	}

	// make sure our owner dies
	if(OwningGearPawn != none)
	{
		// chalk one up for the instigator
		if(LastDamager != none)
		{
			InstigatorPRI = GearPRI(LastDamager.PlayerReplicationInfo);
			MyPRI = GearPRI(OwningGearPawn.COntroller.PlayerReplicationInfo);
			if (InstigatorPRI != None && MyPRI != none)
			{
				// track the kill for the instigator
				InstigatorPRI.ScoreInstantKill(MyPRI, class'GDT_FLameTankExplosion', GDT_NORMAL);
			}
		}
		OwningGearPawn.Died(LastDamager,class'GDT_FlameTankExplosion',Location);
	}

	if(SmokePSComp != none)
	{
		SmokePSComp.DeactivateSystem();
		DetachComponent(SmokePSComp);
		SmokePSComp = none;
	}
}

function SetupAvoidanceCylinder()
{
	if(OwningGearPawn != none)
	{
		CreatedComponent = new(OwningGearPawn) class'AIAvoidanceCylinderComponent';
		CreatedComponent.SetCylinderSize(ExploDamageRadius*0.75f,ExploDamageRadius*0.75f);
		CreatedComponent.SetActorCollision(TRUE,FALSE,TRUE);
		OwningGearPawn.AttachComponent(CreatedComponent);


	}
}

function TellAIToFreakOut()
{
	if(OwningGearPawn != none && OwningGearPawn.Controller != none)
	{
		CommandClassToPush.static.InitCommandUserActor(GearAI(OwningGearPawn.Controller),LastDamager);
	}
}

simulated function bool CanBeSpecialMeleeAttacked( GearPawn Attacker )
{
	return FALSE;
}

simulated state ExploSequence
{

	simulated event BeginState(name PreviousStateName)
	{
		local vector left;
		local vector hittotank;

		// catch fire
		if (Role == Role_Authority)
		{
			bExploded = TRUE;
			bForceNetUpdate = TRUE;
		}

		// determine which persistent flame to play
		left = vector(rotation);
		hittotank = Normal( (LastDamageLoc - Location) * vect(1.0f,1.0f,0) );
		if(hittotank dot left >= 0.f)
		{
			PS_PersistentFlame.SetTemplate(LeftPersistentFlamePS);
		}
		else
		{

			PS_PersistentFlame.SetTemplate(RightPersistentFlamePS);
		}

		PS_PersistentFlame.ActivateSystem();

		PlaySound(SoundCue'Weapon_Grenade.Weapons.PropaneGasIgniteCue', true);

		OwningGearPawn.FlameTankAboutToBlow();
	}

Begin:

	TellAIToFreakOut();
	SetupAvoidanceCylinder();
	Sleep(ExplosionDelay);

	if(FireCracklingSound == none)
	{
		FireCracklingSound = CreateAudioComponent(SoundCue'Weapon_Grenade.Weapons.PropaneGasFireCue',false,true);
		FireCracklingSound.bUseOwnerLocation = true;
		FireCracklingSound.bAutoDestroy = true;
		FireCracklingSound.Location = location;
		AttachComponent(FireCracklingSound);
	}
	if(FireCracklingSound != none)
	{
		FireCracklingSound.FadeIn(0.2f,1.0f);
	}

	OwningGearPawn.IgnitePawn(POF_Blazing,0.f);
	Explode();
	Sleep(0.0); // give a tick to replicate
	Destroy();
};

simulated state ExploSequenceExplosiveDamageType
{
Begin:
	Explode();
	Sleep(0.0); // give a tick to replicate
	Destroy();
}

DefaultProperties
{
	CommandClassToPush=class'AICmd_React_ImOnFire'
	bStatic=false
	bNoDelete=false
	bMovable=true

	bBlockActors=false

	LeftPersistentFlamePS=ParticleSystem'Effects_Gameplay.Fire.P_TankFire_Left'
	RightPersistentFlamePS=ParticleSystem'Effects_Gameplay.Fire.P_TankFire_Right'
	Begin Object Name=PersistentFlame
		Template=none
		Translation=(Z=0)
	End Object
	Components.Add(PersistentFlame)

	Begin Object Name=StaticMeshComponent_Tank
		StaticMesh=StaticMesh'Locust_Hunter.flamethrower_tanks'
		BlockNonZeroExtent=false
	End Object
	FlameTankMesh=StaticMeshComponent_Tank

	Begin Object Name=PreFire
		Template=none
	End Object

	// explosion
	Begin Object Name=ExploTemplate0
		MyDamageType=class'GDT_FlameTankExplosion'
		ParticleEmitterTemplate=ParticleSystem'Effects_Gameplay.Explosions.P_Flamer_Backpack_Explo'
		ExplosionSound=SoundCue'Weapon_Grenade.Weapons.PropaneExplosion01Cue'
	End Object

	SubObjects(0)=(DefaultHealth=350.000000)
	SubObjectHealths(0)=350.000000
	bWorldGeometry=false
}
