/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Vehicle_Jack_Base extends GearVehicle
	native(Vehicle)
	config(Pawn)
	abstract;

cpptext
{
		virtual FLOAT GetGravityZ();
		virtual UBOOL AdjustFlight(FLOAT ZDiff, UBOOL bFlyingDown, FLOAT Distance, AActor* GoalActor);
		virtual FVector AdjustDestination( AActor* GoalActor, FVector Dest=FVector(EC_EventParm) );
		virtual void AdjustThrottle( FLOAT Distance );
}

/** Height for jack to adjust over coverslotmarker so he can fly over mantle paths */
var	float	MantleAdjustHeight;
var float	StdAdjustHeight;

var SoundCue	JackCloakSound,
				JackDecloakSound,
				JackAcquiredPOISound,
				JackInspectPOISound,
				JackHappySound,//not used
				JackSadSound,
				JackRecoilSound,
				JackIdleStillVocalSound,
				JackIdleMovingVocalSound,
				JackArmUnfoldSound,
				JackArmFoldSound,
				JackPointSound,
				JackFlyStartSound,//not used
				JackFlyLoopSound,//not used
				JackFlyStopSound,//not used
				JackFlyChangeDirSound,//not used
				JackMonitorUnfoldSound,
				JackMonitorFoldSound,
				JackWeldingIntroSound,
				JackWeldingOutroSound;

var float		LastJackSoundTime;

var AudioComponent AC_Idle, AC_Welding, AC_Scanning;

/** Spotlight component for Jack's head lights */
var() SpotLightComponent	FrontSpotlightComp;

/** Enum settings list for jack spotlight */
enum EJackSpotlightSetting
{
	JSS_Default,
	JSS_Intervention,
	JSS_Outpost,
};
var() repnotify EJackSpotlightSetting SpotlightSetting;

/** Struct containing light info for each spotlight setting */
struct native JackSpotlightInfo
{
	var() float	InnerConeAngle;
	var() float OuterConeAngle;
	var() float	Radius;
	var() float	FalloffExponent;
	var() float Brightness;
	var() Color LightColor;
};
/** List of available spotlight settings */
var() config array<JackSpotlightInfo>	SpotLightSettingList;


/** Animations */
/** Main mask blend node */
var()					GearAnim_BlendPerBone	JackMask;
/** Blend list for controlling the main body */
var()					AnimNodeBlendList		MainBlendList;
/** Blend list for controlling the monitor */
var()					AnimNodeBlendList		MonitorBlendList;
/** Blend list for controlling the right arm */
var()					AnimNodeBlendList		RightArmBlendList;
/** Sequence player for jack cloak animations */
var()					AnimNodeSequence		CloakAnimNode;

/** Whether spotlight is on/off */
var() repnotify			bool				bSpotlightOn;
/** Whether monitor is unfolded or not */
var() repnotify			bool				bUnfoldMonitor;
/** Whether jack is cloaked or not - used to track the fold/unfold animations for main body */
var() repnotify			Name				CloakedAnimName;
var						Name				JackFoldAnimName;
var						Name				JackUnfoldAnimName;
var						Name				JackRecoilAnimName;

/** Jack is fully cloaked/decloaked */
var()					bool				bCloaking;
/** Whether jack is welding or not */
var() repnotify			bool				bWelding;
/** Whether jack is recoiling or not */
var() repnotify			bool				bRecoil;
/** Whether jack is scanning or not */
var() repnotify			bool				bScanning;
/** Whether jack is pointing or not */
var() repnotify			bool				bPointing;

/** Whether jack is in cinematic mode or not */
var() repnotify			bool				bCinematicMode;

/** Amount of damage to take before playing recoil animation */
var()	config	int			RecoilDamageThreshold;
/** Last time recoil animation was played */
var				float		LastRecoilTime;

