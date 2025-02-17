/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPawn_LocustLeviathan extends GearPawn_LocustLeviathanBase;

/** Master material for leviathan body */
var MaterialInstanceTimeVarying MasterMaterial;
var MaterialInstanceConstant	TentacleMaterial;

/** Parameter curve points for WaterOpacity and ShowTopWater material effect */
var InterpCurveFloat			EmergeWaterOpacityPoints, EmergeShowTopWaterPoints;
var InterpCurveFloat			ZeroToOne, OneToZero;
/** Duration of WaterOpacity and ShowTopWater material effect */
var float						EmergeWaterOpacityDuration, EmergeShowTopWaterDuration;
var float						TentacleMaterialDuration;


var(Effects) array<SkeletalMeshActor_LeviathanWaterStream>	WaterStreams;

simulated function PostBeginPlay()
{
	local InterpCurvePointFloat Point;

	Super.PostBeginPlay();

	// Setup emerge/submerge curve points
	Point.InVal	 = 0.f;
	Point.OutVal = 1.f;
	EmergeWaterOpacityPoints.Points[EmergeWaterOpacityPoints.Points.length] = Point;
	Point.InVal  = 1.f;
	Point.OutVal = 0.f;
	EmergeWaterOpacityPoints.Points[EmergeWaterOpacityPoints.Points.length] = Point;

	Point.InVal  = 0.f;
	Point.OutVal = 1.f;
	EmergeShowTopWaterPoints.Points[EmergeShowTopWaterPoints.Points.length] = Point;
	Point.InVal  = 1.f;
	Point.OutVal = 0.5f;
	EmergeShowTopWaterPoints.Points[EmergeShowTopWaterPoints.Points.length] = Point;

	// Create emerge/submerge material instances
	MasterMaterial = new(Outer) class'MaterialInstanceTimeVarying';
	MasterMaterial.SetParent( MaterialInterface'Locust_Leviathan.Materials.MI_Leviathan_MASTER_INSTANCE' );
	Mesh.SetMaterial( 0, MasterMaterial );


	Point.InVal	 = 0.f;
	Point.OutVal = 0.f;
	ZeroToOne.Points[ZeroToOne.Points.length] = Point;
	Point.InVal  = 1.f;
	Point.OutVal = 1.f;
	ZeroToOne.Points[ZeroToOne.Points.length] = Point;

	Point.InVal	 = 0.f;
	Point.OutVal = 1.f;
	OneToZero.Points[OneToZero.Points.length] = Point;
	Point.InVal  = 1.f;
	Point.OutVal = 0.f;
	OneToZero.Points[OneToZero.Points.length] = Point;

	TentacleMaterial = new(Outer) class'MaterialInstanceConstant';
	TentacleMaterial.SetParent( MaterialInterface'Locust_Leviathan.Materials.MI_Leviathan_Tentacles' );
	Mesh.SetMaterial( 2, TentacleMaterial );

	SpawnWaterStream( 'FaceTentLeft01'  );
	SpawnWaterStream( 'FaceTentLeft02'  );
	SpawnWaterStream( 'FaceTentLeft03'  );
	SpawnWaterStream( 'FaceTentLeft04'  );
	SpawnWaterStream( 'FaceTentRight01' );
	SpawnWaterStream( 'FaceTentRight02' );
	SpawnWaterStream( 'FaceTentRight03' );
	SpawnWaterStream( 'FaceTentRight04' );
}

simulated function Destroyed()
{
	local int Idx;

	Super.Destroyed();

	for( Idx = 0; Idx < WaterStreams.Length; Idx++ )
	{
		WaterStreams[Idx].Destroy();
	}
	WaterStreams.Length = 0;
}

simulated function SpawnWaterStream( Name SocketName )
{
	local SkeletalMeshActor_LeviathanWaterStream Stream;

	Stream = Spawn( class'SkeletalMeshActor_LeviathanWaterStream', self );
	Stream.SetBase( self,, Mesh, SocketName );
	Stream.SetDrawScale3D( vect(7.0,1.5,1.5) );
	WaterStreams[WaterStreams.Length] = Stream;
}

