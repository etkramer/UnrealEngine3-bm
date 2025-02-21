/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SkeletalMeshActor_RiftwormAcidJet extends SkelMeshActor_Unfriendly
	placeable;

var()	SkeletalMeshComponent	Mesh;
var		AnimNodeBlendList		MainBlendList;

enum EAcidJetState
{
	AJS_Inhale,
	AJS_Exhale,
	AJS_Burst,
};
/** Current state of the jet */
var()	repnotify EAcidJetState			AcidJetState;
/** Wait time before initializing */
var()	float							TimeBeforeInit;

/** Height of damage cone */
var()	float							ConeHeight;
/** Radius of damage cone (radians) */
var()	float							ConeFOV;
/** Inner cone  */
var()	float							InnerConeFOV;
/** Amount of time between damage ticks */
var()	float							DamageStepTime;



/** How much to scale the tip mesh */
var()	float							TipMeshScale;

/** Amount of damage jet takes before it bursts */
var()	int								BurstDamageThreshold;
/** Current health level */
var		int								Health;
/** Amount of time before can burst again */
var()	float							MinTimeBetweenBurst;
/** Next time allowed to burst */
var		float							NextBurstAllowedTime;

/** Effects */
var		MaterialInstanceTimeVarying		MITV;
var		InterpCurveFloat				InhalePoints, ExhalePoints, BurstPoints;
var		ParticleSystem					PS_Spray;
var		ParticleSystemComponent			PSC_Spray;
//var		ParticleSystem					PS_Burst;
//var		ParticleSystemComponent			PSC_Burst;

var		MaterialInstanceConstant		Tip_MIC;
var()	StaticMeshComponent				SMC_TipMesh;

/** Holds the ambient spotlight */
var AudioComponent AC_ShowerSound;

struct AcidJetDifficultySettings
{
	/** How quickly the squirting anim plays */
	var()	float							ExhaleAnimRate;
	/** How quickly the inhale anim plays */
	var()	float							InhaleAnimRate;
	/** Damage per second inside outer cone */
	var()	float							DamagePerSecondInCone;
	/** Damage per second inside inner cone */
	var()	float							DamagePerSecondInInnerCone;
};

var()	AcidJetDifficultySettings	CasualSettings;
var()	AcidJetDifficultySettings	NormalSettings;
var()	AcidJetDifficultySettings	HardcoreSettings;
var()	AcidJetDifficultySettings	InsaneSettings;



replication
{
	if (bNetDirty)
		AcidJetState;
}

simulated function PostBeginPlay()
{
	local InterpCurvePointFloat Point;
	local AnimNodeSequence ExhaleAnimNode, InhaleAnimNode;
	local AcidJetDifficultySettings Settings;

	Super.PostBeginPlay();

	if( Mesh != None )
	{
		Settings = GetDifficultySettings();

		// Attach glowy bit to tip
		Tip_MIC = new(outer) class'MaterialInstanceConstant';
		Tip_MIC.SetParent(MaterialInterface'worm_hazards.Materials.M_Worm_Sprayer_Tip_Glowy_01');
		Tip_MIC.SetScalarParametervalue('ShowPulsing', 0.0);

		SMC_TipMesh.SetScale(TipMeshScale);
		SMC_TipMesh.SetMaterial(0, Tip_MIC);
		Mesh.AttachComponentToSocket(SMC_TipMesh, 'TipMesh');

		Mesh.AttachComponentToSocket(AC_ShowerSound, 'Cone');

		// Allow override of anim rates per instance.
		ExhaleAnimNode = AnimNodeSequence(Mesh.FindAnimNode('ExhaleAnim'));
		ExhaleAnimNode.Rate = Settings.ExhaleAnimRate;

		InhaleAnimNode = AnimNodeSequence(Mesh.FindAnimNode('InhaleAnim'));
		InhaleAnimNode.Rate = Settings.InhaleAnimRate;

		// Get main blending node
		MainBlendList = AnimNodeBlendList(Mesh.FindAnimNode('MainBlendList'));

		// Setup points for material
		Point.InVal  = 0.f;
		Point.OutVal = 0.f;
		InhalePoints.Points[InhalePoints.Points.length] = Point;
		Point.InVal  = 1.f;
		Point.OutVal = 1.f;
		InhalePoints.Points[InhalePoints.Points.length] = Point;

		Point.InVal  = 0.f;
		Point.OutVal = 1.f;
		ExhalePoints.Points[ExhalePoints.Points.length] = Point;
		Point.InVal  = 1.f;
		Point.OutVal = 0.f;
		ExhalePoints.Points[ExhalePoints.Points.length] = Point;

		Point.InVal  = 0.f;
		Point.OutVal = 0.f;
		BurstPoints.Points[BurstPoints.Points.length] = Point;
		Point.InVal  = 1.f;
		Point.OutVal = 0.f;
		BurstPoints.Points[BurstPoints.Points.length] = Point;

		MITV = new(outer) class'MaterialInstanceTimeVarying';
		MITV.SetParent( MaterialInterface'worm_hazards.AM.Prefab.M_WORM_Hazards_Gaser01_INST' );
		Mesh.SetMaterial( 0, MITV );
	}

	// Init state
	if( TimeBeforeInit > 0.f )
	{
		SetTimer( TimeBeforeInit, FALSE, nameof(Init) );
	}
	else
	{
		Init();
	}
}