/** Effects */
/** Particle systems for eye glow/lights */
var ParticleSystem			PS_Eye;
var ParticleSystemComponent PSC_Eye_LU, PSC_Eye_LL, PSC_Eye_RU, PSC_Eye_RL;
/** Particle systems for jets */
var ParticleSystem			PS_Jets;
var ParticleSystemComponent PSC_Jets;
/** Particle systems for cloak/decloak */
var ParticleSystem			PS_Cloak;
var ParticleSystemComponent	PSC_Cloak;
/** Particle systems for welding */
var ParticleSystem			PS_Welding;
var ParticleSystemComponent	PSC_Welding;
/** Emitter for impact location of weld - updated each frame it's active */
var ParticleSystem PS_WeldingImpact;
var ParticleSystemComponent PSC_WeldingImpact;
var() float					WeldingImpactOffset;

/** Max distance welder will reach to play impact effect */
var()	config	float		MaxWeldDist;
/** Right arm attachment - hand */
var		StaticMeshComponent	RightArmAttach_Hand;
/** Right arm attachment - welder */
var		StaticMeshComponent	RightArmAttach_Welder;

var SkelControlSingleBone	SkelCtrl_MonitorOrient;

replication
{
	if( Role == ROLE_Authority )
		CloakedAnimName, bUnfoldMonitor, bSpotlightOn, bWelding, bRecoil, bScanning, bPointing, SpotlightSetting, bCinematicMode;
}

simulated event ReplicatedEvent( name VarName )
{
	Super.ReplicatedEvent( VarName );

	switch( VarName )
	{
		case 'CloakedAnimName':
			PlayCloakAnim();
			break;
		case 'bUnfoldMonitor':
			PlayMonitorAnim();
			break;
		case 'bSpotlightOn':
			SetSpotLightState( bSpotlightOn );
			break;
		case 'bWelding':
			PlayWeldingAnimation();
			break;
		case 'bScanning':
			PlayJackScanningAnimation();
			break;
		case 'bPointing':
			if( bPointing )
			{
				PlayJackPointingAnimation();
				break;
			}
		case 'bRecoil':
			if( bRecoil )
			{
				PlayJackRecoilAnimation( TRUE );
			}
			break;
		case 'SpotlightSetting':
			UpdateJackSpotlightSetting();
			break;
		case 'bCinematicMode':
			SetCinematicMode( bCinematicMode );
			break;
	}
}

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	Mesh.AttachComponentToSocket( FrontSpotlightComp, 'Spotlight' );

	PSC_Eye_LU = new(self) class'ParticleSystemComponent';
	PSC_Eye_LU.SetTemplate( PS_Eye );
	PSC_Eye_LU.SetHidden( FALSE );
	PSC_Eye_LU.ActivateSystem();
	Mesh.AttachComponentToSocket( PSC_Eye_LU, 'Eye_L_U' );

	PSC_Eye_LL = new(self) class'ParticleSystemComponent';
	PSC_Eye_LL.SetTemplate( PS_Eye );
	PSC_Eye_LL.SetHidden( FALSE );
	PSC_Eye_LL.ActivateSystem();
	Mesh.AttachComponentToSocket( PSC_Eye_LL, 'Eye_L_L' );

	PSC_Eye_RU = new(self) class'ParticleSystemComponent';
	PSC_Eye_RU.SetTemplate( PS_Eye );
	PSC_Eye_RU.SetHidden( FALSE );
	PSC_Eye_RU.ActivateSystem();
	Mesh.AttachComponentToSocket( PSC_Eye_RU, 'Eye_R_U' );

	PSC_Eye_RL = new(self) class'ParticleSystemComponent';
	PSC_Eye_RL.SetTemplate( PS_Eye );
	PSC_Eye_RL.SetHidden( FALSE );
	PSC_Eye_RL.ActivateSystem();
	Mesh.AttachComponentToSocket( PSC_Eye_RL, 'Eye_R_L' );

	PSC_Jets = new(self) class'ParticleSystemComponent';
	PSC_Jets.SetTemplate( PS_Jets );
	PSC_Jets.SetHidden( FALSE );
	PSC_Jets.ActivateSystem();
	Mesh.AttachComponentToSocket( PSC_Jets, 'Jets' );

	PSC_Cloak = new(self) class'ParticleSystemComponent';
	PSC_Cloak.bAutoActivate = FALSE;
	PSC_Cloak.SetTemplate( PS_Cloak );
	PSC_Cloak.SetHidden( TRUE );
	Mesh.AttachComponentToSocket( PSC_Cloak, 'Jets' );

	PSC_Welding = new(self) class'ParticleSystemComponent';
	PSC_Welding.bAutoActivate = FALSE;
	PSC_Welding.SetTemplate( PS_Welding );
	PSC_Welding.SetHidden( TRUE );
	Mesh.AttachComponentToSocket( PSC_Welding, 'Welder' );

	PSC_WeldingImpact = new(self) class'ParticleSystemComponent';
	PSC_WeldingImpact.bAutoActivate = false;
	PSC_WeldingImpact.SetTemplate(PS_WeldingImpact);
	PSC_WeldingImpact.SetAbsolute(true, true, true);

	Mesh.AttachComponentToSocket( RightArmAttach_Hand, 'Welder' );
	Mesh.AttachComponentToSocket( RightArmAttach_Welder, 'Welder' );

	ShowRightArmAttachment( RightArmAttach_Hand );
	UpdateJackSpotlightSetting();
}

