/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SkeletalMeshActor extends Actor
	native(Anim)
	placeable;

var()		bool		bDamageAppliesImpulse;

/** whether we need to save this Actor's Rotation in checkpoints */
var(Gears) const bool bCheckpointSaveRotation;

/** Whether or not this actor should respond to anim notifies **/
var() bool bShouldDoAnimNotifies;

var()	SkeletalMeshComponent			SkeletalMeshComponent;
var() const editconst LightEnvironmentComponent LightEnvironment;

var		AudioComponent					FacialAudioComp;

// BM1
var export editinline AudioComponent ImpactSoundComponent;
var export editinline AudioComponent ImpactSoundComponent2;
var float LastImpactTime;

// BM1: This should be RB_ForceComponent
var export editinline PrimitiveComponent ImpactForceComponent;

/** Used to replicate mesh to clients */
var repnotify transient SkeletalMesh ReplicatedMesh;

/** used to replicate the material in index 0 */
var repnotify MaterialInterface ReplicatedMaterial;

struct CheckpointRecord
{
	var bool bHidden;
	var rotator Rotation;
};

/** Struct that stores info to update one skel control with a location target */
struct native SkelMeshActorControlTarget
{
	/** Name of SkelControl to update */
	var()	name	ControlName;
	/** Actor to use for location of skel control target. */
	var()	actor	TargetActor;
};

/** Set of skel controls to update targets of, based on Actor location */
var() array<SkelMeshActorControlTarget>		ControlTargets;

// BM1
var() Actor LookAtTarget;
var() interp float LookAtWeight;
var() export editinline SkeletalMeshComponent HeadMesh;

cpptext
{
	// UObject interface
	virtual void CheckForErrors();

	// AActor interface
	virtual void TickSpecial(FLOAT DeltaSeconds);
	virtual void ForceUpdateComponents(UBOOL bCollisionUpdate,UBOOL bTransformOnly);
	virtual UBOOL InStasis();
	virtual void PreviewBeginAnimControl(TArray<class UAnimSet*>& InAnimSets);
	virtual void PreviewSetAnimPosition(FName SlotName, INT ChannelIndex, FName InAnimSeqName, FLOAT InPosition, UBOOL bLooping);
	virtual void PreviewSetAnimWeights(TArray<FAnimSlotInfo>& SlotInfos);
	virtual void PreviewFinishAnimControl();
	virtual void PreviewUpdateFaceFX(UBOOL bForceAnim, const FString& GroupName, const FString& SeqName, FLOAT InPosition);
	virtual void PreviewActorPlayFaceFX(const FString& GroupName, const FString& SeqName, USoundCue* InSoundCue);
	virtual void PreviewActorStopFaceFX();
	virtual UAudioComponent* PreviewGetFaceFXAudioComponent();
	virtual class UFaceFXAsset* PreviewGetActorFaceFXAsset();

	/** Called each from while the Matinee action is running, to set the animation weights for the actor. */
	virtual void SetAnimWeights( const TArray<struct FAnimSlotInfo>& SlotInfos );

protected:
/**
     * This function actually does the work for the GetDetailInfo and is virtual.
     * It should only be called from GetDetailedInfo as GetDetailedInfo is safe to call on NULL object pointers
     **/
	virtual FString GetDetailedInfoInternal() const;
}


replication
{
	if (Role == ROLE_Authority)
		ReplicatedMesh, ReplicatedMaterial;
}

simulated event PostBeginPlay()
{
	// grab the current mesh for replication
	if (Role == ROLE_Authority && SkeletalMeshComponent != None)
	{
		ReplicatedMesh = SkeletalMeshComponent.SkeletalMesh;
	}

	// Unfix bodies flagged as 'full anim weight'
	if( SkeletalMeshComponent != None &&
		//SkeletalMeshComponent.bEnableFullAnimWeightBodies &&
		SkeletalMeshComponent.PhysicsAssetInstance != None )
	{
		SkeletalMeshComponent.PhysicsAssetInstance.SetFullAnimWeightBonesFixed(FALSE, SkeletalMeshComponent);
	}
}