simulated function PlayEmergeFromWaterEffects()
{
	MasterMaterial.SetScalarCurveParameterValue( 'WaterOpacity', EmergeWaterOpacityPoints );
	MasterMaterial.SetDuration( EmergeWaterOpacityDuration );

	MasterMaterial.SetScalarCurveParameterValue( 'ShowTopWater', EmergeShowTopWaterPoints );
	MasterMaterial.SetDuration( EmergeShowTopWaterDuration );

	SetTimer( 1.f, FALSE, nameof(ResetEmergeFlag) );
}

// Handles playing all the mouth tentacle animations by state
simulated function PlayTentacleAnim( int Idx )
{
	local float InterpTime;

	if( Tentacles[Idx].Status == TENTACLE_None )
	{
		TentacleMask.SetMaskWeight( Idx, 0.f, 0.4f );
	}
	else
	if( Tentacles[Idx].Status == TENTACLE_Start )
	{
		Tentacles[Idx].BlendList.SetActiveChild( 0, 0.2f );
		TentacleMask.SetMaskWeight( Idx, 1.f, 0.2f );
	}
	else
	if( Tentacles[Idx].Status == TENTACLE_Tell )
	{
		Tentacles[Idx].BlendList.SetActiveChild( 1, 0.2f );
		Tentacles[Idx].AC_TentacleTell.Play();
	}
	else
	if( Tentacles[Idx].Status == TENTACLE_Attack )
	{
		Tentacles[Idx].BlendList.SetActiveChild( 2, 0.2f );
		Tentacles[Idx].AC_TentacleTell.Stop();

		InterpTime = (TentacleInterpTime + TentacleAttackTime) * 0.9f;

		Tentacles[Idx].SkelCont_Move.BoneTranslation = RepTentacles[Idx].TargetLocation;
		Tentacles[Idx].SkelCont_Move.SetSkelControlStrength( 1.f, InterpTime );
		Tentacles[Idx].SkelCont_Spline.SetSkelControlStrength( 1.f, InterpTime );

		// Delay requested by Lee
		SetTimer( 0.75, FALSE, 'PlayAttackSound' );
	}
	else
	if( Tentacles[Idx].Status == TENTACLE_Interp )
	{
		// do nothing
	}
	else
	if( Tentacles[Idx].Status == TENTACLE_Pause )
	{
		// do nothing
	}
	else
	if( Tentacles[Idx].Status == TENTACLE_Wounded )
	{
		Tentacles[Idx].BlendList.SetActiveChild( 5, 0.1f );
	}
	else
	if( Tentacles[Idx].Status == TENTACLE_Damaged )
	{
		Tentacles[Idx].SkelCont_Move.SetSkelControlStrength( 0.f, 0.2f );
		Tentacles[Idx].SkelCont_Spline.SetSkelControlStrength( 0.f, 0.2f );
		Tentacles[Idx].AC_TentacleTell.Stop();

		Tentacles[Idx].BlendList.SetActiveChild( 3, 0.2f );
	}
	else
	if( Tentacles[Idx].Status == TENTACLE_Retract )
	{
		Tentacles[Idx].SkelCont_Move.SetSkelControlStrength( 0.f, 0.2f );
		Tentacles[Idx].SkelCont_Spline.SetSkelControlStrength( 0.f, 0.2f );
		Tentacles[Idx].AC_TentacleTell.Stop();

		Tentacles[Idx].BlendList.SetActiveChild( 4, 0.2f );
	}
}