simulated function PlayDying( class<DamageType> DamageType, vector HitLoc )
{
	Super.PlayDying( DamageType, HitLoc );

	if( PSC_Eye_LU		!= None ) { PSC_Eye_LU.DeactivateSystem();	}
	if( PSC_Eye_LL		!= None ) { PSC_Eye_LL.DeactivateSystem();	}
	if( PSC_Eye_RU		!= None ) { PSC_Eye_RU.DeactivateSystem();	}
	if( PSC_Eye_RL		!= None ) { PSC_Eye_RL.DeactivateSystem();	}
	if( PSC_Jets		!= None ) { PSC_Jets.DeactivateSystem();	}
	if( PSC_Welding		!= None ) { PSC_Welding.DeactivateSystem();	}
	if( PSC_Cloak		!= None ) { PSC_Cloak.DeactivateSystem();	}
}

simulated function CacheAnimNodes()
{
	Super.CacheAnimNodes();

	JackMask			= GearAnim_BlendPerBone(Mesh.FindAnimNode('JackMask'));
	MainBlendList		= AnimNodeBlendList(Mesh.FindAnimNode('MainBlend'));
	MonitorBlendList	= AnimNodeBlendList(Mesh.FindAnimNode('MonitorBlend'));
	RightArmBlendList   = AnimNodeBlendList(Mesh.FindAnimNode('RightArmBlend'));
	CloakAnimNode		= AnimNodeSequence(Mesh.FindAnimNode('CloakAnim'));

	SkelCtrl_MonitorOrient = SkelControlSingleBone(Mesh.FindSkelControl('MonitorOrient'));
}

simulated function Tick( float DeltaTime )
{
	local Vector	StartTrace, EndTrace, HitLocation, HitNormal;
	local Rotator	WeldDir, MonDir;
	local Actor		HitActor;
	local bool		bHideImpact;
	local PlayerController PC;

	Super.Tick( DeltaTime );

	if( PSC_Welding != None && !PSC_Welding.HiddenGame && PSC_WeldingImpact != None )
	{
		bHideImpact = TRUE;
		if( Mesh.GetSocketWorldLocationAndRotation( 'Welder', StartTrace, WeldDir ) )
		{
			EndTrace = StartTrace + Vector(WeldDir) * MaxWeldDist;
			HitActor = Trace( HitLocation, HitNormal, EndTrace, StartTrace, TRUE, vect(5,5,5) );
			if( HitActor != None )
			{
				bHideImpact = FALSE;
				PSC_WeldingImpact.SetTranslation( HitLocation + (HitNormal * WeldingImpactOffset) );
				PSC_WeldingImpact.SetRotation( WeldDir );
			}
		}

		if( bHideImpact && PSC_WeldingImpact.bAttached )
		{
			DetachComponent(PSC_WeldingImpact);
			PSC_WeldingImpact.DeactivateSystem();
		}
		else 
		if( !bHideImpact && !PSC_WeldingImpact.bAttached )
		{
			AttachComponent(PSC_WeldingImpact);
			PSC_WeldingImpact.ActivateSystem();
		}
	}

	// If the monitor orientation skel controller is active
	if( SkelCtrl_MonitorOrient != None && SkelCtrl_MonitorOrient.ControlStrength > 0.f )
	{
		// Point it to toward the first locally controlled pawn
		foreach LocalPlayerControllers( class'PlayerController', PC )
		{
			MonDir = Rotator(PC.ViewTarget.Location - Location) - Rotation;
			MonDir.Pitch = 0;
			MonDir.Roll  = 0;
			SkelCtrl_MonitorOrient.BoneRotation = MonDir;
			break;
		}
	}
}

