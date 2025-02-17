/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Hydra_Base extends Actor
	native(Vehicle)
	abstract;

/** Actual uber-reaver mesh */
var()	SkeletalMeshComponent						Mesh;
/** Skorge mesh */
var()	SkeletalMeshComponent						RiderMesh;

var()	const editconst LightEnvironmentComponent	LightEnvironment;

enum EHydraTentacle
{
	EHT_FrontLeft,
	EHT_BackLeft,
	EHT_FrontRight,
	EHT_BackRight
};

enum EHydraDamagedPart
{
	EHDP_Mouth,
	EHDP_Turret,
	EHDP_Butt,
	EHDP_FrontLeftTentacle,
	EHDP_BackLeftTentacle,
	EHDP_FrontRightTentacle,
	EHDP_BackRightTentacle
};

// Look up using EHydraDamagedPart
var()	name	PartShootBody[EHydraDamagedPart.EnumCount];
var		int		PartHealth[EHydraDamagedPart.EnumCount];
var		byte	bPartAllowDamage[EHydraDamagedPart.EnumCount];
var		Array<Name>	GoreBreakableJoints;

/** Names of gore bones from GoreNode.GoreSetup so they can be toggled on/off */
var		array<Name>	HelmetGoreBoneName, MouthGoreBoneName, ButtGoreBoneName;


var		ParticleSystemComponent	PSC_TargetLaser[2];

/** Actor that Turret will aim at */
var()	Actor	AimAtActor;

var		float	FireOffsetZ;

/** Bone controllers for big turret */
var		GearSkelCtrl_TurretConstrained	TurretControl;

/** Node that blends in chomping anim */
var		GearAnim_BlendPerBone			ChompBlendNode;
/** Blend list for head actions */
var		GearAnim_BlendList				HeadBlendList;
/** Blend between turret up and down */
var		AnimNodeBlend					TurretBlendNode;
/** Node for blending in root-moving flying anim */
var		AnimNodeBlend					FlyAnimBlend;
/** Node that is used for playing main bone overall motion */
var		AnimNodeSequence				MainBoneAnimNode;
/** Node that plays random animations on the legs */
var		AnimNodeBlendList				RandomLegAnimNode;

/** Slot node for playing full-body anims (eg pain) */
var		GearAnim_Slot					FullBodySlot;
/** Used to blen grabbing anim onto specific tentacle */
var		GearAnim_BlendPerBone			GrabMaskNode;
/** Node used to play animation */
var		AnimNodeSequence				GrabAnimNode;

/** Node for controlling cosmetic gore falling off uber reaver */
var		GearAnim_GoreSystem				GoreNode;

/** Indicates if we are currently chomping or not. */
var		repnotify bool					bJawChomping;

/** Indicates if we are currently roaring or not. */
var		repnotify bool					bRoaring;

/** Indicates if we are currently lowering head or not. */
var		repnotify bool					bLowerHead;

/** Indicates if we turret is currently in raised position or not */
var		repnotify bool					bTurretLowered;

/** Used to replicate playing 'damage' anim */
var		repnotify int					DamageAnimCount;

/** Used to replicate restarting the main bone anim, to keep it in sync between client and server */
var		repnotify int					RestartMainBoneAnimCount;
/** Used to catch when animation  */
var		transient float					LastMainBoneAnimPos;

/** If TRUE, no not blend in motion onto root bone */
var		repnotify bool					bDisableRootAnim;

/** If TRUE, scale away turret */
var()	bool							bHideTurret;

// FL, BL, FR, BR (like enum above)

/** Trail controls for tentacles */
var		SkelControlTrail				TrailControls[4];
/** IK controls for tentacles */
var		GearSkelCtrl_CCD_IK				IKControls[4];
/** Single-bone controls for tentacles */
var		SkelControlSingleBone			SBControl[4];

/** Bones to hide when claws blown off */
var()	name							HideClawBoneNames[4];
/** Used to hide a tentacles */
var		repnotify byte					bHideClaw[4];
/** Used to see which tentacle has changed */
var		byte							bOldHideClaw[4];
/** Used to time when to totally hide/destroy falling claw */
var		float							ClawHideCountdown[4];
/** How long between detachment and hiding claw */
var()	float							ClawHideTime;

var(TargetLaser)	float		LaserLength;
var(TargetLaser)	vector		LaserNoLockColor;
var(TargetLaser)	vector		LaserLockColor;


/** Additional 'fake' velocity for tentacles. */
var()	vector					TentacleFakeVelocity;
/** Additional 'fake' velocity for cosmetic gore pieces */
var()	vector					GoreFakeVelocity;

enum EHydraLaserMode
{
	EHLM_Off,
	EHLM_NoLock,
	EHLM_LockedOn
};

var	repnotify EHydraLaserMode	LaserMode;

var	ParticleSystemComponent PSC_Spittle;

/************************************************************************/
// GRABBING

/** Replicated list of things that tentacles are grabbing */
var	repnotify Actor	GrabActor[4];
/** Use on client to see what changed */
var	transient Actor OldGrabActor[4];

/** tentacle currently being grabbed */
var	int	GrabTentacleIndex;

/** How long to delay enabling IK when grabbing */
var()	float	GrabIKDelay;
/** How long to blend IK in over */
var()	float	GrabIKBlendTime;

/** How long to delay turning on single-bone control to set claw rotation */
var()	float	GrabSBDelay;
/** How long to blend in single-bone controller over */
var()	float	GrabSBBlendTime;
/** How strongly to blend in single-bone control to force location/rotation */
var()	float	GrabSBStrength;

