
/**
 * CQC Move (Close Quarter Combat) Shield execution
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_CQCMove_Shield extends GSM_CQC_Killer_Base;

var Array<CameraAnim>	CameraAnims;

function PlayExecution()
{
	// Play execution Camera Animation.
	if( CameraAnims.Length > 0 )
	{
		PlayExecutionCameraAnim(CameraAnims);
	}

	// Play Face FX Emotion
	if( !PawnOwner.IsActorPlayingFaceFXAnim() )
	{
		PawnOwner.PlayActorFaceFXAnim(None, "Emotions", "Strained", None);
	}

	PawnOwner.SoundGroup.PlayEffort(PawnOwner, GearEffort_BoltokLongExecutionEffort, true);

	Super.PlayExecution();

	PawnOwner.SetTimer(BlendInTime/SpeedModifier, FALSE, nameof(AttachShieldToHand), Self);
	PawnOwner.SetTimer(AnimLength - BlendOutTime/SpeedModifier, FALSE, nameof(AttachShieldToArm), Self);
}

function AttachShieldToHand()
{
	local SkeletalMeshComponent ShieldMesh;
	local Name		SocketName;
	local Vector	RelativeLocation, Scale3D;
	local Rotator	RelativeRotation;
	local SkeletalMeshSocket	ShieldSocket;

	ShieldSocket = PawnOwner.Mesh.GetSocketByName(PawnOwner.GetLeftHandSocketName());
	if( ShieldSocket != None )
	{	
		ShieldMesh = PawnOwner.EquippedShield.Mesh;
		Scale3D = vect(1,1,1);
		PawnOwner.GetShieldAttachBone(SocketName, RelativeRotation, RelativeLocation, Scale3D);
		PawnOwner.Mesh.AttachComponent(ShieldMesh, ShieldSocket.BoneName, ShieldSocket.RelativeLocation, ShieldSocket.RelativeRotation, Scale3D);
	}
}

function AttachShieldToArm()
{
	local SkeletalMeshComponent ShieldMesh;
	local Name		SocketName;
	local Vector	RelativeLocation, Scale3D;
	local Rotator	RelativeRotation;

	ShieldMesh = PawnOwner.EquippedShield.Mesh;
	Scale3D = vect(1,1,1);
	PawnOwner.GetShieldAttachBone(SocketName, RelativeRotation, RelativeLocation, Scale3D);
	PawnOwner.Mesh.AttachComponent(ShieldMesh, PawnOwner.Mesh.GetSocketBoneName(SocketName), RelativeLocation, RelativeRotation, Scale3D);
}

/** Separate function, so other executions can implement their variations. */
function SetVictimRotation()
{
	// Override rotation.
	VictimDesiredYaw = NormalizeRotAxis(Rotator(-DirToVictim).Yaw + 16384);
	Super.SetVictimRotation();
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	// Clear timers if aborted early
	if( PawnOwner.IsTimerActive(nameof(AttachShieldToHand), Self) )
	{
		PawnOwner.ClearTimer(nameof(AttachShieldToHand), Self);
	}
	if( PawnOwner.IsTimerActive(nameof(AttachShieldToArm), Self) )
	{
		PawnOwner.ClearTimer(nameof(AttachShieldToArm), Self);
		AttachShieldToArm();
	}

	Super.SpecialMoveEnded(PrevMove, NextMove);
}

defaultproperties
{
	BS_KillerAnim=(AnimName[BS_FullBody]="CTRL_Shield")
	BS_VictimAnim=(AnimName[BS_FullBody]="DBNO_Shield")

	CameraAnims(0)=CameraAnim'COG_MarcusFenix.Camera_Anims.DBNO_Shield_Cam01'
	CameraAnims(1)=CameraAnim'COG_MarcusFenix.Camera_Anims.DBNO_Shield_Cam02'
	CameraAnims(2)=CameraAnim'COG_MarcusFenix.Camera_Anims.DBNO_Shield_Cam03'

	VictimDeathTime=1.58f
	ExecutionDamageType=class'GDT_ShieldExecute'
	MarkerRelOffset=(X=54.06,Y=12.39,Z=0.0)
	VictimRotInterpSpeed=0.f

	BlendInTime=0.2f
	BlendOutTime=0.2f
}