/** Returns settings to use based on lwoest player difficulty */
simulated function AcidJetDifficultySettings GetDifficultySettings()
{
	local EDifficultyLevel MinDiff;

	// Get lowest difficulty level
	MinDiff = class'DifficultySettings'.static.GetLowestPlayerDifficultyLevel(WorldInfo).default.DifficultyLevel;

	// Get settings for that difficulty
	if(MinDiff == DL_Casual)
	{
		//`log("ACIDJET: CASUAL");
		return CasualSettings;
	}
	else if(MinDiff == DL_Normal)
	{
		//`log("ACIDJET: NORMAL");
		return NormalSettings;
	}
	else if(MinDiff == DL_Hardcore)
	{
		//`log("ACIDJET: HARDCORE");
		return HardcoreSettings;
	}
	else
	{
		//`log("ACIDJET: INSANE");
		return InsaneSettings;
	}
}

simulated function Init()
{
	SetAcidJetState( AJS_EXHALE );
}


simulated event ReplicatedEvent( name VarName )
{
	switch( VarName )
	{
	case 'AcidJetState':
		PlayAcidJetAnimation();
		break;
	}

	Super.ReplicatedEvent( VarName );
}

simulated event OnAnimEnd( AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime )
{
	if( SeqNode.AnimSeqName == 'Inhale' && AcidJetState == AJS_Inhale )
	{
		SetAcidJetState( AJS_Exhale );
	}
	else
	if( SeqNode.AnimSeqName == 'Exhale' && AcidJetState == AJS_Exhale )
	{
		//SetAcidJetState( AJS_Inhale );
	}
	else
	if( SeqNode.AnimSeqName == 'Burst' && AcidJetState == AJS_Burst )
	{
		SetAcidJetState( AJS_Inhale );
	}
}

simulated function SetAcidJetState( EAcidJetState NewState )
{
	AcidJetState = NewState;
	bForceNetUpdate = true;

	if( AcidJetState == AJS_Inhale )
	{
		TriggerEventClass( class'SeqEvt_RiftwormAcidJet', self, 0 );
	}
	// Reset health
	if( AcidJetState == AJS_Exhale )
	{
		TriggerEventClass( class'SeqEvt_RiftwormAcidJet', self, 1 );
		Health = BurstDamageThreshold;
	}
	else
	if( AcidJetState == AJS_Burst )
	{
		NextBurstAllowedTime = WorldInfo.TimeSeconds + MinTimeBetweenBurst;
		TriggerEventClass( class'SeqEvt_RiftwormAcidJet', self, 2 );
	}

	if( Role == ROLE_Authority )
	{
		// Start checking for damage
		if( AcidJetState == AJS_Exhale )
		{
			SetTimer( DamageStepTime, TRUE, nameof(CheckForDamageInCone) );
		}
		// Otherwise, stop checking for damage
		else
		{
			ClearTimer( 'CheckForDamageInCone' );
		}
	}

	PlayAcidJetAnimation();
}