/** How long to delay blending out ungrab animation after release */
var()	float	BlendTentacleDelay;
/** How long to take to blend out release anim */
var()	float	BlendTentacleTime;

/** How long to delay turning on trail controls after ungrab */
var()	float	TrailTentacleDelay;
/** How long to blend on trail control */
var()	float	TrailBlendTime;

/** If set, try and find socket on actor and attach claw to that. */
var()	name	GrabActorSocketName[4];
/** Location of claw relative to thing you are grabbing, for each claw */
var()	vector	GrabActorOffset[4];
/** Rotation of claw relatve to thing you are grabbing, for each claw */
var()	rotator	GrabActorRotOffset[4];

/** Last time slow down anim was played */
var		float	LastPlaySlowDownAnimTime;
/** Old speed to track sudden decelerations */
var		float	OldSpeed;


// Sounds
/** Hydra lunges forwards toward reaver in attack, or other abrupt acceleration */
var		SoundCue		SuddenAccelSound;
/** Ambient vocal sounds to trigger every once in a while adding life\movement */
var		SoundCue		AmbientVocalSound;
/** Main flying loop, should probably have a slight amount of pitch variation tied to  velocity */
var		AudioComponent	FlyingLoopSound;
/** Main pain sound when receiving damage */
var		SoundCue		PainVocalSound;
/** Last time pain sound was played */
var		float			LastPainSoundTime;

/** tells client which gore nodes are enabled */
var repnotify byte bGoreNodesEnabled[40];

var()		float			HydraGravZScale;

/** If TRUE, shooting hydra causes pieces to fall off */
var()		bool			bEnableShootingGore;

replication
{
	if (bNetDirty)
		LaserMode, AimAtActor, bJawChomping, bLowerHead, bRoaring, bTurretLowered, DamageAnimCount,
		bHideClaw, bDisableRootAnim, GrabActor, RestartMainBoneAnimCount, bGoreNodesEnabled,
		bPartAllowDamage, PartHealth;
}

cpptext
{
	virtual FLOAT GetGravityZ();
	virtual void TickSpecial(FLOAT DeltaSeconds);
};

/** (client + server) */
simulated event PostBeginPlay()
{
	local int i, NumMaterials;

	Super.PostBeginPlay();

	// Create MICs for each section, so gore system can modify them
	NumMaterials = Mesh.SkeletalMesh.Materials.length;
	for(i=0; i<NumMaterials; i++)
	{
		Mesh.CreateAndSetMaterialInstanceConstant(i);
	}

	// Attach skorge mesh, depending on whether turret is visible
	if(bHideTurret)
	{
		Mesh.AttachComponentToSocket(RiderMesh, 'GunnerNoTurret' );
	}
	else
	{
		Mesh.AttachComponentToSocket(RiderMesh, 'GunnerSeat' );
	}

	// Attach aiming beams
	Mesh.AttachComponentToSocket(PSC_TargetLaser[0], 'UR_Minigun_Barrel_Lt');
	Mesh.AttachComponentToSocket(PSC_TargetLaser[1], 'UR_Minigun_Barrel_Rt');

	// Attach spittle
	Mesh.AttachComponentToSocket(PSC_Spittle, 'Spittle');

	// If not server - make sure hydra is up to date
	if(Role != ROLE_Authority)
	{
		LaserModeChanged();
		PlayChompingAnim( TRUE );
		TurretLoweredChanged();
		ClientPlayDamageAnim();
		HideClawStateChanged();
		RootAnimStateChanged();
		GrabActorArrayChanged();
	}

	SetTimer( 0.5f, FALSE, nameof(PlayAmbientVocalSound) );
}