simulated event OnAnimEnd( AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime )
{
	//debug
	`AILog_Ext( self@GetFuncName()@SeqNode.AnimSeqName@CloakedAnimName@JackUnfoldAnimName, '', MyGearAI );

	// When jack cloak/uncloak animation ends
	if( SeqNode.AnimSeqName == CloakedAnimName	||
		SeqNode.AnimSeqName == JackUnfoldAnimName )
	{
		FinishedCloaking( (SeqNode.AnimSeqName != JackUnfoldAnimName) );

		// Cancel latent action if used by LDs
		Controller.ClearLatentAction( class'SeqAct_JackControl', FALSE );
	}
	else
	// Otherwise, if monitor finished unfolding/folding
	if( SeqNode.AnimSeqName == 'monitor_unfold' ||
		SeqNode.AnimSeqName == 'monitor_fold'	)
	{
		if( SeqNode.AnimSeqName == 'monitor_fold' )
		{
			JackMask.MaskList[0].DesiredWeight = 0.f;
			JackMask.MaskList[0].BlendTimeToGo = 0.2f;

			SkelCtrl_MonitorOrient.SetSkelControlActive( FALSE );
		}
		else
		{
			SkelCtrl_MonitorOrient.SetSkelControlActive( TRUE );
			SkelCtrl_MonitorOrient.SetSkelControlStrength( 1.f, 0.5f );
		}

		// Cancel latent action for LD
		Controller.ClearLatentAction( class'SeqAct_JackControl', FALSE );
	}
	else
	// Otherwise, if right arm has finished folding in
	if( SeqNode.AnimSeqName == 'right_arm_fold' )
	{
		PlayRightArmSetup( TRUE );
	}
	else
	// Otherwise, if right arm has finished unfolding
	if( SeqNode.AnimSeqName == 'right_arm_unfold' )
	{
		FinishedRightArmSetup();
	}
	else
	// Otherwise, if welding intro animation ends
	if( SeqNode.AnimSeqName == 'welding_intro' )
	{
		// Go to welding loop
		PlayWeldingAnimation( TRUE );
		// Cancel latent action for LD
		Controller.ClearLatentAction( class'SeqAct_JackControl', FALSE );
	}
	else
	// Otherwise, if welding outro animation ends
	if( SeqNode.AnimSeqName == 'welding_outro' )
	{
		// Go back to idle anim
		MainBlendList.SetActiveChild( 0, 0.f );

		// Remove welder from right arm
		PlayRightArmSetup();

		// Cancel latent action for LD
		Controller.ClearLatentAction( class'SeqAct_JackControl', FALSE );
	}
	else
	// Otherwise, if recoiling
	if( SeqNode.AnimSeqName == JackRecoilAnimName )
	{
		SetTimer( GetRecoilDuration(), FALSE, nameof(PlayJackRecoilAnimation) );
	}
	else
	// Otherwise, if pointing
	if( SeqNode.AnimSeqName == 'pointing' )
	{
		// Go back to idle anim
		MainBlendList.SetActiveChild( 0, 0.f );

		bPointing = FALSE;

		// Cancel latent action for LD
		Controller.ClearLatentAction( class'SeqAct_JackControl', FALSE );
	}

	Super.OnAnimEnd( SeqNode, PlayedTime, ExcessTime );
}

simulated function bool IsCloaked()
{
	return (CloakedAnimName != '');
}

function bool SetCloakState( Name inCloakedAnimName, optional bool bForce )
{
	//debug
	`AILog_Ext( self@GetFuncName()@CloakedAnimName@inCloakedAnimName@bForce, 'None', MyGearAI );

	if( bForce || 
		( inCloakedAnimName != '' && CloakedAnimName == '') || 
		( inCloakedAnimName == '' && CloakedAnimName != '') )
	{
		bCloaking = TRUE;
		CloakedAnimName = inCloakedAnimName;
		if( !IsCloaked() )
		{
			ShowJack();
		}
		else
		{
			SetMonitorState( FALSE );
		}
		PlayCloakAnim();
		return TRUE;
	}
	return FALSE;
}

