/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPawn_LocustLeviathanBase extends GearPawn
	native(Pawn)
	abstract;

enum ETentacleStatus
{
	TENTACLE_None,
	TENTACLE_Start,
	TENTACLE_Tell,
	TENTACLE_Attack,
	TENTACLE_Interp,
	TENTACLE_Pause,
	TENTACLE_Retract,
	TENTACLE_Damaged,
	TENTACLE_Wounded,
};

// Skel controllers for mouth tentacles
struct native MouthTentacle
{
	var() Name	SkelCont_MoveName;
	var() Name	SkelCont_SplineName;
	var() SkelControlSingleBone	SkelCont_Move;
	var() SkelControlSpline		SkelCont_Spline;
	var() AnimNodeBlendList		BlendList;

	var() Name					MatParamName;
	var() float					MatTimer;

	var() Actor					Target;
	var() ETentacleStatus		Status;

	var() float					StatusTime;
	var() int					Health;

	var() AudioComponent		AC_TentacleTell;
};

struct native RepMouthTentacle
{
	var() Vector				TargetLocation;
	var() ETentacleStatus		Status;
};

/** List of non replicated information about mouth tentacles */
var(Tentacles)				MouthTentacle		Tentacles[6];
/** Replicated information about mouth tentacles */
var(Tentacles)	repnotify	RepMouthTentacle	RepTentacles[6];

/** Mask for playing animations on separate tentacles */
var(Tentacles)	GearAnim_BlendPerBone			TentacleMask;
/** Amount of health each tentacle has before being hurt */
var(Tentacles)  config int						DefaultTentacleHealth;
/** Number of tentacles hurt during this attack wave */
var(Tentacles)	Byte							NumTentaclesHurt;
/** Amount of time for tentacles to finish attack start anim */
var(Tentacles)		   float					TentacleStartTime;
/** Amount of warm up time before attack */
var(Tentacles)	config float					TentacleWarmUpTime;
/** Amount of time attack takes */
var(Tentacles)	config float					TentacleAttackTime;
/** Amount of time interp takes to reach target */
var(Tentacles)	config float					TentacleInterpTime;
/** Amount of time to pause tentacle after strike */
var(Tentacles)	config float					TentaclePauseTime;
/** Amount of time for tentacle to retract */
var(Tentacles)	config float					TentacleRetractTime;
/** Amount of time for tentacle to retract when hurt */
var(Tentacles)	config float					TentacleHurtTime;
/** Total duration of mouth/tentacles attack before failure */
var(Tentacles)  config float					DurationMouthAttack;
/** Range of time between tentacle attacks */
var(Tentacles)	config Vector2d					DelayBetweenMouthTentacleAttacks;
/** Damage done by tentacle impact */
var(Tentacles)	config float					TentacleImpactDamage;
/** Radius of damage done by tentacle impact */
var(Tentacles)	config float					TentacleImpactRadius;

var(Sound)					float				LastTentacleDamageSoundTime;
var(Sound)	protected const	SoundCue			TentacleTellSound;
var(Sound)  protected const SoundCue			TentacleStartAttackSound;
var(Sound)	protected const SoundCue			TentacleImpactSound;
var(Sound)	protected const SoundCue			TentacleHurtSound;

enum ELeviStatus
{
	LEVISTAT_Closed,
	LEVISTAT_Opening,
	LEVISTAT_Open,
	LEVISTAT_Closing,
};

/** Player has successfully detonated grenade in the mouth */
var(Mouth)					bool				bMouthGrenadeSuccess;
/** Mouth closed before player threw a grenade in it */
var(Mouth)					bool				bMouthClosedFail;
/** Whether mouth is open or not */
var(Mouth)		repnotify	bool				bMouthOpen;
/** Status of mouth */
var(Mouth)					ELeviStatus			MouthStatus;
/** Amount of time before mouth status changes */
var(Mouth)					float				MouthStatusTime;
/** Blend list for controlling the mouth */
var(Mouth)					AnimNodeBlendList	MouthBlendList;
/** Trigger that catches grenade inside mouth */
var(Mouth)		Trigger_LeviathanMouth			MouthTrigger;

/** Whether Jaw is open or not */
var(Jaw)		repnotify	bool				bJawOpen;
/** Status of giant Jaw */
var(Jaw)					ELeviStatus			JawStatus;
/** Amount of time before Jaw status changes */
var(Jaw)					float				JawStatusTime;
/** Blend list for controlling the jaw */
var(Jaw)					AnimNodeBlendList	JawBlendList;
/** Jaw anim notify called */
var(Jaw)					bool				bJawAnimNotifyClosed;