simulated event ReplicatedEvent( name VarName )
{
	if (VarName == 'ReplicatedMesh')
	{
		SkeletalMeshComponent.SetSkeletalMesh(ReplicatedMesh);
	}
	else if (VarName == 'ReplicatedMaterial')
	{
		SkeletalMeshComponent.SetMaterial(0, ReplicatedMaterial);
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

/** Handling Toggle event from Kismet. */
simulated function OnToggle(SeqAct_Toggle action)
{
	local AnimNodeSequence SeqNode;

	SeqNode = AnimNodeSequence(SkeletalMeshComponent.Animations);

	// Turn ON
	if (action.InputLinks[0].bHasImpulse)
	{
		// If animation is not playing - start playing it now.
		if(!SeqNode.bPlaying)
		{
			// This starts the animation playing from the beginning. Do we always want that?
			SeqNode.PlayAnim(SeqNode.bLooping, SeqNode.Rate, 0.0);
		}
	}
	// Turn OFF
	else if (action.InputLinks[1].bHasImpulse)
	{
		// If animation is playing, stop it now.
		if(SeqNode.bPlaying)
		{
			SeqNode.StopAnim();
		}
	}
	// Toggle
	else if (action.InputLinks[2].bHasImpulse)
	{
		// Toggle current animation state.
		if(SeqNode.bPlaying)
		{
			SeqNode.StopAnim();
		}
		else
		{
			SeqNode.PlayAnim(SeqNode.bLooping, SeqNode.Rate, 0.0);
		}
	}
}

function OnSetMaterial(SeqAct_SetMaterial Action)
{
	SkeletalMeshComponent.SetMaterial( Action.MaterialIndex, Action.NewMaterial );
	if (Action.MaterialIndex == 0)
	{
		ReplicatedMaterial = Action.NewMaterial;
		ForceNetRelevant();
	}
}

simulated event BeginAnimControl(array<AnimSet> InAnimSets)
{
	local AnimNodeSequence SeqNode;
	local int i;

	SeqNode = AnimNodeSequence(SkeletalMeshComponent.Animations);

	// Backup existing AnimSets
	if (SkeletalMeshComponent.TemporarySavedAnimSets.length == 0)
	{
		SkeletalMeshComponent.SaveAnimSets();
		SkeletalMeshComponent.AnimSets.length = 0;
	}

	// add animsets instead of replacing so that multiple matinees acting on the same SkeletalMeshActor
	// won't clobber each other unless they're actively animating this Actor at the same time
	for (i = 0; i < InAnimSets.length; i++)
	{
		if (SkeletalMeshComponent.AnimSets.Find(InAnimSets[i]) == INDEX_NONE)
		{
			SkeletalMeshComponent.AnimSets.AddItem(InAnimSets[i]);
		}
	}
	SeqNode.SetAnim('');
}

simulated event SetAnimPosition(name SlotName, int ChannelIndex, name InAnimSeqName, float InPosition, bool bFireNotifies, bool bLooping)
{
	local AnimNodeSequence	SeqNode;
	/*
	local int				i;
	`log(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "PRE - InPosition:" @ InPosition @ "InAnimSeqName:" @ InAnimSeqName);
	for(i=0; i<SkeletalMeshComponent.AnimSets.Length; i++)
	{
		`log(" -" @ SkeletalMeshComponent.AnimSets[i]);
	}
	*/

	SeqNode = AnimNodeSequence(SkeletalMeshComponent.Animations);
	if( SeqNode != None )
	{
		if( SeqNode.AnimSeqName != InAnimSeqName )
		{
			SeqNode.SetAnim(InAnimSeqName);
		}

		SeqNode.bLooping = bLooping;
		SeqNode.SetPosition(InPosition, bFireNotifies);

		//`log(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "POST - CurrentTime:" @ SeqNode.CurrentTime @ "AnimSeqName:" @ SeqNode.AnimSeqName);
	}
}

simulated event FinishAnimControl()
{
	// Restore previous AnimSets
	//@FIXME: doesn't handle multiple matinees with anim tracks working on the same SkeletalMeshActor
	SkeletalMeshComponent.RestoreSavedAnimSets();
}

/** Handler for Matinee wanting to play FaceFX animations in the game. */
simulated event bool PlayActorFaceFXAnim(FaceFXAnimSet AnimSet, String GroupName, String SeqName, SoundCue SoundCueToPlay )
{
	return SkeletalMeshComponent.PlayFaceFXAnim(AnimSet, SeqName, GroupName, SoundCueToPlay);
}

/** Handler for Matinee wanting to stop FaceFX animations in the game. */
simulated event StopActorFaceFXAnim()
{
	SkeletalMeshComponent.StopFaceFXAnim();
}

/** Used to let FaceFX know what component to play dialogue audio on. */
simulated event AudioComponent GetFaceFXAudioComponent()
{
	return FacialAudioComp;
}

/** Function for handling the SeqAct_PlayFaceFXAnim Kismet action working on this Actor. */
simulated function OnPlayFaceFXAnim(SeqAct_PlayFaceFXAnim inAction)
{
	local PlayerController PC;

	SkeletalMeshComponent.PlayFaceFXAnim(inAction.FaceFXAnimSetRef, inAction.FaceFXAnimName, inAction.FaceFXGroupName, inAction.SoundCueToPlay);

	// tell non-local players to play as well
	foreach WorldInfo.AllControllers(class'PlayerController', PC)
	{
		if (NetConnection(PC.Player) != None)
		{
			PC.ClientPlayActorFaceFXAnim(self, inAction.FaceFXAnimSetRef, inAction.FaceFXGroupName, inAction.FaceFXAnimName, inAction.SoundCueToPlay);
		}
	}
}

/** Used by Matinee in-game to mount FaceFXAnimSets before playing animations. */
simulated event FaceFXAsset GetActorFaceFXAsset()
{
	if(SkeletalMeshComponent.SkeletalMesh != None)
	{
		return SkeletalMeshComponent.SkeletalMesh.FaceFXAsset;
	}
	else
	{
		return None;
	}
}

/**
 * Returns TRUE if this actor is playing a FaceFX anim.
 */
simulated function bool IsActorPlayingFaceFXAnim()
{
	return (SkeletalMeshComponent != None && SkeletalMeshComponent.IsPlayingFaceFXAnim());
}


event OnSetSkeletalMesh(SeqAct_SetSkeletalMesh Action)
{
	if (Action.NewSkeletalMesh != None && Action.NewSkeletalMesh != SkeletalMeshComponent.SkeletalMesh)
	{
		SkeletalMeshComponent.SetSkeletalMesh(Action.NewSkeletalMesh);
		ReplicatedMesh = Action.NewSkeletalMesh;
	}
}

/** Handle action that forces bodies to sync to their animated location */
simulated event OnUpdatePhysBonesFromAnim(SeqAct_UpdatePhysBonesFromAnim Action)
{
	if (action.InputLinks[0].bHasImpulse)
	{
		SkeletalMeshComponent.ForceSkelUpdate();
		SkeletalMeshComponent.UpdateRBBonesFromSpaceBases(TRUE, TRUE);
	}
	else if(action.InputLinks[1].bHasImpulse)
	{
		// Fix bodies flagged as 'full anim weight'
		if( SkeletalMeshComponent.PhysicsAssetInstance != None )
		{
			SkeletalMeshComponent.PhysicsAssetInstance.SetAllBodiesFixed(TRUE);
		}
	}
	else if(action.InputLinks[2].bHasImpulse)
	{
		// Unfix bodies flagged as 'full anim weight'
		if( SkeletalMeshComponent.PhysicsAssetInstance != None )
		{
			SkeletalMeshComponent.PhysicsAssetInstance.SetFullAnimWeightBonesFixed(FALSE, SkeletalMeshComponent);
		}
	}
}

/** Handle action to set skel control target from kismet. */
simulated event OnSetSkelControlTarget(SeqAct_SetSkelControlTarget Action)
{
	local int i;

	// Check we have the info we need
	if(Action.SkelControlName == '' || Action.TargetActors.length == 0)
	{
		return;
	}

	// First see if we have an entry for this control
	for(i=0; i<ControlTargets.length; i++)
	{
		// See if name matches
		if(ControlTargets[i].ControlName == Action.SkelControlName)
		{
			// It does - just update target actor
			ControlTargets[i].TargetActor = Actor(Action.TargetActors[Rand(Action.TargetActors.length)]);
			return;
		}
	}

	// Did not find an existing entry - make a new one
	ControlTargets.length = ControlTargets.length + 1;
	ControlTargets[ControlTargets.length-1].ControlName = Action.SkelControlName;
	ControlTargets[ControlTargets.length-1].TargetActor = Actor(Action.TargetActors[Rand(Action.TargetActors.length)]);
}

/** Performs actual attachment. Can be subclassed for class specific behaviors. */
function DoKismetAttachment(Actor Attachment, SeqAct_AttachToActor Action)
{
	local bool	bOldCollideActors, bOldBlockActors, bValidBone, bValidSocket;

	// If a bone/socket has been specified, see if it is valid
	if( SkeletalMeshComponent != None && Action.BoneName != '' )
	{
		// See if the bone name refers to an existing socket on the skeletal mesh.
		bValidSocket	= (SkeletalMeshComponent.GetSocketByName(Action.BoneName) != None);
		bValidBone		= (SkeletalMeshComponent.MatchRefBone(Action.BoneName) != INDEX_NONE);

		// Issue a warning if we were expecting to attach to a bone/socket, but it could not be found.
		if( !bValidBone && !bValidSocket )
		{
			`log(WorldInfo.TimeSeconds @ class @ GetFuncName() @ "bone or socket" @ Action.BoneName @ "not found on actor" @ Self @ "with mesh" @ SkeletalMeshComponent);
		}
	}

	// Special case for handling relative location/rotation w/ bone or socket
	if( bValidBone || bValidSocket )
	{
		// disable collision, so we can successfully move the attachment
		bOldCollideActors	= Attachment.bCollideActors;
		bOldBlockActors		= Attachment.bBlockActors;
		Attachment.SetCollision(FALSE, FALSE);
		Attachment.SetHardAttach(Action.bHardAttach);

		// Sockets by default move the actor to the socket location.
		// This is not the case for bones!
		// So if we use relative offsets, then first move attachment to bone's location.
		if( bValidBone && !bValidSocket )
		{
			if( Action.bUseRelativeOffset )
			{
				Attachment.SetLocation(SkeletalMeshComponent.GetBoneLocation(Action.BoneName));
			}

			if( Action.bUseRelativeRotation )
			{
				Attachment.SetRotation(QuatToRotator(SkeletalMeshComponent.GetBoneQuaternion(Action.BoneName)));
			}
		}

		// Attach attachment to base.
		Attachment.SetBase(Self,, SkeletalMeshComponent, Action.BoneName);

		if( Action.bUseRelativeRotation )
		{
			Attachment.SetRelativeRotation(Attachment.RelativeRotation + Action.RelativeRotation);
		}

		// if we're using the offset, place attachment relatively to the target
		if( Action.bUseRelativeOffset )
		{
			Attachment.SetRelativeLocation(Attachment.RelativeLocation + Action.RelativeOffset);
		}

		// restore previous collision
		Attachment.SetCollision(bOldCollideActors, bOldBlockActors);
	}
	else
	{
		// otherwise base on location
		Super.DoKismetAttachment(Attachment, Action);
	}
}

/**
* Default behaviour when shot is to apply an impulse and kick the KActor.
*/
event TakeDamage(int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	local vector ApplyImpulse;

	// call Actor's version to handle any SeqEvent_TakeDamage for scripting
	Super.TakeDamage(Damage, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, DamageCauser);

	if ( bDamageAppliesImpulse && damageType.default.KDamageImpulse > 0 )
	{
		if ( VSize(momentum) < 0.001 )
		{
			`Log("Zero momentum to SkeletalMeshActor.TakeDamage");
			return;
		}

		ApplyImpulse = Normal(momentum) * damageType.default.KDamageImpulse;
		if ( HitInfo.HitComponent != None )
		{
			HitInfo.HitComponent.AddImpulse(ApplyImpulse, HitLocation, HitInfo.BoneName);
		}
	}
}

// checkpointing
function bool ShouldSaveForCheckpoint()
{
	return (RemoteRole != ROLE_None || bCheckpointSaveRotation);
}
function CreateCheckpointRecord(out CheckpointRecord Record)
{
	Record.bHidden = bHidden;
	if (bCheckpointSaveRotation)
	{
		Record.Rotation = Rotation;
	}
}
function ApplyCheckpointRecord(const out CheckpointRecord Record)
{
	SetHidden(Record.bHidden);
	if (bCheckpointSaveRotation)
	{
		SetRotation(Record.Rotation);
	}
	ForceNetRelevant();
	if (RemoteRole != ROLE_None)
	{
		SetForcedInitialReplicatedProperty(Property'Engine.Actor.bHidden', (bHidden == default.bHidden));
	}
}


/**
 * Called by AnimNotify_PlayParticleEffect
 * Looks for a socket name first then bone name
 *
 * @param AnimNotifyData The AnimNotify_PlayParticleEffect which will have all of the various params on it
 */
event PlayParticleEffect( const AnimNotify_PlayParticleEffect AnimNotifyData )
{
	local vector Loc;
	local rotator Rot;
	local ParticleSystemComponent PSC;

	// if we should not respond to anim notifies OR if this is extreme content and we can't show extreme content then return
	if( ( bShouldDoAnimNotifies == FALSE )
		|| ( ( AnimNotifyData.bIsExtremeContent == TRUE ) && ( WorldInfo.GRI.ShouldShowGore() == FALSE ) )
		)
	{
		return;
	}

	// find the location
	if( AnimNotifyData.SocketName != '' )
	{
		SkeletalMeshComponent.GetSocketWorldLocationAndRotation( AnimNotifyData.SocketName, Loc, Rot );
	}
	else if( AnimNotifyData.BoneName != '' )
	{
		Loc = SkeletalMeshComponent.GetBoneLocation( AnimNotifyData.BoneName );
	}
	else
	{
		Loc = Location;
	}

	// now go ahead and spawn the particle system based on whether we need to attach it or not
	if( AnimNotifyData.bAttach == TRUE )
	{
		PSC = new(self) class'ParticleSystemComponent';  // move this to the object pool once it can support attached to bone/socket and relative translation/rotation
		PSC.SetTemplate( AnimNotifyData.PSTemplate );

		if( AnimNotifyData.SocketName != '' )
		{
			//`log( "attaching AnimNotifyData.SocketName" );
			SkeletalMeshComponent.AttachComponentToSocket( PSC, AnimNotifyData.SocketName );
		}
		else if( AnimNotifyData.BoneName != '' )
		{
			//`log( "attaching AnimNotifyData.BoneName" );
			SkeletalMeshComponent.AttachComponent( PSC, AnimNotifyData.BoneName );
		}

		PSC.ActivateSystem();
		PSC.OnSystemFinished = SkelMeshActorOnParticleSystemFinished;
	}
	else
	{
		WorldInfo.MyEmitterPool.SpawnEmitter( AnimNotifyData.PSTemplate, Loc, rot(0,0,1) );
	}
}


/** We so we detach the Component once we are done playing it **/
simulated function SkelMeshActorOnParticleSystemFinished( ParticleSystemComponent PSC )
{
	SkeletalMeshComponent.DetachComponent( PSC );
}



defaultproperties
{
	Begin Object Class=AnimNodeSequence Name=AnimNodeSeq0
	End Object

	Begin Object Class=DynamicLightEnvironmentComponent Name=MyLightEnvironment
		bEnabled=FALSE
		bSynthesizeSHLight=TRUE
		TickGroup=TG_DuringAsyncWork
	End Object
	Components.Add(MyLightEnvironment)
	LightEnvironment=MyLightEnvironment

	Begin Object Class=SkeletalMeshComponent Name=SkeletalMeshComponent0
		Animations=AnimNodeSeq0
		bUpdateSkelWhenNotRendered=FALSE
		CollideActors=TRUE
		BlockActors=FALSE
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=FALSE
		LightEnvironment=MyLightEnvironment
		RBChannel=RBCC_GameplayPhysics
		RBCollideWithChannels=(Default=TRUE,BlockingVolume=TRUE,GameplayPhysics=TRUE,EffectPhysics=TRUE)
	bForceMeshObjectUpdates=TRUE
	End Object
	CollisionComponent=SkeletalMeshComponent0
	SkeletalMeshComponent=SkeletalMeshComponent0
	Components.Add(SkeletalMeshComponent0)

	Begin Object Class=AudioComponent Name=FaceAudioComponent
	End Object
	FacialAudioComp=FaceAudioComponent
	Components.Add(FaceAudioComponent)

	Physics=PHYS_None
	bEdShouldSnap=TRUE
	bStatic=FALSE
	bCollideActors=TRUE
	bBlockActors=FALSE
	bWorldGeometry=FALSE
	bCollideWorld=FALSE
	bNoEncroachCheck=TRUE
	bProjTarget=TRUE
	bUpdateSimulatedPosition=FALSE
	bStasis=TRUE

	RemoteRole=ROLE_None
	bNoDelete=TRUE

	bShouldDoAnimNotifies=FALSE
}