/** Grab controls we want to change (client + server) */
simulated event PostInitAnimTree(SkeletalMeshComponent SkelComp)
{
	local int TentacleIdx, i;
	local GearAnim_GoreSystem Node;

	Super.PostInitAnimTree(SkelComp);

	if(SkelComp == Mesh)
	{
		ChompBlendNode = GearAnim_BlendPerBone(Mesh.FindAnimNode('ChompBlend'));
		GrabMaskNode = GearAnim_BlendPerBone(Mesh.FindAnimNode('GrabMask'));
		GrabAnimNode = AnimNodeSequence(Mesh.FindAnimNode('GrabAnim'));
		HeadBlendList = GearAnim_BlendList(Mesh.FindAnimNode('HeadBlendList'));

		TurretBlendNode = AnimNodeBlend(Mesh.FindAnimNode('TurretBlend'));
		FullBodySlot = GearAnim_Slot(Mesh.FindAnimNode('FullBodySlot'));
		FlyAnimBlend = AnimNodeBlend(Mesh.FindAnimNode('FlyAnimBlend'));
		MainBoneAnimNode = AnimNodeSequence(Mesh.FindAnimNode('MainBoneAnim'));
		RandomLegAnimNode = AnimNodeBlendList(Mesh.FindAnimNode('RandomLegAnimNode'));

		TurretControl = GearSkelCtrl_TurretConstrained(Mesh.FindSkelControl('Turret_Yaw'));

		// If desired, use this control to scale away the turret
		if(bHideTurret)
		{
			TurretControl.BoneScale = 0.001;
		}

		TrailControls[0] = SkelControlTrail(Mesh.FindSkelControl('Trail_FL'));
		TrailControls[1] = SkelControlTrail(Mesh.FindSkelControl('Trail_BL'));
		TrailControls[2] = SkelControlTrail(Mesh.FindSkelControl('Trail_FR'));
		TrailControls[3] = SkelControlTrail(Mesh.FindSkelControl('Trail_BR'));

		IKControls[0] = GearSkelCtrl_CCD_IK(Mesh.FindSkelControl('IK_FL'));
		IKControls[1] = GearSkelCtrl_CCD_IK(Mesh.FindSkelControl('IK_BL'));
		IKControls[2] = GearSkelCtrl_CCD_IK(Mesh.FindSkelControl('IK_FR'));
		IKControls[3] = GearSkelCtrl_CCD_IK(Mesh.FindSkelControl('IK_BR'));

		SBControl[0] = SkelControlSingleBone(Mesh.FindSkelControl('SB_FL'));
		SBControl[1] = SkelControlSingleBone(Mesh.FindSkelControl('SB_BL'));
		SBControl[2] = SkelControlSingleBone(Mesh.FindSkelControl('SB_FR'));
		SBControl[3] = SkelControlSingleBone(Mesh.FindSkelControl('SB_BR'));

		// Turn on trail controls and off ik and single-bone
		for(TentacleIdx=0; TentacleIdx<4; TentacleIdx++)
		{
			TrailControls[TentacleIdx].BlendType = ABT_Cubic;
			TrailControls[TentacleIdx].SetSkelControlActive(TRUE);
			TrailControls[TentacleIdx].bActorSpaceFakeVel = TRUE;
			TrailControls[TentacleIdx].FakeVelocity = TentacleFakeVelocity;

			IKControls[TentacleIdx].BlendType = ABT_Cubic;
			IKControls[TentacleIdx].SetSkelControlActive(FALSE);

			SBControl[TentacleIdx].BlendType = ABT_Cubic;
			SBControl[TentacleIdx].SetSkelControlActive(FALSE);
		}

		// Iterate over all GearAnim_GoreSystem - but should only have one
		foreach Mesh.AllAnimNodes(class'GearAnim_GoreSystem', Node)
		{
			if(GoreNode == None)
			{
				GoreNode = Node;
			}
			else
			{
				`log("Hydra_Base: More than one gore node found!"@Mesh.AnimTreeTemplate);
			}
		}

		if(GoreNode == None)
		{
			`log("Hydra_Base: No gore node found!"@Mesh.AnimTreeTemplate);
		}
		else
		{
			GoreNode.ActorSpaceAdditionalVel = GoreFakeVelocity;

			for (i = 0; i < GoreNode.GoreSetup.length; i++)
			{
				bGoreNodesEnabled[i] = byte(GoreNode.GoreSetup[i].bEnabled);
			}
		}
	}
}

/** Handle repnotifys (client only) */
simulated function ReplicatedEvent(name VarName)
{
	local int i;

	if(VarName == 'LaserMode')
	{
		LaserModeChanged();
	}
	else if(VarName == 'bJawChomping')
	{
		PlayChompingAnim( FALSE );
	}
	else if( VarName == 'bLowerHead' )
	{
		PlayHeadLowerAnim();
	}
	else if( VarName == 'bRoaring' )
	{
		PlayRoarAnim( FALSE );
	}
	else if(VarName == 'bTurretLowered')
	{
		TurretLoweredChanged();
	}
	else if(VarName == 'DamageAnimCount')
	{
		ClientPlayDamageAnim();
	}
	else if(VarName == 'bHideClaw')
	{
		HideClawStateChanged();
	}
	else if(VarName == 'bDisableRootAnim')
	{
		RootAnimStateChanged();
	}
	else if(VarName == 'RestartMainBoneAnimCount')
	{
		MainBoneAnimNode.PlayAnim(TRUE, 1.0, 0.0);
	}
	else if(VarName == 'GrabActor')
	{
		GrabActorArrayChanged();
	}
	else if (VarName == nameof(bGoreNodesEnabled))
	{
		if (GoreNode != None)
		{
			for (i = 0; i < GoreNode.GoreSetup.length; i++)
			{
				GoreNode.GoreSetup[i].bEnabled = bool(bGoreNodesEnabled[i]);
			}
		}
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

simulated event OnAnimEnd( AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime )
{
	if( SeqNode.AnimSeqName == 'UR_Head_Chomp_Start' ||
		SeqNode.AnimSeqName == 'UR_Head_Chomp_End' )
	{
		PlayChompingAnim( TRUE );
	}
	else
	if( SeqNode.AnimSeqName == 'UR_Head_Roar_Start' ||
		SeqNode.AnimSeqName == 'UR_Head_Roar_End' )
	{
		PlayRoarAnim( TRUE );
	}
}


/** (client + server) */
simulated event Tick(float DeltaTime)
{
	local Rotator	TurretRot;
	local vector	BonePos, AimVec;
	local float		PivotTargetMag, AdjustAngle, MainBoneAnimPos;
	local float		SpeedDiff;
	local int		i, BoneIndex;

	Super.Tick(DeltaTime);

	// Find gun direction
	TurretRot = Rotation;
	if(AimAtActor != None)
	{
		// Use position of turret base as start of aim
		BonePos = Mesh.GetBoneLocation('b_UR_TurretArm03', 0);
		AimVec = (AimAtActor.Location - BonePos);
		TurretRot = rotator(AimVec);

		PivotTargetMag = VSize(AimVec);
		AdjustAngle = (PI * 0.5) - Acos(FireOffsetZ/PivotTargetMag); // correction in radians
		TurretRot.Pitch -= (AdjustAngle * 10430.2192); // Convert from radians to Unreal units
	}

	// Update gun controls
	TurretControl.DesiredBoneRotation = TurretRot;

	// Update (frickin) laser beams - if they are on
	if(LaserMode != EHLM_Off)
	{
		SetLaserEndPoint(PSC_TargetLaser[0], 'UR_Minigun_Barrel_Lt');
		SetLaserEndPoint(PSC_TargetLaser[1], 'UR_Minigun_Barrel_Rt');
	}

	UpdateGrabControlTargets();

	if(Role == ROLE_Authority)
	{
		// See if the root anim time is before last frame - if so we have looped - so tell clients
		MainBoneAnimPos = MainBoneAnimNode.CurrentTime;
		if(MainBoneAnimPos < LastMainBoneAnimPos)
		{
			RestartMainBoneAnimCount++;
		}

		// Save position
		LastMainBoneAnimPos = MainBoneAnimPos;
	}

	SpeedDiff = (VSize(Velocity)-OldSpeed);
	if( Abs(SpeedDiff) > 100 )
	{
		PlayAccelAnim( SpeedDiff );
	}
	OldSpeed = VSize(Velocity);

	// Count down any broken off claws to then destroy/hide them
	for(i=0; i<4; i++)
	{
		if(ClawHideCountdown[i] > 0.0)
		{
			ClawHideCountdown[i] -= DeltaTime;
			if(ClawHideCountdown[i] <= 0.0)
			{
				BoneIndex = Mesh.MatchRefBone(HideClawBoneNames[i]);
				if(BoneIndex != INDEX_NONE)
				{
					Mesh.HideBone(BoneIndex, TRUE);
				}
				ClawHideCountdown[i] = 0.0;
			}
		}
	}
}

/** Util for updating controls to where the claw is supposed to be (client + server) */
simulated function UpdateGrabControlTargets()
{
	local vector GrabPos;
	local rotator GrabRot;
	local matrix RelTM, ActorTM, ClawWorldTM;
	local int i;
	local Vehicle_RideReaver_Base RideReaver;

	// Update grab controls
	for(i=0; i<4; i++)
	{
		if(GrabActor[i] != None)
		{
			RideReaver = Vehicle_RideReaver_Base(GrabActor[i]);

			// If no socket, or no ridereaver, just actor offset
			if(GrabActorSocketName[i] == '' || RideReaver == None)
			{
				RelTM = MakeRotationTranslationMatrix(GrabActorOffset[i], GrabActorRotOffset[i]);
				ActorTM = MakeRotationTranslationMatrix(GrabActor[i].Location, GrabActor[i].Rotation);
				ClawWorldTM = RelTM * ActorTM;
				
				GrabPos = MatrixGetOrigin(ClawWorldTM);
				GrabRot = MatrixGetRotator(ClawWorldTM);
			}
			// Otherwise use socket
			else
			{
				RideReaver.Mesh.GetSocketWorldLocationAndRotation(GrabActorSocketName[i], GrabPos, GrabRot);
			}

			// Set IK control target
			IKControls[i].EffectorLocation = GrabPos;
			DrawDebugSphere(IKControls[i].EffectorLocation, 10.0, 10, 255,0,0, FALSE);

			// Set single-bone control target
			SBControl[i].BoneTranslation = IKControls[i].EffectorLocation;
			SBControl[i].BoneRotation = GrabRot;
		}
	}
}

/** Util to see if a bone it relevant to a particular damage bone */
simulated function bool BoneForDamage(name Bone, name Parent)
{
	return (Bone == Parent || Mesh.BoneIsChildOf(Bone, Parent));
}

/** Fire off any events for supplied part being damaged. (server only) */
simulated function FirePartDamageEvent(EHydraDamagedPart DamagedPart, Controller EventInstigator)
{
	local SequenceEvent Evt;
	local SeqEvt_HydraPartDamaged DamageEvt;

	// Look over bound events
	foreach GeneratedEvents(Evt)
	{
		// See if its a hydra one
		DamageEvt = SeqEvt_HydraPartDamaged(Evt);
		if (DamageEvt != None)
		{
			// and part set matches
			if(DamagedPart == DamageEvt.DamagedPart)
			{
				// fire it off!
				Evt.CheckActivate(self, EventInstigator, FALSE);
			}
		}
	}
}

/** Intercept damage to look at which bone it hit and decrement relevant health */
simulated event TakeDamage(int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	local EHydraDamagedPart DamagedPart;
	local int PartIndex;
	local bool bExplosiveDmg;

	//`log("HYDRA DAMAGE"@EventInstigator@DamageType@HitInfo.BoneName@DamageCauser@HitLocation);
	//DrawDebugSphere(HitLocation, 25.0, 10.0, 255,0,0, TRUE);

	// If explosive- we damage all enabled parts.
	bExplosiveDmg = ClassIsChildOf(DamageType, class'GDT_Explosive');

	// First, pass to cosmetic gore system
	if(GoreNode != None && bEnableShootingGore)
	{
		if(bExplosiveDmg)
		{
			GoreNode.UpdateGoreDamageRadial(HitLocation, Damage, FALSE);
		}
		else
		{
			GoreNode.UpdateGoreDamage(HitInfo.BoneName, HitLocation, Damage);
		}
	}

	// Ignore hits to things other than main hydra mesh
	if(Role < ROLE_Authority)
	{
		return;
	}

	// Decrease part health based on hit bone name
	for(PartIndex=0; PartIndex<EHDP_MAX; PartIndex++)
	{
		//`log("COMPARE"@HitInfo.BoneName@PartShootBody[PartIndex]);
		if(bExplosiveDmg || BoneForDamage(HitInfo.BoneName, PartShootBody[PartIndex]))
		{
			//`log("OK!"@PartHealth[PartIndex]);
			if(PartHealth[PartIndex] > 0 && bPartAllowDamage[PartIndex] == 1)
			{
				PartHealth[PartIndex] -= Damage;

				if(PartHealth[PartIndex] <= 0)
				{
					DamagedPart = EHydraDamagedPart(PartIndex);

					FirePartDamageEvent(DamagedPart, EventInstigator);
				}
			}
		}
	}

	if( Damage > 0 && TimeSince(LastPainSoundTime) > 2.f )
	{
		LastPainSoundTime = WorldInfo.TimeSeconds;
		PlaySound( PainVocalSound,,, TRUE );
	}
}

simulated function bool HitWillCauseDamage(TraceHitInfo HitInfo)
{
	local int PartIndex;

	for (PartIndex=0; PartIndex<EHDP_MAX; PartIndex++)
	{
		if ( (PartHealth[PartIndex] > 0) && (bPartAllowDamage[PartIndex] == 1) && BoneForDamage(HitInfo.BoneName, PartShootBody[PartIndex]) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

/** Util for updating end point of a aiming laser beam (client + server) */
simulated function SetLaserEndPoint(ParticleSystemComponent LaserPSC, name SocketName)
{
	local vector SocketPos, TraceEndPos, HitLoc, HitNorm;
	local Rotator SocketRot;
	local Actor HitActor;

	Mesh.GetSocketWorldLocationAndRotation(SocketName, SocketPos, SocketRot);
	TraceEndPos = SocketPos + (vector(SocketRot) * LaserLength);
	HitActor = Trace(HitLoc, HitNorm, TraceEndPos, SocketPos, TRUE,,, TRACEFLAG_Bullet);
	if(HitActor != None)
	{
		TraceEndPos = HitLoc;
	}
	LaserPSC.SetVectorParameter('End', TraceEndPos);
}

/** Called when LaserMode changes to update particle system (client + server) */
simulated function LaserModeChanged()
{
	if(LaserMode == EHLM_Off)
	{
		PSC_TargetLaser[0].DeactivateSystem();
		PSC_TargetLaser[1].DeactivateSystem();
	}
	else
	{
		PSC_TargetLaser[0].ActivateSystem();
		PSC_TargetLaser[1].ActivateSystem();

		if(LaserMode == EHLM_NoLock)
		{
			PSC_TargetLaser[0].SetVectorParameter('Laser_Color', LaserNoLockColor);
			PSC_TargetLaser[1].SetVectorParameter('Laser_Color', LaserNoLockColor);
		}
		else
		{
			PSC_TargetLaser[0].SetVectorParameter('Laser_Color', LaserLockColor);
			PSC_TargetLaser[1].SetVectorParameter('Laser_Color', LaserLockColor);
		}

		// Update end points now
		SetLaserEndPoint(PSC_TargetLaser[0], 'UR_Minigun_Barrel_Lt');
		SetLaserEndPoint(PSC_TargetLaser[1], 'UR_Minigun_Barrel_Rt');
	}
}

/** Handler for SeqAct_HydraSetTargetLaser action  (server only) */
function OnHydraSetTargetLaser(SeqAct_HydraSetTargetLaser Action)
{
	LaserMode = Action.NewMode;
	LaserModeChanged();
}

/** Handler for SeqAct_HydraToggleBodyParts action  (server only) */
function OnHydraToggleBodyParts( SeqAct_HydraToggleBodyParts Action )
{
	local int PartIdx, NodeIdx, Idx;
	local array<Name> BoneList;
	local byte NewState;

	for( PartIdx = 0; PartIdx < Action.Parts.Length; PartIdx++ )
	{
		if( Action.Parts[PartIdx] == TPI_Helmet )
		{
			for( Idx = 0; Idx < HelmetGoreBoneName.Length; Idx++ )
			{
				BoneList[BoneList.Length] = HelmetGoreBoneName[Idx];
			}
		}
		else
		if( Action.Parts[PartIdx] == TPI_Butt )
		{
			for( Idx = 0; Idx < HelmetGoreBoneName.Length; Idx++ )
			{
				BoneList[BoneList.Length] = ButtGoreBoneName[Idx];
			}
		}
		else
		if( Action.Parts[PartIdx] == TPI_Mouth01 )
		{
			BoneList[BoneList.Length] = MouthGoreBoneName[0];
		}
		else
		if( Action.Parts[PartIdx] == TPI_Mouth02 )
		{
			BoneList[BoneList.Length] = MouthGoreBoneName[1];
		}
		else
		if( Action.Parts[PartIdx] == TPI_Mouth03 )
		{
			BoneList[BoneList.Length] = MouthGoreBoneName[2];
		}
	}

	for( PartIdx = 0; PartIdx < BoneList.Length; PartIdx++ )
	{
		NodeIdx = GoreNode.GoreSetup.Find( 'GoreBoneName', BoneList[PartIdx] );

		//`log( ">>>"@PartIdx@BoneList[PartIdx]@NodeIdx@OtherIdx );

		if( NodeIdx >= 0 )
		{
			if( Action.InputLinks[0].bHasImpulse )
			{
				GoreNode.GoreSetup[NodeIdx].bEnabled = TRUE;
			}
			else if( Action.InputLinks[1].bHasImpulse )
			{
				GoreNode.GoreSetup[NodeIdx].bEnabled = FALSE;
			}
			else
			{
				GoreNode.GoreSetup[NodeIdx].bEnabled = !GoreNode.GoreSetup[NodeIdx].bEnabled;
			}
			bGoreNodesEnabled[NodeIdx] = byte(GoreNode.GoreSetup[NodeIdx].bEnabled);

			if( Action.bForceFallOff && GoreNode.GoreSetup[NodeIdx].bEnabled )
			{
				GoreNode.ForceLoseGorePiece( GoreNode.GoreSetup[NodeIdx].GoreBoneName );
			}

			//`log( "GoreBone enabled?"@GoreNode.GoreSetup[NodeIdx].bEnabled  );
		}
/*
		else
		{
			for( Idx = 0; Idx < GoreNode.GoreSetup.Length; Idx++ )
			{
				`log( ">>>>>>>"@Idx@GoreNode.GoreSetup[Idx].GoreBoneName@(GoreNode.GoreSetup[Idx].GoreBoneName==BoneList[PartIdx]));
			}
		}
*/
	}

	// Update the replicated 'bHideClaw' array
	for( PartIdx = 0; PartIdx < Action.Parts.Length; PartIdx++ )
	{
		// If its one of the claws we are modifying...
		if( Action.Parts[PartIdx] == TPI_FrontLeftTentacle	||
			Action.Parts[PartIdx] == TPI_BackLeftTentacle	||
			Action.Parts[PartIdx] == TPI_FrontRightTentacle ||
			Action.Parts[PartIdx] == TPI_BackRightTentacle	)
		{
			Idx = Action.Parts[PartIdx]-TPI_FrontLeftTentacle;

			if( Action.InputLinks[0].bHasImpulse )
			{
				NewState = 0;
			}
			else if( Action.InputLinks[1].bHasImpulse )
			{
				NewState = 1;
			}
			else
			{
				NewState = (bHideClaw[Idx] == 1) ? 0 : 1;
			}

			bHideClaw[Idx] = NewState;
		}
	}

	// Update graphics state on the server
	HideClawStateChanged();
}

/** When a hide claw bool changes, call this to update mesh state. */
simulated function HideClawStateChanged()
{
	local int i;
	local Name TentacleBoneName;
	local vector BoneLoc;

	for(i=0; i<4; i++)
	{
		if((bHideClaw[i] != bOldHideClaw[i]) && bHideClaw[i] == 1)
		{
			ClawHideCountdown[i] = ClawHideTime;

			TentacleBoneName = GoreBreakableJoints[i];

			// Turn on blending in physics!
			Mesh.PhysicsWeight = 1.0;

			BoneLoc = Mesh.GetBoneLocation( TentacleBoneName );
			Mesh.BreakConstraint( vect(0,0,-100), BoneLoc, TentacleBoneName );

			Mesh.AddInstanceVertexWeightBoneParented(TentacleBoneName, FALSE);
			Mesh.AddInstanceVertexWeightBoneParented(Mesh.GetParentBone(TentacleBoneName), FALSE);
		}
	}

	// Update 'old' state
	for(i=0; i<4; i++)
	{
		bOldHideClaw[i] = bHideClaw[i];
	}
}

/** Handler for SeqAct_HydraSetAimAtActor action  (server only) */
function OnHydraSetAimAtActor(SeqAct_HydraSetAimAtActor Action)
{
	local Controller C;
	local Actor AimActor;

	AimActor = Action.NewAimAtActor;
	C = Controller(AimActor);
	if(C != None)
	{
		AimActor = C.Pawn;
	}

	`log("HYDRA AIM AT"@AimActor);
	AimAtActor = AimActor;
}

/** Handler for SeqAct_HydraSetDamage action  (server only) */
function OnHydraSetDamage(SeqAct_HydraSetDamage Action)
{
	local int PartIndex;

	PartIndex = Action.Part;

	// Allow damage
	if(Action.InputLinks[0].bHasImpulse)
	{
		bPartAllowDamage[PartIndex] = 1;
	}
	// Disallow damage
	else if(Action.InputLinks[1].bHasImpulse)
	{
		bPartAllowDamage[PartIndex] = 0;
	}

	// Set health (can be done at some time as other things)
	if(Action.InputLinks[2].bHasImpulse)
	{
		PartHealth[PartIndex] = Action.Health;
	}
}

function OnHydraRoar( SeqAct_HydraRoar Action )
{
	// Roaring
	if( Action.InputLinks[0].bHasImpulse )
	{
		bRoaring = TRUE;
		if( Action.Duration > 0 )
		{
			SetTimer( Action.Duration, FALSE, nameof(CancelRoar) );
		}
		else
		{
			ClearLatentAction( class'SeqAct_HydraRoar', FALSE );
		}
	}
	// Not roaring
	else
	if( Action.InputLinks[1].bHasImpulse )
	{
		bRoaring = FALSE;
		ClearLatentAction( class'SeqAct_HydraRoar', FALSE );
	}

	PlayRoarAnim( FALSE );
}

function CancelRoar()
{
	bRoaring = FALSE;
	PlayRoarAnim( FALSE );

	ClearLatentAction( class'SeqAct_HydraRoar', FALSE );
}

/** Called when jaw roar-ness has changed */
simulated function PlayRoarAnim( bool bIdle )
{
	if( bRoaring )
	{
		if( bIdle )
		{
			HeadBlendList.SetActiveChild( 4, 0.1f );
		}
		else
		{
			ChompBlendNode.SetMaskWeight( 0, 1.f, 0.5f );
			HeadBlendList.SetActiveChild( 3, 0.1f );
		}
	}
	else
	{
		if( bIdle )
		{
			ChompBlendNode.SetMaskWeight( 0, 0.f, 0.5f );
		}
		else
		{
			HeadBlendList.SetActiveChild( 5, 0.1f );
		}
	}
}

function OnHydraHeadDown( SeqAct_HydraHeadDown Action )
{
	// lower head
	if(Action.InputLinks[0].bHasImpulse)
	{
		bLowerHead = TRUE;
		if( Action.Duration > 0 )
		{
			SetTimer( Action.Duration, FALSE, nameof(CancelHeadLowered) );
		}
		else
		{
			ClearLatentAction( class'SeqAct_HydraHeadDown', FALSE );
		}
	}
	// raise head
	else if( Action.InputLinks[1].bHasImpulse )
	{
		bLowerHead = FALSE;
		ClearLatentAction( class'SeqAct_HydraHeadDown', FALSE );
	}

	PlayHeadLowerAnim();
}


function CancelHeadLowered()
{
	bLowerHead = FALSE;
	PlayHeadLowerAnim();

	ClearLatentAction( class'SeqAct_HydraHeadDown', FALSE );
}

simulated function PlayHeadLowerAnim()
{
	if( bLowerHead )
	{
		ChompBlendNode.SetMaskWeight( 0, 1.0, 0.25 );
		HeadBlendList.SetActiveChild( 6, 0.1f );
	}
	else
	{
		ChompBlendNode.SetMaskWeight(0, 0.0, 0.25);
	}
}


/** Handle SeqAct_HydraJawChomp action (server only) */
function OnHydraJawChomp(SeqAct_HydraJawChomp Action)
{
	// Chomping
	if(Action.InputLinks[0].bHasImpulse)
	{
		bJawChomping = TRUE;
		if( Action.Duration > 0 )
		{
			SetTimer( Action.Duration, FALSE, nameof(CancelChomp) );
		}
		else
		{
			ClearLatentAction( class'SeqAct_HydraJawChomp', FALSE );
		}
	}
	// Not chomping
	else if(Action.InputLinks[1].bHasImpulse)
	{
		bJawChomping = FALSE;
		ClearLatentAction( class'SeqAct_HydraJawChomp', FALSE );
	}

	PlayChompingAnim( FALSE );
}

function CancelChomp()
{
	bJawChomping = FALSE;
	PlayChompingAnim( FALSE );

	ClearLatentAction( class'SeqAct_HydraJawChomp', FALSE );
}

/** Called when jaw chomping-ness has changed */
simulated function PlayChompingAnim( bool bIdle )
{
	if( bJawChomping )
	{
		if( bIdle )
		{
			HeadBlendList.SetActiveChild( 1, 0.1f );
		}
		else
		{
			ChompBlendNode.SetMaskWeight( 0, 1.0, 0.5 );
			HeadBlendList.SetActiveChild( 0, 0.1f );
		}
	}
	else
	{
		if( bIdle )
		{
			ChompBlendNode.SetMaskWeight(0, 0.0, 0.5);
		}
		else
		{
			HeadBlendList.SetActiveChild( 2, 0.1f );
		}
	}
}

simulated function OnHydraTentacleGrab(SeqAct_HydraTentacleGrab Action)
{
	local int TentacleIndex;
	local Controller C;

	TentacleIndex = Action.Tentacle;

	if(Action.InputLinks[0].bHasImpulse)
	{
		GrabActor[TentacleIndex] = Action.GrabActor;
		// Check if its a controller - if so, grab pawn
		C = Controller(GrabActor[TentacleIndex]);
		if(C != None)
		{
			GrabActor[TentacleIndex] = C.Pawn;
		}

		`log("GRAB!"@TentacleIndex@GrabActor[TentacleIndex]);
	}
	else if(Action.InputLinks[1].bHasImpulse)
	{
		`log("RELEASE");
		GrabActor[TentacleIndex] = None;
	}

	GrabActorArrayChanged();
}

simulated function GrabActorChanged(INT TentacleIndex)
{
	// Released!
	if(GrabActor[TentacleIndex] == None)
	{
		IKControls[TentacleIndex].SetSkelControlStrength(0.0, GrabIKBlendTime);
		SBControl[TentacleIndex].SetSkelControlStrength(0.0, GrabSBBlendTime);

		ClearTimer('StartIK');
		ClearTimer('StartSB');

		SetTimer( BlendTentacleDelay, FALSE, nameof(FinishBlend) );
		SetTimer( TrailTentacleDelay, FALSE, nameof(FinishTrail) );

		GrabAnimNode.SetAnim('UR_TentacleDamaged');
		GrabAnimNode.PlayAnim(FALSE, 1.0, 0.0); // Play damage anim
	}
	// Grabbed!
	else
	{
		GrabTentacleIndex = TentacleIndex;

		TrailControls[TentacleIndex].SetSkelControlStrength(0.0, TrailBlendTime);

		ClearTimer('FinishBlend');
		ClearTimer('FinishTrail');

		SetTimer( GrabIKDelay, FALSE, nameof(StartIK) );
		SetTimer( GrabSBDelay, FALSE, nameof(StartSB) );

		GrabAnimNode.SetAnim('UR_TentacleGrab');
		GrabAnimNode.PlayAnim(FALSE, 1.0, 0.0);		// Play grab anim
		GrabMaskNode.SetMaskWeight(TentacleIndex, 1.0, 0.2);

		UpdateGrabControlTargets();
	}
}

/** The GrabActor array has changed - find which one and call GrabActorChanged (client only) */
simulated function GrabActorArrayChanged()
{
	local int i;

	// Look to see what changed
	for(i=0; i<4; i++)
	{
		if(GrabActor[i] != OldGrabActor[i])
		{
			GrabActorChanged(i);
		}

		// Save off new state to compare in future
		OldGrabActor[i] = GrabActor[i];
	}
}

simulated function FinishBlend()
{
	GrabMaskNode.SetMaskWeight(GrabTentacleIndex, 0.0, BlendTentacleTime);
}

simulated function FinishTrail()
{
	TrailControls[GrabTentacleIndex].SetSkelControlStrength(1.0, TrailBlendTime);
}

simulated function StartIK()
{
	IKControls[GrabTentacleIndex].SetSkelControlStrength(1.0, GrabIKBlendTime);
}

simulated function StartSB()
{
	SBControl[GrabTentacleIndex].SetSkelControlStrength(GrabSBStrength, GrabSBBlendTime);
}

/** Handle kismet action for raising/lowering turret (client + server) */
function OnHydraMoveTurret(SeqAct_HydraMoveTurret Action)
{
	if(Action.InputLinks[0].bHasImpulse)
	{
		bTurretLowered = TRUE;
	}
	else if(Action.InputLinks[1].bHasImpulse)
	{
		bTurretLowered = FALSE;
	}

	TurretLoweredChanged();
}

/** Handle change to  */
simulated function TurretLoweredChanged()
{
	if(bTurretLowered)
	{
		TurretControl.SetSkelControlActive(FALSE);
		TurretBlendNode.SetBlendTarget(1.0, 0.5);
	}
	else
	{
		TurretControl.SetSkelControlActive(TRUE);
		TurretBlendNode.SetBlendTarget(0.0, 0.5);
	}
}

/** Called from kismet to play an animation. (server only) */
function OnHydraPlayAnim(SeqAct_HydraPlayAnim Action)
{
	if(Action.InputLinks[0].bHasImpulse)
	{
		DamageAnimCount++;
		ClientPlayDamageAnim();
	}
}

/** Handle a change in DamageAnimCount and play animation (client + server) */
simulated function ClientPlayDamageAnim()
{
	FullBodySlot.PlayCustomAnim('ADD_UR_Damaged', 1.0, 0.2, 0.2, FALSE);
}

/** Called from kismet to toggle root animation. (server only) */
function OnHydraSetRootAnim(SeqAct_HydraSetRootAnim Action)
{
	if(Action.InputLinks[0].bHasImpulse)
	{
		bDisableRootAnim = FALSE;
	}
	else if(Action.InputLinks[1].bHasImpulse)
	{
		bDisableRootAnim = TRUE;
	}

	RootAnimStateChanged();
}

/** When root anim bool changes, call this to update anim state. */
simulated function RootAnimStateChanged()
{
	if(bDisableRootAnim)
	{
		FlyAnimBlend.SetBlendTarget(1.0, 0.5);
	}
	else
	{
		FlyAnimBlend.SetBlendTarget(0.0, 0.5);
	}
}

/** CAlled from anim notify when roaring */
simulated function SpittleNotify()
{
	PSC_Spittle.ActivateSystem(FALSE);
}

simulated function PlayAccelAnim( float Diff )
{
	if( Diff < 0 )
	{
		if( TimeSince(LastPlaySlowDownAnimTime) > 3.f )
		{
			LastPlaySlowDownAnimTime = WorldInfo.TimeSeconds;
			RandomLegAnimNode.SetActiveChild( 3, 0.5f );
		}
	}

	PlaySound( SuddenAccelSound, TRUE );
}

simulated function PlayAmbientVocalSound()
{
	if( !bHidden )
	{
		PlaySound( AmbientVocalSound, TRUE );
	}

	SetTimer( 5.f + 5.f*FRand(), FALSE, nameof(PlayAmbientVocalSound) );
}

defaultproperties
{
	Begin Object Class=DynamicLightEnvironmentComponent Name=MyLightEnvironment
		bEnabled=TRUE
		TickGroup=TG_DuringAsyncWork
	End Object
	Components.Add(MyLightEnvironment)
	LightEnvironment=MyLightEnvironment

	TickGroup=TG_PostAsyncWork

	Begin Object Class=SkeletalMeshComponent Name=SkeletalMeshComponent0
		CollideActors=TRUE
		BlockActors=FALSE
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=FALSE
		LightEnvironment=MyLightEnvironment
		bForceMeshObjectUpdates=TRUE
		bAllowAmbientOcclusion=FALSE // does not really work on skelmeshes
	End Object
	CollisionComponent=SkeletalMeshComponent0
	Mesh=SkeletalMeshComponent0
	Components.Add(SkeletalMeshComponent0)

	Begin Object Class=SkeletalMeshComponent Name=SkeletalMeshComponent1
		CollideActors=TRUE
		BlockActors=FALSE
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=FALSE
		LightEnvironment=MyLightEnvironment
		ShadowParent=SkeletalMeshComponent0
		bForceMeshObjectUpdates=TRUE
	End Object
	RiderMesh=SkeletalMeshComponent1

	Begin Object Class=ParticleSystemComponent Name=PSC_TargetLaser0
		bAutoActivate=FALSE
	End Object
	PSC_TargetLaser[0]=PSC_TargetLaser0

	Begin Object Class=ParticleSystemComponent Name=PSC_TargetLaser1
		bAutoActivate=FALSE
	End Object
	PSC_TargetLaser[1]=PSC_TargetLaser1

	Begin Object Class=ParticleSystemComponent Name=PSC_Spittle0
		bAutoActivate=FALSE
	End Object
	PSC_Spittle=PSC_Spittle0

	Physics=PHYS_Interpolating

	bEdShouldSnap=TRUE
	bStatic=FALSE
	bCollideActors=TRUE
	bBlockActors=FALSE
	bWorldGeometry=FALSE
	bCollideWorld=FALSE
	bNoEncroachCheck=TRUE
	bProjTarget=TRUE
	bUpdateSimulatedPosition=FALSE

	RemoteRole=ROLE_SimulatedProxy
	bNoDelete=TRUE


}