/** Whether Eyes are open or not */
var(Eye)		repnotify	bool				bEyeOpen;
/** Whether Eyes are squeezed shut or not */
var(Eye)		repnotify	bool				bSqueezeEyes;
/** Status of the Eyes */
var(Eye)					ELeviStatus			EyeStatus;
/** Amount of time before Eyes status changes */
var(Eye)					float				EyeStatusTime;
/** Blend list for controlling the Eyes */
var(Eye)					AnimNodeBlendList	RightEyeBlendList, LeftEyeBlendList;

var(Sound)	protected const SoundCue			MouthOpenSound;
var(Sound)	protected const SoundCue			MouthOpenIdleSound;
var(Sound)	protected const SoundCue			MouthShutSound;
var(Sound)	protected const SoundCue			MouthDamagedSound;
var(Sound)	protected const SoundCue			MouthExplosionSound;

/** Effects */
/** Inner mouth opens up... saliva spray, etc */
var(Effects) ParticleSystem						PS_MouthOpen;
var(Effects) ParticleSystemComponent			PSC_MouthOpen;
/** Grenade explodes in inner mouth */
var(Effects) ParticleSystem						PS_MouthGrenDamage;
var(Effects) ParticleSystemComponent			PSC_MouthGrenDamage;
/** Water dripping off inside of mouth */
var(Effects) ParticleSystem						PS_MouthDrip;
var(Effects) ParticleSystemComponent			PSC_MouthDrip;
/** Mucus inside of mouth */
var(Effects) ParticleSystem						PS_MouthMucus;
var(Effects) ParticleSystemComponent			PSC_MouthMucus;
/** Water dripping off face tentacles */
var(Effects) ParticleSystem						PS_FaceTentacleDrip;
var(Effects) ParticleSystemComponent			PSC_FaceTentacleDrip;

var(Effects)	repnotify	bool				bEmergedFromWater;

replication
{
	if( Role == ROLE_Authority )
		bMouthOpen, bMouthGrenadeSuccess, bJawOpen, bEyeOpen, bSqueezeEyes, bEmergedFromWater, RepTentacles;
}

simulated function SetDontModifyDamage( bool NewValue );

simulated event ReplicatedEvent( name VarName )
{
	Super.ReplicatedEvent( VarName );

	switch( VarName )
	{
		case 'bMouthOpen':
			PlayMouthAnim( FALSE );
			break;
		case 'bJawOpen':
			PlayJawAnim( FALSE );
			break;
		case 'bEyeOpen':
			PlayEyeAnim( FALSE );
			break;
		case 'bEmergedFromWater':
			if( bEmergedFromWater )
			{
				PlayEmergeFromWaterEffects();
			}
			break;
		case 'RepTentacles':
			TentacleStatusChanged();
			break;
		default:
			break;
	}
}

simulated function PostBeginPlay()
{
	local int Idx;

	Super.PostBeginPlay();

	if( Role == ROLE_Authority )
	{
		MouthTrigger = Spawn( class'Trigger_LeviathanMouth', self,, Location, Rotation );
		MouthTrigger.SetBase( self,, Mesh, 'b_Mouth_Hole_05');
		MouthTrigger.SetRelativeLocation( vect(0,0,0) );
	}

	PlayMouthAnim( TRUE );
	PlayJawAnim( TRUE );
	PlayEyeAnim( TRUE );

	ActivateParticleSystem( PSC_MouthDrip , PS_MouthDrip , 'MouthCeiling' );
	ActivateParticleSystem( PSC_MouthMucus, PS_MouthMucus, 'MouthMucusBack' );

	for( Idx = 0; Idx < ArrayCount(Tentacles); Idx++ )
	{
		Tentacles[Idx].AC_TentacleTell = CreateAudioComponent(TentacleTellSound, FALSE, TRUE);
		if( Tentacles[Idx].AC_TentacleTell != None )
		{
			Mesh.AttachComponentToSocket( Tentacles[Idx].AC_TentacleTell, '' );
		}
	}
}