simulated function UpdateTentacleEffects( float DeltaTime )
{
	local int	Idx;
	local float Pct;

	for( Idx = 0; Idx < ArrayCount(Tentacles); Idx++ )
	{
		if( Tentacles[Idx].Status == TENTACLE_Start		||
			Tentacles[Idx].Status == TENTACLE_Tell		||
			Tentacles[Idx].Status == TENTACLE_Attack	||
			Tentacles[Idx].Status == TENTACLE_Interp	||
			Tentacles[Idx].Status == TENTACLE_Pause		)
		{
			Tentacles[Idx].MatTimer = FMin( Tentacles[Idx].MatTimer + DeltaTime, TentacleMaterialDuration );
		}
		else
		{
			Tentacles[Idx].MatTimer = FMax( Tentacles[Idx].MatTimer - DeltaTime, 0.f );
		}
			
		Pct = Tentacles[Idx].MatTimer / TentacleMaterialDuration;

		TentacleMaterial.SetScalarParameterValue( Tentacles[Idx].MatParamName, Pct );
	}
}


defaultproperties
{
	ControllerClass=class'GearAI_Leviathan'

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'Locust_Leviathan.Mesh.Leviathan'
		PhysicsAsset=PhysicsAsset'Locust_Leviathan.Mesh.Leviathan_Physics'
		AnimTreeTemplate=AnimTree'Locust_Leviathan.animation.AT_Leviathan'
		AnimSets(0)=AnimSet'Locust_Leviathan.animation.Leviathan_Mouth'
		AnimSets(1)=AnimSet'Locust_Leviathan.Animation.Leviathan_Body'
		MorphSets(0)=MorphTargetSet'Locust_Leviathan.Mesh.Leviathan_Morphs'
		bHasPhysicsAssetInstance=TRUE
		bEnableFullAnimWeightBodies=TRUE
		BlockActors=FALSE
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=FALSE
		CollideActors=TRUE
		BlockRigidBody=TRUE
		bAlwaysVisible=TRUE
		bIgnoreHiddenActorsMembership=TRUE
	End Object

	Begin Object Name=CollisionCylinder
		CollisionRadius=+0034.000000
		CollisionHeight=+0078.000000
		BlockNonZeroExtent=FALSE
		BlockZeroExtent=FALSE
		BlockActors=FALSE
		CollideActors=FALSE
	End Object

	TentacleStartAttackSound=SoundCue'Locust_Leviathan_Efforts.Leviathan.Leviathan_ToothRetractsACue'
	TentacleImpactSound=SoundCue'Locust_Leviathan_Efforts.Leviathan.Leviathan_ToothImpactCue'
	TentacleHurtSound=SoundCue'Locust_Leviathan_Efforts.Leviathan.Leviathan_ToothScreamCue'  
	TentacleTellSound=SoundCue'Locust_Leviathan_Efforts.Leviathan.Leviathan_ToothExtendsCue'

	MouthDamagedSound=SoundCue'Locust_Leviathan_Efforts.Leviathan.Leviathan_VocalToothPainCue'
	MouthExplosionSound=SoundCue'Weapon_Grenade.Impacts.GrenadeBoloExplosionCue'
	MouthOpenSound=SoundCue'Locust_Leviathan_Efforts.Leviathan.Leviathan_ThroatExtendACue'
	MouthOpenIdleSound=SoundCue'Locust_Leviathan_Efforts.Leviathan.Leviathan_ThroatOpensCue'
	MouthShutSound=SoundCue'Locust_Leviathan_Efforts.Leviathan.Leviathan_ThroatExtendBCue'  
	
	PS_MouthOpen=ParticleSystem'Locust_Leviathan.Effects.P_Leviathan_Mouth_Saliva_Roar'
	PS_MouthGrenDamage=ParticleSystem'Locust_Leviathan.Effects.P_Leviathan_Blood_Grenade_Explosion'
	PS_FaceTentacleDrip=ParticleSystem'Locust_Leviathan.Effects.P_Leviathan_Water_Dripping_Face_Tentacle'
	PS_MouthDrip=ParticleSystem'Locust_Leviathan.Effects.P_Leviathan_Water_Dripping_Mouth_Ceiling'
	PS_MouthMucus=ParticleSystem'Locust_Leviathan.Effects.P_Leviathan_Mucus_Mouth_Throat'

	EmergeWaterOpacityDuration=30.f
	EmergeShowTopWaterDuration=60.f
	TentacleMaterialDuration=2.f
}