simulated function PlayAcidJetAnimation()
{
	local float Duration;
	local vector TipLoc;
	local rotator TipRot;

	Mesh.GetSocketWorldLocationAndRotation('TipMesh', TipLoc, TipRot);

	if( PSC_Spray == None )
	{
		PSC_Spray = new(self) class'ParticleSystemComponent';
		PSC_Spray.SetTemplate( PS_Spray );
		PSC_Spray.SetHidden( FALSE );
		Mesh.AttachComponentToSocket( PSC_Spray, 'Cone' );
	}
	//if( PSC_Burst == None )
	//{
	//	PSC_Burst = new(self) class'ParticleSystemComponent';
	//	PSC_Burst.SetTemplate( PS_Burst );
	//	PSC_Burst.SetHidden( FALSE );
	//	Mesh.AttachComponentToSocket( PSC_Burst, 'Cone' );
	//}

	if( AcidJetState == AJS_Inhale )
	{
		MainBlendList.SetActiveChild( 0, 0.25f );
		Duration = Mesh.GetAnimLength( 'Inhale' );

		Tip_MIC.SetScalarParametervalue('ShowPulsing', 0.0);

		if( PSC_Spray != None )
		{
			PSC_Spray.DeactivateSystem();
		}

		PlaySound(SoundCue'Foley_Flesh.AcidSprayer.AcidSprayer_ChargingCue', TRUE, TRUE, TRUE, TipLoc, TRUE);

		if (AC_ShowerSound.IsPlaying())
		{
			AC_ShowerSound.Stop();
		}

		//if( PSC_Burst != None )
		//{
		//	PSC_Burst.DeactivateSystem();
		//}

		MITV.SetScalarCurveParameterValue( 'GasVisibility', InhalePoints);
	}
	else
	if( AcidJetState == AJS_Exhale )
	{
		MainBlendList.SetActiveChild( 1, 0.25f );
		Duration = 1.0; Mesh.GetAnimLength( 'Exhale' );

		Tip_MIC.SetScalarParametervalue('ShowPulsing', 1.0);

		if( PSC_Spray != None )
		{
			PSC_Spray.ActivateSystem();
		}

		if (!AC_ShowerSound.IsPlaying())
		{
			PlaySound(SoundCue'Foley_Flesh.AcidSprayer.AcidSprayer_SprayStartCue', TRUE, TRUE, TRUE, TipLoc, TRUE);
			AC_ShowerSound.Play();
		}

		//if( PSC_Burst != None )
		//{
		//	PSC_Burst.DeactivateSystem();
		//}

		MITV.SetScalarCurveParameterValue( 'GasVisibility', ExhalePoints);
	}
	else
	if( AcidJetState == AJS_Burst )
	{
		MainBlendList.SetActiveChild( 2, 0.5f );
		Duration = Mesh.GetAnimLength( 'Burst' );

		Tip_MIC.SetScalarParametervalue('ShowPulsing', 0.0);

		if( PSC_Spray != None )
		{
			PSC_Spray.DeactivateSystem();
		}

		PlaySound(SoundCue'Foley_Flesh.AcidSprayer.AcidSprayer_HitReactionCue', TRUE, TRUE, TRUE, TipLoc, TRUE);

		if (AC_ShowerSound.IsPlaying())
		{
			AC_ShowerSound.Stop();
		}

		//if( PSC_Burst != None )
		//{
		//	PSC_Burst.ActivateSystem();
		//}
		MITV.SetScalarCurveParameterValue( 'GasVisibility', BurstPoints);
	}

	//`log( GetFuncName()@Duration@AcidJetState );

	MITV.SetDuration( Duration );
}

function CheckForDamageInCone()
{
	local Pawn		P;
	local Vector	SocketLoc;
	local Rotator	SocketRot;
	local float		DotP, Angle;
	local AcidJetDifficultySettings Settings;

	Mesh.GetSocketWorldLocationAndRotation( 'Cone', SocketLoc, SocketRot );

	Settings = GetDifficultySettings();

	foreach WorldInfo.AllPawns( class'Pawn', P, SocketLoc, ConeHeight )
	{
		DotP = Vector(SocketRot) DOT Normal(P.Location-SocketLoc);
		Angle = ACos(DotP); // convert to radians

		if( Angle < InnerConeFOV )
		{
			P.TakeDamage( (Settings.DamagePerSecondInInnerCone * DamageStepTime), None, P.Location, vect(0,0,0), class'GDT_AcidJet',, self );
		}
		else if( Angle < ConeFOV )
		{
			P.TakeDamage( (Settings.DamagePerSecondInCone * DamageStepTime), None, P.Location, vect(0,0,0), class'GDT_AcidJet',, self );
		}
	}
}

event TakeDamage(int DamageAmount, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	Super.TakeDamage( DamageAmount, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, DamageCauser );

	// When jet is inhaling, it can take damage
	if( (HitInfo.HitComponent == SMC_TipMesh) && (AcidJetState == AJS_Exhale) && (WorldInfo.TimeSeconds >= NextBurstAllowedTime) )
	{
		// Update health
		Health -= DamageAmount;
		// If took enough damage
		if( Health <= 0 )
		{
			// Burst
			SetAcidJetState( AJS_Burst );
		}
	}
}