simulated function Tick( float DeltaTime )
{
	local int Idx;

	Super.Tick( DeltaTime );

	// Update tentacle status - Only on server
	if( Role == ROLE_Authority )
	{
		for( Idx = 0; Idx < ArrayCount(Tentacles); Idx++ )
		{
			if( Tentacles[Idx].Status != TENTACLE_None )
			{
				Tentacles[Idx].StatusTime -= DeltaTime;
				if( Tentacles[Idx].StatusTime < 0.f )
				{
					switch( Tentacles[Idx].Status )
					{
						case TENTACLE_Start:
							SetTentacleStatus( Idx, TENTACLE_Tell, TentacleWarmUpTime );
							break;
						case TENTACLE_Tell:
							SetTentacleStatus( Idx, TENTACLE_Attack, TentacleAttackTime, Tentacles[Idx].Target.Location + vect(0,0,-48) );
							break;
						case TENTACLE_Attack:
//							SetTentacleStatus( Idx, TENTACLE_Interp, TentacleInterpTime );
//							break;
						case TENTACLE_Interp:
							TentacleImpact( Idx );
							SetTentacleStatus( Idx, TENTACLE_Pause, TentaclePauseTime );
							break;
						case TENTACLE_Pause:
							SetTentacleStatus( Idx, TENTACLE_Retract, TentacleRetractTime );
							break;
						case TENTACLE_Damaged:
							SetTentacleStatus( Idx, TENTACLE_Wounded, TentacleHurtTime );
							break;
						case TENTACLE_Retract:	// fall thru
						case TENTACLE_Wounded:
							SetTentacleStatus( Idx, TENTACLE_None, 0.f );
							break;
					}
				}
			}
		}
	}

	UpdateTentacleEffects( DeltaTime );

	// Update GarbageCan/Mouth status - server & client
	if( MouthStatus == LEVISTAT_Opening || MouthStatus == LEVISTAT_Closing )
	{
		MouthStatusTime -= DeltaTime;
		if( MouthStatusTime < 0.f )
		{
			PlayMouthAnim( TRUE );
		}
	}
	else
	// Otherwise, if enough tentacles hurt... trigger mouth open
	if( Role == ROLE_Authority && 
		!bMouthOpen && 
		GearAI_Leviathan(Controller).CurrentMouthSeq != None && 
		NumTentaclesHurt >= GearAI_Leviathan(Controller).CurrentMouthSeq.NumTentaclesToHurtToOpenMouth )
	{
		OpenMouth( GearAI_Leviathan(Controller).CurrentMouthSeq.Duration );
	}

	// Update Jaw status - server & client
	if( JawStatus == LEVISTAT_Opening || JawStatus == LEVISTAT_Closing )
	{
		JawStatusTime -= DeltaTime;
		if( JawStatusTime < 0.f )
		{
			PlayJawAnim( TRUE );
		}
	}

	// Update Eye Status - server & client
	if( EyeStatus == LEVISTAT_Opening || EyeStatus == LEVISTAT_Closing )
	{
		EyeStatusTime -= DeltaTime;
		if( EyeStatusTime < 0.f )
		{
			PlayEyeAnim( TRUE );
		}
	}

//	FlushPersistentDebugLines();
//	DrawDebugBox( MouthTrigger.Location, vect(40,40,40), 255, 0, 0, TRUE );
}

/** Status change replicated to client... find all the tentacles that have changed and play the new anim */
simulated function TentacleStatusChanged()
{
	local int Idx;

	for( Idx = 0; Idx < ArrayCount(Tentacles); Idx++ )
	{
		if( Tentacles[Idx].Status != RepTentacles[Idx].Status )
		{
			Tentacles[Idx].Status  = RepTentacles[Idx].Status;
			PlayTentacleAnim( Idx );
		}
	}
}

function SetMovementPhysics()
{
	SetPhysics( PHYS_None );
}

