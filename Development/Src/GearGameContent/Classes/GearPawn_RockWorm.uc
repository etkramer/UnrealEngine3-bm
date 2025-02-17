/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_RockWorm extends GearPawn_RockWormBase
	implements(AIobjectAvoidanceInterface);

var() SoundCue TailShotSound;
var() SoundCue BiteGibSound;
var() SoundCue EatMyFruitSound;
var() SoundCue ISawThatFruitSound;
var() SoundCue AmbientSound;

/** sounds to play when moving.  Play 0 at head, and increment sound to play as we move down the tail */
var array<SoundCue> MovementSounds;
var array<AudioComponent> MovementAudioComps;

var() vector2d PitchRange;


var ParticleSystem MouthPS;
var ParticleSystem DustPS;
var name MouthPSSocketName;
var name DustPSSocketName;
var ParticleSystemComponent MouthPSC;
var ParticleSystemComponent DustPSC;
var AIAvoidanceCylinder AvoidanceCylinder;

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	// timer for ambient sound loop
	SetTimer(RandRange(7,12),FALSE,nameof(PlayAmbient));


	DustPSC = new(Self) class'ParticleSystemComponent';
	DustPSC.SetTemplate(DustPS);
	Mesh.AttachComponentToSocket(DustPSC,DustPSSocketName);

	// timer for movement sounds and dust effects
	SetTimer(0.25,TRUE,nameof(PlayMovement));

	MouthPSC = new(self) class'ParticleSystemComponent';
	MouthPSC.SetTemplate(MouthPS);
	Mesh.AttachComponentToSocket(MouthPSC,MouthPSSocketName);
	MouthPSC.Activatesystem();

	SetupAvoidanceCylinder();
}


function SetupAvoidanceCylinder()
{
	local float AvoidRadius;

	if( !bDeleteMe && AvoidanceCylinder == None )
	{
		AvoidRadius = GetCollisionRadius() * 2.5f;
		AvoidanceCylinder = Spawn(class'AIAvoidanceCylinder',self,,Location,,,TRUE);
		if(AvoidanceCylinder!=none)
		{
			
			AvoidanceCylinder.SetBase(self);
			AvoidanceCylinder.SetRelativeLocation(vect(50.f,0.f,0.f));
			AvoidanceCylinder.SetCylinderSize(AvoidRadius,AvoidRadius);
			AvoidanceCylinder.SetAvoidanceTeam(TEAM_EVERYONE);
			AvoidanceCylinder.SetEnabled(true);
		}
		//DrawDebugSphere(Location,AvoidRadius,16,255,0,0,TRUE);
	}

}

function bool ShouldAvoid(GearAI AskingAI, AIAvoidanceCylinderComponent TriggeringComponent)
{
	if(AskingAI == Controller)
	{
		return false;
	}
	return true;
}
function bool ShouldEvade(GearAI AskingAI, AIAvoidanceCylinderComponent TriggeringComponent);
function bool ShouldRoadieRun(GearAI AskingAI, AIAvoidanceCylinderComponent TriggeringComponent);

simulated function Destroyed()
{
	if(AvoidanceCylinder != none)
	{
		AvoidanceCylinder.Destroy();
		AvoidanceCylinder = none;
	}
	Super.Destroyed();
}


simulated function PlayAmbient()
{
	PlaySound(AmbientSound,TRUE,,TRUE);
	SetTimer(RandRange(7,12),FALSE,nameof(PlayAmbient));
}

simulated function PlayMovement()
{
	local int Idx;
	local int Count;
	local int Increment;
	local Rockworm_TailSegment CurSeg;
	local float VelSq;

	VelSq = VSizeSq(Velocity);
	// movement dust
	if(VelSq > 100.f)
	{
		if(!DustPSC.bIsActive)
		{
			DustPSC.ActivateSystem();
		}
	}
	else
	{
		if(DustPSC.bIsActive)
		{
			DustPSC.DeactivateSystem();
		}
	}
	
	// movement sounds
	Increment = (NUM_TAILSEGMENTS+1)/MovementSounds.length;
	CurSeg = TailSegmentList;
	Count=0;
	Idx=0;
	
	while(CurSeg != none)
	{

		if(Count % Increment == 0 && MovementSounds[Idx] != none)
		{
			if(MovementAudioComps.length <= Idx || MovementAudioComps[Idx] == none)
			{				
				MovementAudioComps[Idx] = CurSeg.CreateAudioComponent(MovementSounds[Idx],TRUE,TRUE,TRUE,,TRUE);
				//`log("Creating component at "$Idx$": "$MovementAudioComps[Idx]);
				MovementAudioComps[Idx].bUseOwnerLocation=true;
				MovementAudioComps[Idx].bShouldRemainActiveIfDropped=true;
				CurSeg.AttachComponent(MovementAudioComps[Idx]);
			}

			if(MovementAudioComps[Idx] != none)
			{
				if(!MovementAudioComps[Idx].IsPlaying() && VelSq > 16.f * 16.f)
				{
					//`log("Fading in comp "@Idx);
					MovementAudioComps[Idx].FadeIn(0.5f,1.f);
				}
				else if(MovementAudioComps[Idx].IsPlaying() && VelSq < 16.f * 16.f)
				{
					//`log("Fading out comp "@Idx);
					MovementAudioComps[Idx].FadeOut(0.24f,0.f);
				}
			}
			Idx++;

		}

		CurSeg = CurSeg.NextSegment;
		Count++;
	}

}

function PlaySecondAttackSound()
{
	PlaySound(BiteGibSound);
}