simulated function PlayCloakAnim()
{
	//debug
	`AILog_Ext( self@GetFuncName()@CloakedAnimName, 'None', MyGearAI );

	if( PSC_Cloak != None )
	{
		PSC_Cloak.SetHidden( FALSE );
		PSC_Cloak.ActivateSystem();
	}

	if( IsCloaked() )
	{
		HideJack( TRUE );
		FrontSpotlightComp.SetEnabled( FALSE );

		PlayJackSound( JackCloakSound, TRUE );

		CloakAnimNode.SetAnim( CloakedAnimName );
		MainBlendList.SetActiveChild( 1, 0.2f );
	}
	else
	{
		PlayJackSound( JackDecloakSound, TRUE );

		MainBlendList.SetActiveChild( 2, 0.0f );
	}
}

simulated function FinishedCloaking( bool bHide, optional bool bForceResUpdate )
{
	// Go back to idle anim
	MainBlendList.SetActiveChild( 0, 0.f );

	// Hide jack if cloaking
	if( bHide )
	{
		HideJack();
	}
	// Turn jacks spotlight back on when done unfolding
	else
	{
		CloakedAnimName = '';
		FrontSpotlightComp.SetEnabled( bSpotLightOn );
	}

	// Turn off cloaking effect
	if( PSC_Cloak != None )
	{
		PSC_Cloak.SetHidden( TRUE );
		PSC_Cloak.DeactivateSystem();
	}

	// No longer recoiling
	bRecoil = FALSE;
	// Finished cloaking
	bCloaking = FALSE;
}

simulated function HideJack( optional bool bEffectsOnly )
{
	//debug
	`AILog_Ext( self@GetFuncName()@bEffectsOnly, '', MyGearAI );

	if( !bEffectsOnly )
	{
		SetHidden( TRUE );
		SetCollision( FALSE, FALSE, FALSE );
		AC_Idle.Stop();
	}

	if( PSC_Eye_LU != None ) { PSC_Eye_LU.SetHidden( TRUE ); PSC_Eye_LU.DeactivateSystem(); }
	if( PSC_Eye_LL != None ) { PSC_Eye_LL.SetHidden( TRUE ); PSC_Eye_LL.DeactivateSystem(); }
	if( PSC_Eye_RU != None ) { PSC_Eye_RU.SetHidden( TRUE ); PSC_Eye_RU.DeactivateSystem(); }
	if( PSC_Eye_RL != None ) { PSC_Eye_RL.SetHidden( TRUE ); PSC_Eye_RL.DeactivateSystem(); }

	if( PSC_Welding != None ) { PSC_Welding.SetHidden( TRUE ); PSC_Welding.DeactivateSystem(); }
	if (PSC_WeldingImpact != None)
	{
		DetachComponent(PSC_WeldingImpact);
		PSC_WeldingImpact.DeactivateSystem();
	}
}