simulated function CacheAnimNodes()
{
	local int Idx;
	local String Str;
	local AnimNodeSequence Seq;

	Super.CacheAnimNodes();

	for( Idx = 0; Idx < ArrayCount(Tentacles); Idx++ )
	{
		Tentacles[Idx].SkelCont_Move		= SkelControlSingleBone(Mesh.FindSkelControl(Tentacles[Idx].SkelCont_MoveName));
		Tentacles[Idx].SkelCont_Spline		= SkelControlSpline(Mesh.FindSkelControl(Tentacles[Idx].SkelCont_SplineName));
		if( Tentacles[Idx].SkelCont_Move != None )
		{
			Tentacles[Idx].SkelCont_Move.SetSkelControlActive( TRUE );
			Tentacles[Idx].SkelCont_Move.SetSkelControlStrength( 0.f, 0.f );
		}
		if( Tentacles[Idx].SkelCont_Spline != None )
		{
			Tentacles[Idx].SkelCont_Spline.SetSkelControlActive( TRUE );
			Tentacles[Idx].SkelCont_Spline.SetSkelControlStrength( 0.f, 0.f );
		}

		if( Tentacles[Idx].SkelCont_Move == None || Tentacles[Idx].SkelCont_Spline == None )
		{
			`warn( "FAILED TO FIND SKEL CONTROLLERS FOR LEVIATHAN"@Idx@Tentacles[Idx].SkelCont_MoveName@Tentacles[Idx].SkelCont_Move@Tentacles[Idx].SkelCont_SplineName@Tentacles[Idx].SkelCont_Spline);
		}

		Str = "TentacleBlend_"$Idx;
		Tentacles[Idx].BlendList = AnimNodeBlendList(Mesh.FindAnimNode(Name(Str)));

		if( Tentacles[Idx].BlendList == None )
		{
			`warn( "FAILED TO FIND BLEND LIST FOR TENTACLE"@Idx@Str );
		}
	}

	TentacleMask		= GearAnim_BlendPerBone(Mesh.FindAnimNode('TentacleMask'));
	MouthBlendList		= AnimNodeBlendList(Mesh.FindAnimNode('MouthBlend'));
	JawBlendList		= AnimNodeBlendList(Mesh.FindAnimNode('JawBlend'));
	RightEyeBlendList	= AnimNodeBlendList(Mesh.FindAnimNode('RightEyeBlend'));
	LeftEyeBlendList	= AnimNodeBlendList(Mesh.FindAnimNode('LeftEyeBlend'));

	TentacleStartTime	= Mesh.GetAnimLength( 'M_T_Attack_Start' );
	TentacleWarmUpTime	= (TentacleWarmUpTime > 0) ? float(FCeil( FMax( TentacleWarmUpTime,  0.2f ) / Mesh.GetAnimLength('M_T_Attack_Loop') )) : Mesh.GetAnimLength('M_T_Attack_Loop');
	TentacleAttackTime	= (TentacleAttackTime > 0) ? FMax( TentacleAttackTime, 0.2f ) : Mesh.GetAnimLength('M_T_Attack');
	TentacleInterpTime  = FMax( TentacleInterpTime,  0.2f );
	TentaclePauseTime	= FMax( TentaclePauseTime,	 0.2f );
	TentacleRetractTime = (TentacleRetractTime > 0) ? FMax( TentacleRetractTime, 0.2f ) : Mesh.GetAnimLength('M_T_Attack_Retract');
	TentacleHurtTime	= FMax( TentacleHurtTime, 0.2f );

	foreach Mesh.AllAnimNodes( class'AnimNodeSequence', Seq )
	{
		if( Seq.NodeName == 'Attack' )
		{
			Seq.Rate = Mesh.GetAnimRateByDuration( 'M_T_Attack', TentacleAttackTime );
			TentacleInterpTime *= (1.f / Seq.Rate);
		}
		if( Seq.NodeName == 'Retract' )
		{
			Seq.Rate = Mesh.GetAnimRateByDuration( 'M_T_Attack_Retract', TentacleRetractTime );
		}
	}
}

simulated function ClearAnimNodes()
{
	local int Idx;

	Super.ClearAnimNodes();

	for( Idx = 0; Idx < ArrayCount(Tentacles); Idx++ )
	{
		Tentacles[Idx].SkelCont_Move	= None;
		Tentacles[Idx].SkelCont_Spline	= None;
	}
}

function bool IsTentacleAvailable( int Idx )
{
	if( Tentacles[Idx].Status != TENTACLE_None )
	{
		return FALSE;
	}

	// Don't use middle tentacles if mouth is open to avoid clipping
	if( bMouthOpen && (Idx == 1 || Idx == 4) )
	{
		return FALSE;
	}


	return TRUE;
}