simulated event Tick(float DeltaTime)
{
	local int Idx;
	local float PctMaxSpeed, PitchAdd;
	Super.Tick(DeltaTime);

	// loop through movement audio components and pitch shift them based on our speed
	PctMaxSpeed = VSize(Velocity) / DefaultGroundSpeed;
	PitchAdd = Lerp(PitchRange.X,PitchRange.Y,PctMaxSpeed);

	for(Idx=0;Idx<MovementAudioComps.length;Idx++)
	{
		if(MovementAudioComps[Idx] != none)
		{
			MovementAudioComps[Idx].PitchMultiplier = 1.f + PitchAdd;
		}
	}
}


/*
var bool bDebugCover;

simulated event Tick(float DeltaTime)
{
	local CoverSlot Slot;
	local int idx;
	local PlayerController PC;
	local vector prevloc;
	Super.Tick(DeltaTime);

	idx=0;
	if(bDebugCover)
	{
		foreach LocalPlayerControllers(class'PlayerController', PC)
		{
			break;
		}

		foreach LeftCover.Slots(Slot)
		{
			PC.RemoveDebugText(Slot.SlotMarker);
			PC.AddDebugText(string(idx), Slot.SlotMarker);
			if(prevloc != vect(0,0,0))
			{
				DrawDebugLine(prevloc,LeftCover.GetSlotLocation(idx),255,0,0);
			}
			prevloc = LeftCover.GetSlotLocation(idx);
			if(Slot.SlotMarker.Owner != none)
			{
				DrawDebugLine(prevloc,Slot.SlotMarker.Owner.Location,255,0,0);
			}
			idx++;
		}
		idx=0;
		prevloc=vect(0,0,0);

		foreach RightCover.Slots(Slot)
		{
			PC.RemoveDebugText(Slot.SlotMarker);
			PC.AddDebugText(string(idx), Slot.SlotMarker);
			if(prevloc != vect(0,0,0))
			{
				DrawDebugLine(prevloc,RightCOver.GetSlotLocation(idx),255,0,0);
			}
			prevloc = RightCover.GetSlotLocation(idx);
			if(Slot.SlotMarker.Owner != none)
			{
				DrawDebugLine(prevloc,Slot.SlotMarker.Owner.Location,255,0,0);
			}
			idx++;
		}


	}
}
*/
simulated function PlayDying(class<DamageType> DamageType, vector HitLoc)
{
	Destroy();
}

function OuchYouShotMyBum()
{
	PlaySound(TailShotSound);
	Super.OuchYouShotMyBum();
}


function MovingToFruit(Rockworm_FruitBase Fruit)
{
	PlaySound(ISawThatFruitSound);
}

function ArrivedAtFruit(Rockworm_FruitBase Fruit)
{
	Fruit.NotifyBeingEaten(self);
	PlaySound(EatMyFruitSound);
	GearAI_Rockworm(MyGearAI).ArrivedAtFruit();
	DoSpecialMove(SM_PushObject,TRUE);
}

defaultproperties
{
	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'Locust_Rockworm.Mesh.Rockworm_F_Head'
		PhysicsAsset=PhysicsAsset'Locust_Rockworm.Rockworm_F_Head_Physics'
		AnimTreeTemplate=AnimTree'Locust_Rockworm.AnimTree.Rockworm_AnimTree'
		AnimSets(0)=AnimSet'Locust_Rockworm.Anims.Animset_Rockworm_Head'
		Translation=(X=-75,z=-50)
		bAcceptsStaticDecals=FALSE
		bAcceptsDynamicDecals=FALSE
		Rotation=(Yaw=-16384)
	End Object

	Begin Object Name=CollisionCylinder
		CollisionHeight=+0048.000000
		CollisionRadius=+0050.000000
	End Object


	TailSegmentClass=class'RockWorm_TailSegmentSM'
	
	SightBoneName=none
	PelvisBoneName=none
	RearPivotBoneName=b_Head_RearPivot
	bRespondToExplosions=false
	bTranslateMeshByCollisionHeight=false


	AttackSound=SoundCue'Locust_Rockworm_Efforts.rockworm.Rockworm_BiteVocalCue'
	TailShotSound=SoundCue'Locust_Rockworm_Efforts.rockworm.Rockworm_TailShotCue'
	BiteGibSound=SoundCue'Locust_Rockworm_Efforts.rockworm.Rockworm_BiteBodyCue'
	EatMyFruitSound=SoundCue'Locust_Rockworm_Efforts.rockworm.Rockworm_FruitEatCue'
	ISawThatFruitSound=SoundCue'Locust_Rockworm_Efforts.rockworm.Rockworm_FruitSeeCue'
	AmbientSound=SoundCue'Locust_Rockworm_Efforts.rockworm.Rockworm_AmbientCue'

	MovementSounds(0)=SoundCue'Locust_Rockworm_Efforts.rockworm.Rockworm_MoveLoop01Cue'
	MovementSounds(1)=SoundCue'Locust_Rockworm_Efforts.rockworm.Rockworm_MoveLoop02Cue'
	//MovementSounds(2)=SoundCue'Locust_Rockworm_Efforts.rockworm.Rockworm_MoveLoop03Cue'
	//MovementSounds(3)=SoundCue'Locust_Rockworm_Efforts.rockworm.Rockworm_MoveLoop04Cue'

	PitchRange=(X=-0.25,Y=0.25)

	MouthPSSocketName=slime
	MouthPS=ParticleSystem'Locust_Rockworm.Effects.P_Rockworm_Mouth_goo'

	DustPSSocketName=Dust
	DustPS=ParticleSystem'Locust_Rockworm.Effects.P_Rockworm_Ground_Dust'
}