simulated function ShowJack()
{
	//debug
	`AILog_Ext( self@GetFuncName(), '', MyGearAI );

	SetHidden( FALSE );
	SetCollision( default.bCollideActors, default.bBlockActors, default.bIgnoreEncroachers );
	AC_Idle.Play();

	if( PSC_Eye_LU != None ) { PSC_Eye_LU.SetHidden( FALSE ); PSC_Eye_LU.ActivateSystem(); }
	if( PSC_Eye_LL != None ) { PSC_Eye_LL.SetHidden( FALSE ); PSC_Eye_LL.ActivateSystem(); }
	if( PSC_Eye_RU != None ) { PSC_Eye_RU.SetHidden( FALSE ); PSC_Eye_RU.ActivateSystem(); }
	if( PSC_Eye_RL != None ) { PSC_Eye_RL.SetHidden( FALSE ); PSC_Eye_RL.ActivateSystem(); }
}

simulated function SetCinematicMode( bool bInCinematicMode )
{
	bCinematicMode = bInCinematicMode;
	if( bInCinematicMode )
	{
		FinishedCloaking( TRUE );
	}
	else
	{
		ShowJack();
		FinishedCloaking( FALSE, TRUE );
	}
}

function bool SetMonitorState( bool bUnfold )
{
	if( bUnfoldMonitor != bUnfold )
	{
		bUnfoldMonitor = bUnfold;
		PlayMonitorAnim();
		return TRUE;
	}
	return FALSE;
}

simulated function PlayMonitorAnim()
{
	// Turn on monitor mask
	if( JackMask.MaskList[0].DesiredWeight != 1.f )
	{
		JackMask.MaskList[0].DesiredWeight = 1.f;
		JackMask.MaskList[0].BlendTimeToGo = 0.2f;
	}

	if( bUnfoldMonitor )
	{
		MonitorBlendList.SetActiveChild( 1, 0.2f );
		PlayJackSound( JackMonitorUnfoldSound, TRUE );
	}
	else
	{
		MonitorBlendList.SetActiveChild( 0, 0.2f );
		SkelCtrl_MonitorOrient.SetSkelControlStrength( 0.f, 0.25f );

		PlayJackSound( JackMonitorFoldSound, TRUE );
	}
}

simulated function SetSpotLightState( bool bOn )
{
	bSpotLightOn = bOn;
	FrontSpotlightComp.SetEnabled( bSpotLightOn );
}

simulated function OnJackSpotlight( SeqAct_JackSpotlight inAction )
{
	SpotlightSetting = inAction.SpotlightSetting;
	UpdateJackSpotlightSetting();
}

simulated function UpdateJackSpotlightSetting()
{
	// Spotlight component settings
	FrontSpotlightComp.InnerConeAngle = SpotLightSettingList[SpotlightSetting].InnerConeAngle;
	FrontSpotlightComp.OuterConeAngle = SpotLightSettingList[SpotlightSetting].OuterConeAngle;

	// Pointlight component settings
	FrontSpotlightComp.Radius = SpotLightSettingList[SpotlightSetting].Radius;
	FrontSpotlightComp.FalloffExponent = SpotLightSettingList[SpotlightSetting].FalloffExponent;

	// Light component settings
	FrontSpotlightComp.SetLightProperties( SpotLightSettingList[SpotlightSetting].Brightness, SpotLightSettingList[SpotlightSetting].LightColor );
}

/** Updates the right arm attachment depending on what component is passed in */
simulated function ShowRightArmAttachment( PrimitiveComponent Comp )
{
	RightArmAttach_Hand.SetHidden( (Comp != RightArmAttach_Hand) );
	RightArmAttach_Welder.SetHidden( (Comp != RightArmAttach_Welder) );
}

/** Handles folding/unfolding the right arm and updating the appropriate attachment */
simulated function PlayRightArmSetup( optional bool bUpdateAttachment )
{
	// Turn on arm mask
	if( JackMask.MaskList[1].DesiredWeight != 1.f )
	{
		JackMask.MaskList[1].DesiredWeight = 1.f;
		JackMask.MaskList[1].BlendTimeToGo = 0.2f;
		JackMask.MaskList[2].DesiredWeight = 1.f;
		JackMask.MaskList[2].BlendTimeToGo = 0.2f;
	}

	// Fold arm in before updating attachment
	if( !bUpdateAttachment )
	{
		PlayJackSound( JackArmFoldSound, TRUE );

		RightArmBlendList.SetActiveChild( 0, 0.1f );
	}
	// Otherwise, arm is currently in...
	else
	{
		// Update attachment
		if( bWelding )
		{
			ShowRightArmAttachment( RightArmAttach_Welder );
		}
		else
		{
			ShowRightArmAttachment( RightArmAttach_Hand );
		}

		PlayJackSound( JackArmUnfoldSound, TRUE );

		// Then unfold the arm...
		RightArmBlendList.SetActiveChild( 1, 0.f );

	}
}

/** Arm has completed it's attachment setup and unfolded... trigger next animation */
simulated function FinishedRightArmSetup()
{
	// Turn off arm mask
	if( JackMask.MaskList[1].DesiredWeight != 0.f )
	{
		JackMask.MaskList[1].DesiredWeight = 0.f;
		JackMask.MaskList[1].BlendTimeToGo = 0.2f;
		JackMask.MaskList[2].DesiredWeight = 0.f;
		JackMask.MaskList[2].BlendTimeToGo = 0.2f;
	}

	// Transition to next action w/ right arm
	if( bWelding )
	{
		PlayWeldingAnimation();
	}
}

/** Trigger a change in the welding state */
function bool SetWeldingState( bool bWeld )
{
	if( bWeld != bWelding )
	{
		bWelding = bWeld;
		if( bWelding )
		{
			// Get welder on right arm first
			PlayRightArmSetup();
		}
		else
		{
			// Play welding outro first
			PlayWeldingAnimation();
		}
		return TRUE;
	}
	return FALSE;
}

/** Play intro/loop/outro welding animations */
simulated function PlayWeldingAnimation( optional bool bLoop )
{
	// If welding...
	if( bWelding )
	{
		// If loop anim desired...
		if( bLoop )
		{
			// Show the welding effect
			if( PSC_Welding != None )
			{
				PSC_Welding.SetHidden( FALSE );
				PSC_Welding.ActivateSystem();
			}

			// Play the welding loop animation
			MainBlendList.SetActiveChild( 4, 0.1f );
		}
		else
		{
			// Otherwise, play the welding intro animation
			MainBlendList.SetActiveChild( 3, 0.2f );
			PlayJackSound( JackWeldingIntroSound, TRUE );
		}
	}
	// Otherwise, done stop welding...
	else
	{
		// Hide the welding effects
		if( PSC_Welding != None )
		{
			PSC_Welding.SetHidden( TRUE );
			PSC_Welding.DeactivateSystem();
		}
		if (PSC_WeldingImpact != None)
		{
			DetachComponent(PSC_WeldingImpact);
			PSC_WeldingImpact.DeactivateSystem();
		}

		// Play the welding outro animation
		MainBlendList.SetActiveChild( 5, 0.2f );
		PlayJackSound( JackWeldingOutroSound, TRUE );
	}

	if( bWelding && bLoop )
	{
		AC_Welding.Play();
	}
	else
	{
		AC_Welding.Stop();
	}
}

function bool SetScanningState( bool bScan )
{
	if( bScan != bScanning )
	{
		bScanning = bScan;
		PlayJackScanningAnimation();
		return TRUE;
	}
	return FALSE;
}

simulated function PlayJackScanningAnimation()
{
	if( bScanning )
	{
		// Play the scan loop animation
		MainBlendList.SetActiveChild( 7, 0.1f );
		AC_Scanning.Play();
	}
	else
	{
		// Play the idle animation
		MainBlendList.SetActiveChild( 0, 0.1f );
		AC_Scanning.Stop();
	}
}

function bool SetPointingState( bool bPoint )
{
	if( bPoint != bPointing )
	{
		bPointing = bPoint;
		PlayJackPointingAnimation();
		return TRUE;
	}
	return FALSE;
}

simulated function PlayJackPointingAnimation()
{
	MainBlendList.SetActiveChild( 8, 0.1f );
	PlayJackSound( JackPointSound, TRUE );
}

function bool Recoil()
{
	if( !bRecoil )
	{
		bRecoil = TRUE;
		RecoilDamageThreshold = default.RecoilDamageThreshold;
		LastRecoilTime = WorldInfo.TimeSeconds;
		PlayJackRecoilAnimation( TRUE );
		return TRUE;
	}
	return FALSE;
}

simulated function PlayJackRecoilAnimation( optional bool bFoldUp )
{
	if( bFoldUp )
	{
		FrontSpotlightComp.SetEnabled( FALSE );
		MainBlendList.SetActiveChild( 6, 0.2f );

		PlayJackSound( JackRecoilSound, TRUE );
	}
	else
	{
		// Unfold from recoil
		MainBlendList.SetActiveChild( 2, 0.2f );
	}
}

simulated function float GetRecoilDuration()
{
	return 1.5f + FRand() * 3.f;
}


simulated function PlayJackSound( SoundCue Sound, optional bool bForce )
{
	if( !bForce && TimeSince(LastJackSoundTime) < 1.f )
		return;

	LastJackSoundTime = WorldInfo.TimeSeconds;
	PlaySound( Sound );
}

function bool ShouldBeStationary()
{
	if( bUnfoldMonitor || IsCloaked() || bCloaking || bWelding || bScanning || bPointing )
	{
		return TRUE;
	}

	return FALSE;
}

function bool ShouldRecoil()
{
	if( bRecoil || bUnfoldMonitor || bWelding || bScanning || bCloaking )
	{
		return FALSE;
	}

	if( GearAI_Jack(Controller).MoveAction != None )
	{
		return FALSE;
	}

	return TRUE;
}

function bool ShouldTakeDamageImpulse()
{
	if( bWelding || bScanning )
	{
		return FALSE;
	}

	if( GearAI_Jack(Controller).MoveAction != None )
	{
		return FALSE;
	}

	return TRUE;
}

/** Stop jack from taking damage. */
event TakeDamage( int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser )
{
	if( TimeSince(LastRecoilTime) > 5.f )
	{
		if( ShouldRecoil() )
		{
			RecoilDamageThreshold -= Damage;
			if( RecoilDamageThreshold <= 0 )
			{
				GearAI_Jack(Controller).DoRecoil();
			}
		}
	}
	else
	{
		// If already wrapped up... extend timer again
		if( bRecoil && IsTimerActive('PlayJackRecoilAnimation') )
		{
			SetTimer( GetRecoilDuration(), FALSE, nameof(PlayJackRecoilAnimation) );
		}

		PlayJackSound( JackSadSound );
	}

	Damage = 0;

	if( ShouldTakeDamageImpulse() )
	{
		Super.TakeDamage( Damage, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, DamageCauser );
	}
}

simulated function UpdateLookSteerStatus()
{
	bUsingLookSteer = FALSE;
}

/** No one drives jack */
simulated function bool CanEnterVehicle(Pawn P);
function bool TryToDrive(Pawn P);
simulated function NotifyTeamChanged();


function PlayIdleSound()
{
	if( VSize(Velocity) > 200.f )
	{
		PlayJackSound( JackIdleMovingVocalSound );
	}
	else
	{
		PlayJackSound( JackIdleStillVocalSound );
	}
}

function AcquiredPOI( JackPointOfInterest POI )
{
	PlayJackSound( JackAcquiredPOISound, TRUE );
}

function InspectingPOI( JackPointOfInterest POI )
{
	PlayJackSound( JackInspectPOISound );
}

native function bool IsTooCloseToPlayerCamera( optional out Vector out_CamLoc, optional out Rotator out_CamRot );

function PancakeOther(Pawn Other);

defaultproperties
{
	bCanFly=TRUE
	bCanWalk=TRUE
	bJumpCapable=FALSE
	bCanJump=FALSE
	bCanSwim=FALSE
	bCanClimbLadders=TRUE
	bCanBeBaseForPawns=FALSE
	bCanStrafe=TRUE
	bTurnInPlace=TRUE
	bNoEncroachCheck=TRUE
	bFollowLookDir=TRUE
	bCanBeFrictionedTo=FALSE
	bModifyNavPointDest=TRUE
	bNeverAValidEnemy=TRUE

	CameraNoRenderCylinder_High=(Radius=88,Height=100)
	CameraNoRenderCylinder_Low=(Radius=88,Height=100)
	CameraNoRenderCylinder_FlickerBuffer=(Radius=10,Height=10)
	bBlockCamera=FALSE

	MantleAdjustHeight=50.f
	StdAdjustHeight=256.f
}