function bool AnyTentaclesAttacking()
{
	local int Idx;

	for( Idx = 0; Idx < ArrayCount(Tentacles); Idx++ )
	{
		if( !IsTentacleAvailable( Idx ) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

function int GetNextAttackTentacle()
{
	local int Idx;
	local array<int> Avail;

	for( Idx = 0; Idx < ArrayCount(Tentacles); Idx++ )
	{
		if( IsTentacleAvailable( Idx ) )
		{
			Avail[Avail.Length] = Idx;
		}
	}

	if( Avail.Length == 0 )
	{
		return -1;
	}

	return Avail[Rand(Avail.Length)];
}

function SetTentacleStatus( int Idx, ETentacleStatus NewStatus, float NewTime, optional Vector TargLoc )
{
	Tentacles[Idx].Status		= NewStatus;
	Tentacles[Idx].StatusTime	= NewTime;

	// Change replicated status so clients know
	RepTentacles[Idx].Status		 = NewStatus;
	RepTentacles[Idx].TargetLocation = TargLoc;

	PlayTentacleAnim( Idx );

	//debug
	`AILog_Ext( GetFuncName()@Idx@NewStatus@NewTime@TargLoc, '', MyGearAI );
}

// Handles playing all the mouth tentacle animations by state
simulated function PlayTentacleAnim( int Idx );
simulated function UpdateTentacleEffects( float DeltaTime );

simulated function StartTentacleAttack( int Idx, Actor inTarget )
{
	if( bMouthOpen || inTarget == None || !IsTentacleAvailable( Idx ) )
	{
		return;
	}

	Tentacles[Idx].Target	= inTarget;
	Tentacles[Idx].Health	= DefaultTentacleHealth;
	SetTentacleStatus( Idx, TENTACLE_Start, TentacleStartTime );
}

simulated function PlayAttackSound()
{
	PlaySound( TentacleStartAttackSound, TRUE );
}

function TentacleImpact( int Idx )
{
	//debug
	`AILog_Ext( GetFuncName()@TentacleImpactDamage@TentacleImpactRadius@RepTentacles[Idx].TargetLocation, '', MyGearAI );

	HurtRadius( TentacleImpactDamage, TentacleImpactRadius, class'GDT_Leviathan_Tentacle', 0.f, RepTentacles[Idx].TargetLocation,,, TRUE );
	PlaySound( TentacleImpactSound );
}

function StopAllTentacleAttacks()
{
	local int Idx;

	for( Idx = 0; Idx < ArrayCount(Tentacles); Idx++ )
	{
		if( Tentacles[Idx].Status != TENTACLE_None	  &&
			Tentacles[Idx].Status != TENTACLE_Damaged &&
			Tentacles[Idx].Status != TENTACLE_Wounded &&
			Tentacles[Idx].Status != TENTACLE_Retract )
		{
			SetTentacleStatus( Idx, TENTACLE_Retract, TentacleRetractTime );
		}
	}
}

function OpenEyes( float Duration )
{
	bEyeOpen	 = TRUE;
	bSqueezeEyes = FALSE;
	PlayEyeAnim( FALSE );
	SetTimer( Duration, FALSE, nameof(CloseEyes) );
}

function CloseEyes( optional bool bSqueeze )
{
	bEyeOpen	 = FALSE;
	bSqueezeEyes = bSqueeze;
	PlayEyeAnim( FALSE );
	ClearTimer( GetFuncName() );

	// Clear latent action... Abort if not shot in eyes
	Controller.ClearLatentAction( class'SeqAct_Leviathan_Eyes', !bSqueezeEyes );
}

simulated function PlayEyeAnim( bool bIdle )
{
	if( bEyeOpen )
	{
		if( bIdle )
		{
			EyeStatus = LEVISTAT_Open;
			RightEyeBlendList.SetActiveChild( 6, 0.1f );
			LeftEyeBlendList.SetActiveChild( 6, 0.1f );
		}
		else
		{
			EyeStatus = LEVISTAT_Opening;
			EyeStatusTime = Mesh.GetAnimLength( (bSqueezeEyes?'E_Right_S_Open':'E_Right_R_Open') );
			RightEyeBlendList.SetActiveChild( (bSqueezeEyes?4:1), 0.1f );
			LeftEyeBlendList.SetActiveChild( (bSqueezeEyes?4:1), 0.1f );
		}
	}
	else
	{
		if( bIdle )
		{
			EyeStatus = LEVISTAT_Closed;
			RightEyeBlendList.SetActiveChild( (bSqueezeEyes?3:0), 0.1f );
			LeftEyeBlendList.SetActiveChild( (bSqueezeEyes?3:0), 0.1f );
		}
		else
		{
			EyeStatus = LEVISTAT_Closing;
			EyeStatusTime = Mesh.GetAnimLength( (bSqueezeEyes?'E_Right_S_Close':'E_Right_R_Close') );
			RightEyeBlendList.SetActiveChild( (bSqueezeEyes?5:2), 0.1f );
			LeftEyeBlendList.SetActiveChild( (bSqueezeEyes?5:2), 0.1f );
		}
	}
}

function OpenJaw( float Duration )
{
	bJawOpen = TRUE;
	bJawAnimNotifyClosed = FALSE;
	PlayJawAnim( FALSE );
	SetTimer( Duration, FALSE, nameof(CloseJaw) );
}

function CloseJaw()
{
	bJawOpen = FALSE;
	PlayJawAnim( FALSE );
}

function AnimNotify_JawClosed()
{
	bJawAnimNotifyClosed = TRUE;
}

simulated function PlayJawAnim( bool bIdle )
{
	local AnimNodeSequence JawOpenAnimNode;
	if( bJawOpen )
	{
		if( bIdle )
		{
			JawStatus = LEVISTAT_Open;
			JawBlendList.SetActiveChild( 2, 0.1f );
		}
		else
		{
			JawOpenAnimNode = AnimNodeSequence(JawBlendList.Children[1].Anim);
			JawStatusTime = JawOpenAnimNode.GetAnimPlaybackLength();
			JawStatus = LEVISTAT_Opening;
			JawBlendList.SetActiveChild( 1, 0.1f );
		}
	}
	else
	{
		if( bIdle )
		{
			JawStatus = LEVISTAT_Closed;
			JawBlendList.SetActiveChild( 0, 0.1f );
		}
		else
		{
			JawOpenAnimNode = AnimNodeSequence(JawBlendList.Children[3].Anim);
			JawStatusTime = JawOpenAnimNode.GetAnimPlaybackLength();
			JawStatus = LEVISTAT_Closing;
			JawBlendList.SetActiveChild( 3, 0.1f );
		}
	}
}

function OpenMouth( float Duration )
{
	bMouthOpen		 = TRUE;
	bMouthClosedFail = FALSE;
	SetTimer( Duration, FALSE, nameof(CloseMouth) );

	if( GearAI_Leviathan(Controller).ValidMouthTargets.Length < 2 )
	{
		StopAllTentacleAttacks();
	}

	PlayMouthAnim( FALSE );

	TriggerEventClass( class'SeqEvt_Leviathan_MouthOpen', self );
}

function CloseMouth()
{
	bMouthOpen		 = FALSE;
	bMouthClosedFail = TRUE;
	NumTentaclesHurt = 0;

	PlayMouthAnim( FALSE );

	ClearTimer( 'CloseMouth' );
}

function HurtMouth()
{
	bMouthOpen			 = FALSE;
	bMouthClosedFail	 = FALSE;
	bMouthGrenadeSuccess = TRUE;
	NumTentaclesHurt	 = 0;

	PlayMouthAnim( FALSE );

	ClearTimer( 'CloseMouth' );
}

function bool HandleGrenadeTouch( GearProj_Grenade Gren, Trigger_LeviathanMouth Trig )
{
	if( !bMouthOpen || Gren == None )
		return FALSE;

	if( Trig == None )
	{
		foreach Gren.VisibleCollidingActors( class'Trigger_LeviathanMouth', Trig, Gren.DamageRadius, Gren.Location )
		{
			break;
		}
	}

	if( Trig != None )
	{
		HurtMouth();
		return TRUE;
	}

	return FALSE;
}

simulated function PlayMouthAnim( bool bIdle )
{
	if( bMouthOpen )
	{
		if( bIdle )
		{
			MouthStatus		= LEVISTAT_Open;
			MouthBlendList.SetActiveChild( 2, 0.1f );

			PlaySound( MouthOpenIdleSound, TRUE );
		}
		else
		{
			MouthStatus		= LEVISTAT_Opening;
			MouthStatusTime	= Mesh.GetAnimLength( 'M_G_Open' );
			MouthBlendList.SetActiveChild( 1, 0.1f );

			PlaySound( MouthOpenSound, TRUE );
			ActivateParticleSystem( PSC_MouthOpen, PS_MouthOpen, 'MouthEffect' );
		}
	}
	else
	{
		if( bIdle )
		{
			MouthStatus		= LEVISTAT_Closed;
			MouthBlendList.SetActiveChild( 0, 0.1f );
		}
		else
		{
			MouthStatus		= LEVISTAT_Closing;
			MouthStatusTime = Mesh.GetAnimLength( 'M_G_Close' );
			MouthBlendList.SetActiveChild( 3, 0.1f );

			if( bMouthGrenadeSuccess )
			{
				SetTimer( 0.25f, FALSE, 'PlayMouthDamageEffects' );
			}
			else
			{
				PlaySound( MouthShutSound, TRUE );
			}

			DeactivateParticleSystem( PSC_MouthOpen );
		}
	}
}

simulated function PlayMouthDamageEffects()
{
	PlaySound( MouthDamagedSound, TRUE );
	PlaySound( MouthExplosionSound, TRUE );
	ActivateParticleSystem( PSC_MouthGrenDamage, PS_MouthGrenDamage, 'MouthEffect' );
}

simulated function ActivateParticleSystem( out ParticleSystemComponent PSC, ParticleSystem PS, Name SocketName )
{
	if( SocketName == '' )
		return;

	if( PSC == None )
	{
		PSC = new(self) class'ParticleSystemComponent';
		PSC.SetTemplate( PS );
		PSC.SetTickGroup(TG_PostUpdateWork);
		Mesh.AttachComponentToSocket( PSC, SocketName );
	}
	if( PSC != None )
	{
		PSC.SetHidden( FALSE );
		PSC.ActivateSystem(TRUE);
	}
}

simulated function DeactivateParticleSystem( ParticleSystemComponent PSC )
{
	if( PSC != None )
	{
		PSC.DeactivateSystem();
	}
}

/** Handle taking damage in various body locations */
function AdjustPawnDamage
(
	out	int					Damage,
	Pawn					InstigatedBy,
	Vector					HitLocation,
	out Vector				Momentum,
	class<GearDamageType>	GearDamageType,
	optional	out	TraceHitInfo		HitInfo
)
{
	local int Idx;

	//debug
	`log( GetFuncName()@Damage@HitInfo.BoneName@GetTentacleIdxByBoneName( HitInfo.BoneName ), bDebug );

	Idx = GetTentacleIdxByBoneName( HitInfo.BoneName );
	if( Idx >= 0 &&
		(Tentacles[Idx].Status == TENTACLE_Tell	  ||
		 Tentacles[Idx].Status == TENTACLE_Attack ||
		 Tentacles[Idx].Status == TENTACLE_Interp ||
		 Tentacles[Idx].Status == TENTACLE_Pause) )
	{
		Tentacles[Idx].Health -= Damage;
		if( Tentacles[Idx].Health <= 0 )
		{
			NumTentaclesHurt++;
			SetTentacleStatus( Idx, TENTACLE_Damaged, Mesh.GetAnimLength('M_T_Hit') );
			PlaySound( TentacleHurtSound );
		}
	}

	PlayTakeHit( Damage, InstigatedBy, HitLocation, GearDamageType, Momentum, HitInfo );

	Damage = 0;
}

function int GetTentacleIdxByBoneName( Name InBone )
{
	switch( InBone )
	{
		case 'b_Mouth_Tntcl_L_A_05':
		case 'b_Mouth_Tntcl_L_A_06':
		case 'b_Mouth_Tntcl_L_A_07':
		case 'b_Mouth_Tntcl_L_A_08':
		case 'b_Mouth_Tntcl_L_A_09':
		case 'b_Mouth_Tntcl_L_A_10':
		case 'b_Mouth_Tntcl_L_A_11':
		case 'b_Mouth_Tntcl_L_A_End':
			return 0;
		case 'b_Mouth_Tntcl_L_B_05':
		case 'b_Mouth_Tntcl_L_B_06':
		case 'b_Mouth_Tntcl_L_B_07':
		case 'b_Mouth_Tntcl_L_B_08':
		case 'b_Mouth_Tntcl_L_B_09':
		case 'b_Mouth_Tntcl_L_B_10':
		case 'b_Mouth_Tntcl_L_B_11':
		case 'b_Mouth_Tntcl_L_B_End':
			return 1;
		case 'b_Mouth_Tntcl_L_C_05':
		case 'b_Mouth_Tntcl_L_C_06':
		case 'b_Mouth_Tntcl_L_C_07':
		case 'b_Mouth_Tntcl_L_C_08':
		case 'b_Mouth_Tntcl_L_C_09':
		case 'b_Mouth_Tntcl_L_C_10':
		case 'b_Mouth_Tntcl_L_C_11':
		case 'b_Mouth_Tntcl_L_C_End':
			return 2;
		case 'b_Mouth_Tntcl_L_D_05':
		case 'b_Mouth_Tntcl_L_D_06':
		case 'b_Mouth_Tntcl_L_D_07':
		case 'b_Mouth_Tntcl_L_D_08':
		case 'b_Mouth_Tntcl_L_D_09':
		case 'b_Mouth_Tntcl_L_D_10':
		case 'b_Mouth_Tntcl_L_D_11':
		case 'b_Mouth_Tntcl_L_D_End':
			return 3;
		case 'b_Mouth_Tntcl_L_E_05':
		case 'b_Mouth_Tntcl_L_E_06':
		case 'b_Mouth_Tntcl_L_E_07':
		case 'b_Mouth_Tntcl_L_E_08':
		case 'b_Mouth_Tntcl_L_E_09':
		case 'b_Mouth_Tntcl_L_E_10':
		case 'b_Mouth_Tntcl_L_E_11':
		case 'b_Mouth_Tntcl_L_E_End':
			return 4;
		case 'b_Mouth_Tntcl_L_F_05':
		case 'b_Mouth_Tntcl_L_F_06':
		case 'b_Mouth_Tntcl_L_F_07':
		case 'b_Mouth_Tntcl_L_F_08':
		case 'b_Mouth_Tntcl_L_F_09':
		case 'b_Mouth_Tntcl_L_F_10':
		case 'b_Mouth_Tntcl_L_F_11':
		case 'b_Mouth_Tntcl_L_F_End':
			return 5;
	}

	return -1;
}

simulated function bool HitAttackingTentacle( TraceHitInfo HitInfo )
{
	local int Idx;

	Idx = GetTentacleIdxByBoneName( HitInfo.BoneName );
	if( Idx >= 0 )
	{
		if( RepTentacles[Idx].Status == TENTACLE_Start	||
			RepTentacles[Idx].Status == TENTACLE_Tell	||
			RepTentacles[Idx].Status == TENTACLE_Attack ||
			RepTentacles[Idx].Status == TENTACLE_Interp ||
			RepTentacles[Idx].Status == TENTACLE_Pause	)
		{
			return TRUE;
		}
	}

	return FALSE;
}

simulated function bool ShouldTurnCursorRedFor(GearPC PC)
{
	return FALSE;
}

function EmergeFromWater()
{
	bEmergedFromWater = TRUE;
	PlayEmergeFromWaterEffects();
}

function ResetEmergeFlag()
{
	bEmergedFromWater = FALSE;
}

simulated function PlayEmergeFromWaterEffects();


defaultproperties
{
	Tentacles(0)=(SkelCont_MoveName=LeftTop_Move,SKelCont_SplineName=LeftTop_Spline,MatParamName=R_Tent1)
	Tentacles(1)=(SkelCont_MoveName=LeftMid_Move,SKelCont_SplineName=LeftMid_Spline,MatParamName=R_Tent2)
	Tentacles(2)=(SkelCont_MoveName=LeftBot_Move,SKelCont_SplineName=LeftBot_Spline,MatParamName=R_Tent3)
	Tentacles(3)=(SkelCont_MoveName=RightTop_Move,SKelCont_SplineName=RightTop_Spline,MatParamName=L_Tent1)
	Tentacles(4)=(SkelCont_MoveName=RightMid_Move,SKelCont_SplineName=RightMid_Spline,MatParamName=L_Tent2)
	Tentacles(5)=(SkelCont_MoveName=RightBot_Move,SKelCont_SplineName=RightBot_Spline,MatParamName=L_Tent3)

	bRespondToExplosions=FALSE
	bCanPlayHeadShotDeath=FALSE
	bCanPlayPhysicsHitReactions=FALSE
	bCanDBNO=FALSE
	bCanRecoverFromDBNO=FALSE
	bBlockCamera=FALSE
	bCollideWorld=FALSE
	bCollideActors=TRUE
	bBlockActors=FALSE
	bIgnoreEncroachers=TRUE
	bNeverAValidEnemy=TRUE
	bInvalidMeleeTarget=TRUE
	bUpdateSimulatedPosition=false
	bReplicateMovement=false
	bNoDelete=true

	SightRadius=0.f
	HearingThreshold=0.f

	PelvisBoneName=None
	PickupFocusBoneName=None
	PickupFocusBoneNameKickup=None

	SupportedEvents.Add(class'GearGame.SeqEvt_Leviathan_MouthOpen')

//	bDebug=TRUE
}