/** Make cursor go red when over tip mesh */
simulated function bool HitAreaIsUnfriendly(TraceHitInfo HitInfo)
{
	return (HitInfo.HitComponent == SMC_TipMesh);
}

`if(`notdefined(FINAL_RELEASE))
simulated function Tick( float DeltaTime )
{
	local Vector	SocketLoc;
	local Rotator	SocketRot;
	local Color		Red, Green;

	Super.Tick( DeltaTime );

	if( bDebug )
	{
		Mesh.GetSocketWorldLocationAndRotation( 'Cone', SocketLoc, SocketRot );

		Red.R = 255;
		Red.A = 255;
		DrawDebugCone( SocketLoc, Vector(SocketRot), ConeHeight, ConeFOV, ConeFOV, 10, Red );

		Green.G = 255;
		Green.A = 255;
		DrawDebugCone( SocketLoc, Vector(SocketRot), ConeHeight, InnerConeFOV, InnerConeFOV, 10, Green );
	}
}
`endif

defaultproperties
{
	CasualSettings=(InhaleAnimRate=1.0,ExhaleAnimRate=1.0,DamagePerSecondInCone=100.0,DamagePerSecondInInnerCone=200.0)
	NormalSettings=(InhaleAnimRate=1.0,ExhaleAnimRate=1.0,DamagePerSecondInCone=100.0,DamagePerSecondInInnerCone=200.0)
	HardcoreSettings=(InhaleAnimRate=1.0,ExhaleAnimRate=1.0,DamagePerSecondInCone=100.0,DamagePerSecondInInnerCone=200.0)
	InsaneSettings=(InhaleAnimRate=1.0,ExhaleAnimRate=1.0,DamagePerSecondInCone=100.0,DamagePerSecondInInnerCone=200.0)

	ConeHeight=768.f
	ConeFOV=0.7071
	InnerConeFOV=0.4
	DamageStepTime=0.25f

	BurstDamageThreshold=100
	MinTimeBetweenBurst=3.f


	TipMeshScale=3.0

	Begin Object Name=SkeletalMeshComponent0
		SkeletalMesh=SkeletalMesh'worm_hazards.AM.Mesh.A_WORM_Hazards_Gaser02'
		PhysicsAsset=PhysicsAsset'worm_hazards.AM.Mesh.A_WORM_Hazards_Gaser02_Physics'
		AnimTreeTemplate=AnimTree'worm_hazards.AM.Animations.WORM_Hazards_Gaser_Tree'
		AnimSets(0)=AnimSet'worm_hazards.AM.Animations.WORM_Hazards_Gaser_Anims2'

		CastShadow=TRUE
		bCastDynamicShadow=TRUE
		bAcceptsStaticDecals=FALSE
		bAcceptsDynamicDecals=FALSE

		BlockActors=TRUE
		BlockZeroExtent=TRUE
		BlockRigidBody=TRUE
		BlockNonzeroExtent=TRUE
		CollideActors=TRUE
	End Object
	Mesh=SkeletalMeshComponent0

	PS_Spray=ParticleSystem'dp_riftwormgags.FX.P_RiftWorm_Acidspray_Prefab_01'
	//PS_Burst=ParticleSystem'dp_riftwormgags.FX.P_RiftWorm_Acidspray_Prefab_02'

	Begin Object Class=StaticMeshComponent Name=TipMeshComp0
		StaticMesh=StaticMesh'GOW_CaveFoliage.SM.Mesh.S_GOW_CaveFoliage_WormFood_Top01'
		bCastDynamicShadow=FALSE
		bAcceptsStaticDecals=FALSE
		bAcceptsDynamicDecals=FALSE

		BlockActors=FALSE
		BlockZeroExtent=TRUE
		BlockRigidBody=FALSE
		BlockNonzeroExtent=FALSE
		CollideActors=TRUE
	End Object
	SMC_TipMesh=TipMeshComp0

	// Spotlight sound.
	Begin Object Class=AudioComponent Name=MyShowerSound
		SoundCue=SoundCue'Foley_Flesh.AcidSprayer.AcidSprayer_SprayLoopCue'
		bStopWhenOwnerDestroyed=TRUE
		bAutoPlay=TRUE
	End Object
	AC_ShowerSound=MyShowerSound

	RemoteRole=ROLE_SimulatedProxy
	bAlwaysRelevant=true

	//debug
	//bDebug=TRUE
}
