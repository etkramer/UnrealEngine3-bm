/**
 * GearPawnFX:  this is meant to hold all of the "effects" type code
 *
 * Currently this is an object but depending on much further we want to go, it should be turned into an Actor.
 *
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPawnFX extends Object within GearPawn
	config(Pawn);

/** This will show all of the debug info for spawning these blood decals **/
var() config bool bShowDebugInfo;


/** This will leave a trail of blood when you are DBNO **/
simulated final function SpawnABloodTrail_DBNO()
{
	if( WorldInfo.GRI.ShouldShowGore() )
	{
		if( VSize(Velocity) > 10.0f )
		{
			LeaveADecal( BloodDecalTrace_GroundBelowThePawn, BloodDecalChoice_DBNO, BloodDecalTimeVaryingParams_DBNO );
		}
	}
}

/** This will leave a trail of blood when you are MeatBagging **/
simulated final function SpawnABloodTrail_MeatBag()
{
	if( WorldInfo.GRI.ShouldShowGore() )
	{
		if( VSize(Velocity) > 10.0f )
		{
			if( FRand() > 0.40f )
			{
				LeaveADecal( BloodDecalTrace_GroundBelowThePawn, BloodDecalChoice_MeatBag, BloodDecalTimeVaryingParams_Default );
			}
			else
			{
				LeaveADecal( BloodDecalTrace_GroundBelowThePawn, BloodDecalChoice_MeatBagHeelScuff, BloodDecalTimeVaryingParams_Default );
			}
		}
	}
}

/** This will leave a trail of blood when you are in cover **/
simulated final function SpawnABloodTrail_Wall()
{
	if( WorldInfo.GRI.ShouldShowGore() )
	{
		if( VSize(Velocity) > 10.0f )
		{
			LeaveADecal( BloodDecalTrace_CoverBehindPawnMiddleOfBody, BloodDecalChoice_Wall, BloodDecalTimeVaryingParams_Wall );
		}
	}
}

simulated final function SpawnABloodTrail_GibExplode_Ground()
{
	if( WorldInfo.GRI.ShouldShowGore() )
	{
		LeaveADecal( BloodDecalTrace_GroundBelowThePawn_Rand, BloodDecalChoice_GibExplode_Ground, BloodDecalTimeVaryingParams_Default );
	}
}

/** This will unleash a blast of gore on the walls, ceiling, and floor around the character**/
simulated final function SpawnABloodTrail_GibExplode_360()
{
	if( WorldInfo.GRI.ShouldShowGore() )
	{
		LeaveADecal( BloodDecalTrace_360AroundPawn_Forward, BloodDecalChoice_ChainsawSpray_Ground, BloodDecalTimeVaryingParams_Default );
		LeaveADecal( BloodDecalTrace_360AroundPawn_Left, BloodDecalChoice_ChainsawSpray_Ground, BloodDecalTimeVaryingParams_Default );
		LeaveADecal( BloodDecalTrace_360AroundPawn_Backward, BloodDecalChoice_ChainsawSpray_Ground, BloodDecalTimeVaryingParams_Default );
		LeaveADecal( BloodDecalTrace_360AroundPawn_Right, BloodDecalChoice_ChainsawSpray_Ground, BloodDecalTimeVaryingParams_Default );
		//LeaveADecal( BloodDecalTrace_360AroundPawn_Up, BloodDecalChoice_ChainsawSpray_Ground, BloodDecalTimeVaryingParams_Default );
		LeaveADecal( BloodDecalTrace_360AroundPawn_Down, BloodDecalChoice_GibExplode_Ground, BloodDecalTimeVaryingParams_Default );

		LeaveADecal( BloodDecalTrace_360AroundPawn_Down, BloodDecalChoice_GibExplode_Ground_SmallSplat, BloodDecalTimeVaryingParams_Default );
		LeaveADecal( BloodDecalTrace_360AroundPawn_Down, BloodDecalChoice_GibExplode_Ground_SmallSplat, BloodDecalTimeVaryingParams_Default );
		// 50% chance of leaving a third small splat
		if( FRand() > 0.50f )
		{
			LeaveADecal( BloodDecalTrace_360AroundPawn_Down, BloodDecalChoice_GibExplode_Ground_SmallSplat, BloodDecalTimeVaryingParams_Default );
		}
	}
}

/** This will leave a trail of blood on the ground when you chainsaw someone **/
simulated final function SpawnABloodTrail_ChainsawSpray_Ground()
{
	if (WorldInfo.GRI.ShouldShowGore() && MyGearWeapon != None)
	{
		LeaveADecal( BloodDecalTrace_GroundBelowTheWeaponEnd_Rand, BloodDecalChoice_ChainsawSpray_Ground, BloodDecalTimeVaryingParams_Default );
	}
}

/** This will leave a trail of blood on the wall when you chainsaw someone **/
simulated final function SpawnABloodTrail_ChainsawSpray_Wall()
{
	if (WorldInfo.GRI.ShouldShowGore() && MyGearWeapon != None)
	{
		LeaveADecal( BloodDecalTrace_TraceFromEndOfChainsaw, BloodDecalChoice_ChainsawSpray_Wall, BloodDecalTimeVaryingParams_Default );
	}
}

/** This will leave a trail of blood on the wall when you chainsaw someone **/
simulated final function SpawnABloodTrail_GibImpact( vector HitLoc )
{
	if( WorldInfo.GRI.ShouldShowGore() )
	{
		LeaveADecal( BloodDecalTrace_TraceGibImpact, BloodDecalChoice_GibImpact, BloodDecalTimeVaryingParams_Default, HitLoc );
	}
}



/** This will leave a trail of blood from your headshot stump **/
simulated final function SpawnABloodTrail_HeadShot()
{
	if( WorldInfo.GRI.ShouldShowGore() )
	{
		LeaveADecal( BloodDecalTrace_FromNeck, BloodDecalChoice_HeadShot, BloodDecalTimeVaryingParams_Default );
	}
}


simulated final function SpawnABloodTrail_BloodPool()
{
	if( WorldInfo.GRI.ShouldShowGore() )
	{
		LeaveADecal( BloodDecalTrace_GroundBelowThePawn, BloodDecalChoice_BloodPool, BloodDecalTimeVaryingParams_Default );
	}
}


simulated final function SpawnABloodTrail_PawnIsReallyHurt()
{
	if( WorldInfo.GRI.ShouldShowGore() )
	{
		LeaveADecal( BloodDecalTrace_GroundBelowThePawn, BloodDecalChoice_PawnIsReallyHurt, BloodDecalTimeVaryingParams_Default );
	}
}


simulated final function SpawnABloodTrail_HitByABullet()
{
	if( WorldInfo.GRI.ShouldShowGore() )
	{
		LeaveADecal( BloodDecalTrace_GroundBelowThePawn, BloodDecalChoice_HitByBullet, BloodDecalTimeVaryingParams_Default );
	}
}


simulated final function SpawnABloodTrail_LimbBreak( vector InStartLocation )
{
	if( WorldInfo.GRI.ShouldShowGore() )
	{
		//DrawDebugCoordinateSystem( InStartLocation, rot(0,0,0), 3.0f, TRUE );
		LeaveADecal( BloodDecalTrace_GroundBelowThePawn, BloodDecalChoice_LimbBreak, BloodDecalTimeVaryingParams_Default, InStartLocation );
	}
}






/** This is the Delegate for the Tracing Policy for LeaveADecal **/
simulated delegate DecalTrace( out vector out_TraceStart, out vector out_TraceDest, const float RandomOffsetRadius, optional vector ForceStartLocation )
{
	`log( "DecalTrace Delegate was not set" );
	ScriptTrace();
}

/** This is the Delegate for the Decal Choice Policy for LeaveADecal **/
simulated delegate DecalChoice( const out TraceHitInfo HitInfo, out float out_DecalRotation, out DecalData out_DecalData )
{
	`log( "DecalChoice Delegate was not set" );
	ScriptTrace();
}

/** This is the Delegate for the MITV Params Policy for LeaveADecal **/
simulated delegate DecalTimeVaryingParams( out MaterialInstance MI_Decal )
{
	`log( "DecalTimeVaryingParams Delegate was not set" );
	ScriptTrace();
}


// @ todo pass in a hit normal so we can get hits on walls also
simulated final function BloodDecalTrace_TraceGibImpact( out vector out_TraceStart, out vector out_TraceDest, const float RandomOffsetRadius, optional vector ForceStartLocation )
{
	out_TraceStart = ForceStartLocation + ( Vect(0,0,15));
	out_TraceDest =  out_TraceStart - ( Vect(0,0,100));
}

simulated final function BloodDecalTrace_FromNeck( out vector out_TraceStart, out vector out_TraceDest, const float RandomOffsetRadius, optional vector ForceStartLocation )
{
	local vector				Loc;
	local rotator				Rot;

	if( Mesh.GetSocketWorldLocationAndRotation( 'HeadShotBloodPS', Loc, Rot ) )
	{
		//`log( "BloodDecalTrace_FromNeck" );
		out_TraceStart = Loc;

		// move this down some number of units so we intersect the floor and leave nice blood spurts
		out_TraceDest = (out_TraceStart + (vector(Rot) * 128.0f)) + vect(0,0,-50);

		//DrawDebugLine( out_TraceStart, out_TraceDest, 255, 1, 1, TRUE );
		//DrawDebugCoordinateSystem( out_TraceStart, Rot, 7.0f, TRUE );
		//`log( out_TraceStart );
	}
}


simulated final function BloodDecalTrace_FromNeckForFacePunch( out vector out_TraceStart, out vector out_TraceDest, const float RandomOffsetRadius, optional vector ForceStartLocation )
{
	local vector				Loc;
	local rotator				Rot;

	local vector RandomOffsetVect;

	RandomOffsetVect = GetRandomOffsetVector_XY( -1.0f*RandomOffsetRadius, RandomOffsetRadius );

	if( Mesh.GetSocketWorldLocationAndRotation( 'HeadShotBloodPS', Loc, Rot ) )
	{
		//`log( "BloodDecalTrace_FromNeck" );
		out_TraceStart = Loc;

		// move this down some number of units so we intersect the floor and leave nice blood spurts
		out_TraceDest = (out_TraceStart + (vector(Rot) * 128.0f)) + vect(0,0,-100) + RandomOffsetVect;

		//DrawDebugLine( out_TraceStart, out_TraceDest, 255, 1, 1, TRUE );
		//DrawDebugCoordinateSystem( out_TraceStart, Rot, 7.0f, TRUE );
		//`log( out_TraceStart );
	}
}



/**
 * This will look at the previous blood trail decal and then figure out what the rotation of the next decal needs to be to look
 * like the current one "trailed" from the previous one.
 **/
simulated final function float DetermineRotationOfBloodTrailDecal()
{
	local float Retval;

	if( Rotator(Location - LocationOfLastBloodTrail).Yaw >= 0 )
	{
		Retval = abs((Rotator(Location - LocationOfLastBloodTrail).Yaw)/65535.0f) * -360.0f;
	}
	else
	{
		Retval = abs((Rotator(Location - LocationOfLastBloodTrail).Yaw)/65535.0f) * 360.0f;
	}

	//`log( "Rotation: " $ Rotation.Yaw @ Retval );

	return Retval;
}


/**
 * This will trace from the pawn to the ground below them.
 **/
simulated final function BloodDecalTrace_GroundBelowThePawn( out vector out_TraceStart, out vector out_TraceDest, const float RandomOffsetRadius, optional vector ForceStartLocation )
{
	local vector RandomOffsetVect;

	RandomOffsetVect = GetRandomOffsetVector_XY( -1.0f*RandomOffsetRadius, RandomOffsetRadius );

	//`log( "BloodDecalTrace_GroundBelowThePawn" );
	out_TraceStart = Location + ( Vect(0,0,15)) + RandomOffsetVect;;
	out_TraceDest = out_TraceStart - ( Vect(0,0,256));
}


/** return as a random XY vector based on the inputs **/
simulated final function vector GetRandomOffsetVector_XY( float MinVal, float MaxVal )
{
	local vector2d RandomOffset;
	local vector RandomOffsetVect;

	RandomOffset.X = MinVal;
	RandomOffset.Y = MaxVal;

	RandomOffsetVect.X = GetRangeValueByPct(RandomOffset,FRand());
	RandomOffsetVect.Y = GetRangeValueByPct(RandomOffset,FRand());
	RandomOffsetVect.Z = 0.0f;

	return RandomOffsetVect;
}



/**
 * This will trace from the pawn to the ground below them. Randomly spread out around them.
 **/
simulated final function BloodDecalTrace_GroundBelowThePawn_Rand( out vector out_TraceStart, out vector out_TraceDest, const float RandomOffsetRadius, optional vector ForceStartLocation )
{
	local vector RandomOffsetVect;

	RandomOffsetVect = GetRandomOffsetVector_XY( -128.0f, 128.0f );

	//`log( "BloodDecalTrace_GroundBelowThePawn_Rand" );
	out_TraceStart = Location + Vect(0,0,15) + RandomOffsetVect;
	out_TraceDest = out_TraceStart - Vect(0,0,256);

	//`log( out_TraceStart );
}


/** This is "forward" trace from the pawn **/
simulated final function BloodDecalTrace_360AroundPawn_Forward( out vector out_TraceStart, out vector out_TraceDest, const float RandomOffsetRadius, optional vector ForceStartLocation )
{
	local vector RandomOffsetVect;

	RandomOffsetVect = GetRandomOffsetVector_XY( -1.0f*RandomOffsetRadius, RandomOffsetRadius );

	//`log( "BloodDecalTrace_GroundBelowThePawn_Rand" );
	out_TraceStart = Location + Vect(0,0,15) + RandomOffsetVect;
	out_TraceDest = out_TraceStart + Vect(0,256,0);

	//`log( out_TraceStart );
}

/** This is "left" trace from the pawn **/
simulated final function BloodDecalTrace_360AroundPawn_Left( out vector out_TraceStart, out vector out_TraceDest, const float RandomOffsetRadius, optional vector ForceStartLocation )
{
	local vector RandomOffsetVect;

	RandomOffsetVect = GetRandomOffsetVector_XY( -1.0f*RandomOffsetRadius, RandomOffsetRadius );

	//`log( "BloodDecalTrace_GroundBelowThePawn_Rand" );
	out_TraceStart = Location + Vect(0,0,15) + RandomOffsetVect;
	out_TraceDest = out_TraceStart + Vect(-256,0,0);

	//`log( out_TraceStart );
}

/** This is "right" trace from the pawn **/
simulated final function BloodDecalTrace_360AroundPawn_Right( out vector out_TraceStart, out vector out_TraceDest, const float RandomOffsetRadius, optional vector ForceStartLocation )
{
	local vector RandomOffsetVect;

	RandomOffsetVect = GetRandomOffsetVector_XY( -1.0f*RandomOffsetRadius, RandomOffsetRadius );

	//`log( "BloodDecalTrace_GroundBelowThePawn_Rand" );
	out_TraceStart = Location + Vect(0,0,15) + RandomOffsetVect;
	out_TraceDest = out_TraceStart + Vect(256,0,0);

	//`log( out_TraceStart );
}

/** This is "backward" trace from the pawn **/
simulated final function BloodDecalTrace_360AroundPawn_Backward( out vector out_TraceStart, out vector out_TraceDest, const float RandomOffsetRadius, optional vector ForceStartLocation )
{
	local vector RandomOffsetVect;

	RandomOffsetVect = GetRandomOffsetVector_XY( -1.0f*RandomOffsetRadius, RandomOffsetRadius );

	//`log( "BloodDecalTrace_GroundBelowThePawn_Rand" );
	out_TraceStart = Location + Vect(0,0,15) + RandomOffsetVect;
	out_TraceDest = out_TraceStart + Vect(0,-256,0);

	//`log( out_TraceStart );
}

/** This is "up" trace from the pawn **/
simulated final function BloodDecalTrace_360AroundPawn_Up( out vector out_TraceStart, out vector out_TraceDest, const float RandomOffsetRadius, optional vector ForceStartLocation )
{
	local vector RandomOffsetVect;

	RandomOffsetVect = GetRandomOffsetVector_XY( -1.0f*RandomOffsetRadius, RandomOffsetRadius );

	//`log( "BloodDecalTrace_GroundBelowThePawn_Rand" );
	out_TraceStart = Location + Vect(0,0,15) + RandomOffsetVect;
	out_TraceDest = out_TraceStart + Vect(0,0,256);

	//`log( out_TraceStart );
}

/** This is "down" trace from the pawn **/
simulated final function BloodDecalTrace_360AroundPawn_Down( out vector out_TraceStart, out vector out_TraceDest, const float RandomOffsetRadius, optional vector ForceStartLocation )
{
	local vector RandomOffsetVect;

	RandomOffsetVect = GetRandomOffsetVector_XY( -1.0f*RandomOffsetRadius, RandomOffsetRadius );

	//`log( "BloodDecalTrace_GroundBelowThePawn_Rand" );
	out_TraceStart = Location + Vect(0,0,15) + RandomOffsetVect;
	out_TraceDest = out_TraceStart + Vect(0,0,-256);

	//`log( out_TraceStart );
}


/**
 * This will trace from the pawn to the ground below them. Randomly spread out around them.
 **/
simulated final function BloodDecalTrace_GroundBelowTheWeaponEnd_Rand( out vector out_TraceStart, out vector out_TraceDest, const float RandomOffsetRadius, optional vector ForceStartLocation )
{
	local SkeletalMeshComponent	WeapMesh;
	local vector				Loc;
	local rotator				Rot;

	local vector2d RandomOffset;
	local vector RandomOffsetVect;


	WeapMesh = SkeletalMeshComponent(MyGearWeapon.Mesh);

	if( WeapMesh.GetSocketWorldLocationAndRotation( MyGearWeapon.MuzzleSocketName, Loc, Rot ) )
	{
		//`log( "BloodDecalTrace_GroundBelowTheWeaponEnd_Rand" );
		RandomOffset.X = -128;
		RandomOffset.Y = 128;

		RandomOffsetVect.X = GetRangeValueByPct(RandomOffset,FRand());
		RandomOffsetVect.Y = GetRangeValueByPct(RandomOffset,FRand());
		RandomOffsetVect.Z = 0.0f;

		//`log( "BloodDecalTrace_GroundBelowTheWeaponEnd_Rand" );
		out_TraceStart = Loc + Vect(0,0,15) + RandomOffsetVect;
		out_TraceDest =  out_TraceStart + Vect(0,0,-256);

		//`log( out_TraceStart );
	}
}


/**
 * This will determine where to trace for the wall smear bood trail.
 **/
simulated final function BloodDecalTrace_CoverBehindPawnMiddleOfBody( out vector out_TraceStart, out vector out_TraceDest, const float RandomOffsetRadius, optional vector ForceStartLocation )
{
	local vector SocketLocation;

	//`log( "BloodDecalTrace_CoverBehindPawnMiddleOfBody" );

	SocketLocation = Mesh.GetBoneLocation( PelvisBoneName );
	out_TraceStart = SocketLocation;
	out_TraceStart.Z = SocketLocation.Z;

	out_TraceDest = SocketLocation - ((GetCollisionRadius() + 50) * AcquiredCoverInfo.Normal);
	out_TraceDest.Z = SocketLocation.Z;

	//DrawDebugCoordinateSystem( out_TraceStart, rot(0,0,0), 2.0f, TRUE );
	//DrawDebugLine( out_TraceStart, out_TraceDest, 255, 0, 0, TRUE );
}


/**
 * This will trace from the pawn to the ground below them.
 **/
simulated final function BloodDecalTrace_TraceFromEndOfChainsaw( out vector out_TraceStart, out vector out_TraceDest, const float RandomOffsetRadius, optional vector ForceStartLocation )
{
	local SkeletalMeshComponent	WeapMesh;
	local vector				Loc;
	local rotator				Rot;

	WeapMesh = SkeletalMeshComponent(MyGearWeapon.Mesh);
	if( WeapMesh.GetSocketWorldLocationAndRotation( MyGearWeapon.MuzzleSocketName, Loc, Rot ) )
	{
		//`log( "BloodDecalTrace_TraceFromEndOfChainsaw" );
		out_TraceStart = Loc;
		out_TraceDest =  out_TraceStart + (Vector(Rot) * 512.0f);
	}
}


/**
 * This will choose the Decals to use for the generic blood splatter
 **/
simulated final function BloodDecalChoice_HeadShot( const out TraceHitInfo HitInfo, out float out_DecalRotation, out DecalData out_DecalData )
{
	out_DecalData = class'GearPhysicalMaterialProperty'.static.GetBloodDecalData( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( Hitinfo ), GDTT_NeckSpurt, WorldInfo );

	if (out_DecalData.bRandomizeRotation)
	{
		out_DecalRotation = FRand() * 360.0f;
	}

	//out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Materials.DM_BloodSmear_Var01_Spot_1' );
}


/**
 * This will choose the Decals to use for the generic blood splatter
 **/
simulated final function BloodDecalChoice_Splat( const out TraceHitInfo HitInfo, out float out_DecalRotation, out DecalData out_DecalData )
{
	//out_MI_Decal.SetParent( MaterialInstance'Gear_Blood_DecalStaticSwitch.MITV_UsefadingYes' );
	//out_MI_Decal.SetParent( MaterialInstance'Gear_Blood_DecalStaticSwitch.Decals.MITV_UsefadingNo' );
	//out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Decals.MITV_BloodSplatter02' );

	out_DecalData = class'GearPhysicalMaterialProperty'.static.GetBloodDecalData( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( Hitinfo ), GDTT_Blood_GenericSplat, WorldInfo );

	if (out_DecalData.bRandomizeRotation)
	{
		out_DecalRotation = FRand() * 360.0f;
	}

	//out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Decals.BloodSplatter02' );
}

/**
* This will choose the Decals to use for the generic blood splatter
**/
simulated final function BloodDecalChoice_GibImpact( const out TraceHitInfo HitInfo, out float out_DecalRotation, out DecalData out_DecalData )
{
	//out_MI_Decal.SetParent( MaterialInstance'Gear_Blood_DecalStaticSwitch.MITV_UsefadingYes' );
	//out_MI_Decal.SetParent( MaterialInstance'Gear_Blood_DecalStaticSwitch.Decals.MITV_UsefadingNo' );
	//out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Decals.MITV_BloodSplatter02' );

	out_DecalData = class'GearPhysicalMaterialProperty'.static.GetBloodDecalData( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( Hitinfo ), GDTT_GibImpact, WorldInfo );

	if (out_DecalData.bRandomizeRotation)
	{
		out_DecalRotation = FRand() * 360.0f;
	}

	//out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Materials.DM_BloodSmear_Var01_Spot_1' );
}



/**
 * This will choose the Decals to use for the generic blood splatter
 **/
simulated final function BloodDecalChoice_ChainsawSpray_Wall( const out TraceHitInfo HitInfo, out float out_DecalRotation, out DecalData out_DecalData )
{
	out_DecalData = class'GearPhysicalMaterialProperty'.static.GetBloodDecalData( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( Hitinfo ), GDTT_Chainsaw_Wall, WorldInfo );
	//out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Materials.DM_BloodSmear_Var01_Spray_2' );

	if (out_DecalData.bRandomizeRotation)
	{
		out_DecalRotation = FRand() * 360.0f;
	}
}

simulated final function BloodDecalChoice_ChainsawSpray_Ground( const out TraceHitInfo HitInfo, out float out_DecalRotation, out DecalData out_DecalData )
{
	out_DecalData = class'GearPhysicalMaterialProperty'.static.GetBloodDecalData( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( Hitinfo ), GDTT_Chainsaw_Ground, WorldInfo );
	//out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Materials.DM_BloodSmear_Var01_Spot_6' );

	if (out_DecalData.bRandomizeRotation)
	{
		out_DecalRotation = FRand() * 360.0f;
	}
}

simulated final function BloodDecalChoice_GibExplode_Ground( const out TraceHitInfo HitInfo, out float out_DecalRotation, out DecalData out_DecalData )
{
	out_DecalData = class'GearPhysicalMaterialProperty'.static.GetBloodDecalData( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( Hitinfo ), GDTT_GibExplode_Ground, WorldInfo );
	//out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Materials.DM_BloodSmear_Var01_Spot_6' );

	if (out_DecalData.bRandomizeRotation)
	{
		out_DecalRotation = FRand() * 360.0f;
	}
}

/**
 * This will choose the Decals to use for the wall trail.  Data will be moved to PhysicalMaterial System / Content Driven.
 **/
simulated final function BloodDecalChoice_Wall( const out TraceHitInfo HitInfo, out float out_DecalRotation, out DecalData out_DecalData )
{
	if( LocationOfLastBloodTrail == Vect(0,0,0) )
	{
		out_DecalData = class'GearPhysicalMaterialProperty'.static.GetBloodDecalData( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( Hitinfo ), GDTT_Wall_SlammingIntoCover, WorldInfo );
		//out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Materials.DM_BloodSmear_Var01_Spot_1' );
	}
	else
	{
		out_DecalData = class'GearPhysicalMaterialProperty'.static.GetBloodDecalData( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( Hitinfo ), GDTT_Wall_Smear, WorldInfo );
		//out_MI_Decal.SetParent( MaterialInstanceConstant'Gear_Blood.Materials.DM_BloodSmear_Wall_Straight_01' );
	}

	if (out_DecalData.bRandomizeRotation)
	{
		out_DecalRotation = FRand() * 360.0f;
	}
	else
	{
		out_DecalRotation = 0.0f;
	}
}

/**
 * This will choose the Decals to use for DBNO.  Data will be moved to PhysicalMaterial System / Content Driven.
 **/
simulated final function BloodDecalChoice_DBNO( const out TraceHitInfo HitInfo, out float out_DecalRotation, out DecalData out_DecalData )
{
	out_DecalRotation = DetermineRotationOfBloodTrailDecal();

	if( LocationOfLastBloodTrail == Vect(0,0,0) )
	{
		out_DecalData = class'GearPhysicalMaterialProperty'.static.GetBloodDecalData( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( Hitinfo ), GDTT_DBNO_BodyHittingFloor, WorldInfo );
		//out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Materials.DM_BloodSmear_Var01_Spot_1' );
	}
	else
	{
		out_DecalData = class'GearPhysicalMaterialProperty'.static.GetBloodDecalData( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( Hitinfo ), GDTT_DBNO_Smear, WorldInfo );
		//out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Materials.DM_BloodSmear_Var01_Straight_8' );
	}
}

/**
 * This will choose the Decals to use for the splatter blood trail.  Data will be moved to PhysicalMaterial System / Content Driven.
 **/
simulated final function BloodDecalChoice_MeatBag( const out TraceHitInfo HitInfo, out float out_DecalRotation, out DecalData out_DecalData )
{
	if( LocationOfLastBloodTrail == Vect(0,0,0) )
	{
		out_DecalData = class'GearPhysicalMaterialProperty'.static.GetBloodDecalData( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( Hitinfo ), GDTT_MeatBag_FirstGrabbing, WorldInfo );
		//out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Materials.DM_BloodSmear_Var01_Spot_1' );
	}
	else
	{
		out_DecalData = class'GearPhysicalMaterialProperty'.static.GetBloodDecalData( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( Hitinfo ), GDTT_MeatBag_BloodSplatter, WorldInfo );
		//out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Decals.MITV_BloodSplatter02' );
		//out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Decals.BloodSplatter02' );
	}

	if (out_DecalData.bRandomizeRotation)
	{
		out_DecalRotation = FRand() * 360.0f;
	}
}

/**
 * This will choose the Decals to use for the MeatBag heel scuff.   Data will be moved to PhysicalMaterial System / Content Driven.
 **/
simulated final function BloodDecalChoice_MeatBagHeelScuff( const out TraceHitInfo HitInfo, out float out_DecalRotation, out DecalData out_DecalData )
{
	out_DecalRotation = DetermineRotationOfBloodTrailDecal();

	if( LocationOfLastBloodTrail == Vect(0,0,0) )
	{
		out_DecalData = class'GearPhysicalMaterialProperty'.static.GetBloodDecalData( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( Hitinfo ), GDTT_MeatBag_FirstGrabbing, WorldInfo );
		//out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Materials.DM_BloodSmear_Var01_Spot_1' );

		if (out_DecalData.bRandomizeRotation)
		{
			out_DecalRotation = FRand() * 360.0f;
		}
	}
	else
	{
		out_DecalData = class'GearPhysicalMaterialProperty'.static.GetBloodDecalData( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( Hitinfo ), GDTT_MeatBag_HeelScuff, WorldInfo );
		//out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Materials.DM_BloodSmear_Var01_Straight_5' );
	}
}

simulated final function BloodDecalChoice_BloodPool( const out TraceHitInfo HitInfo, out float out_DecalRotation, out DecalData out_DecalData )
{
	out_DecalData = class'GearPhysicalMaterialProperty'.static.GetBloodDecalData( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( Hitinfo ), GDTT_BloodPool, WorldInfo );

	if (out_DecalData.bRandomizeRotation)
	{
		out_DecalRotation = FRand() * 360.0f;
	}
}

simulated final function BloodDecalChoice_GibExplode_Ground_SmallSplat( const out TraceHitInfo HitInfo, out float out_DecalRotation, out DecalData out_DecalData )
{
	out_DecalData = class'GearPhysicalMaterialProperty'.static.GetBloodDecalData( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( Hitinfo ), GDTT_GibExplode_Ground_SmallSplat, WorldInfo );

	if (out_DecalData.bRandomizeRotation)
	{
		out_DecalRotation = FRand() * 360.0f;
	}
}

simulated final function BloodDecalChoice_PunchFace( const out TraceHitInfo HitInfo, out float out_DecalRotation, out DecalData out_DecalData )
{
	out_DecalData = class'GearPhysicalMaterialProperty'.static.GetBloodDecalData( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( Hitinfo ), GDTT_ExecutionLong_PunchFace, WorldInfo );

	if (out_DecalData.bRandomizeRotation)
	{
		out_DecalRotation = FRand() * 360.0f;
	}
}


simulated final function BloodDecalChoice_PawnIsReallyHurt( const out TraceHitInfo HitInfo, out float out_DecalRotation, out DecalData out_DecalData )
{
	out_DecalData = class'GearPhysicalMaterialProperty'.static.GetBloodDecalData( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( Hitinfo ), GDTT_PawnIsReallyHurtSmallSplat, WorldInfo );

	if (out_DecalData.bRandomizeRotation)
	{
		out_DecalRotation = FRand() * 360.0f;
	}
}

simulated final function BloodDecalChoice_HitByBullet( const out TraceHitInfo HitInfo, out float out_DecalRotation, out DecalData out_DecalData )
{
	out_DecalData = class'GearPhysicalMaterialProperty'.static.GetBloodDecalData( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( Hitinfo ), GDTT_HitByBulletSmallSplat, WorldInfo );

	if (out_DecalData.bRandomizeRotation)
	{
		out_DecalRotation = FRand() * 360.0f;
	}
}


simulated final function BloodDecalChoice_LimbBreak( const out TraceHitInfo HitInfo, out float out_DecalRotation, out DecalData out_DecalData )
{
	out_DecalData = class'GearPhysicalMaterialProperty'.static.GetBloodDecalData( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( Hitinfo ), GDTT_LimbBreak, WorldInfo );

	if (out_DecalData.bRandomizeRotation)
	{
		out_DecalRotation = FRand() * 360.0f;
	}
}




/**
 * This will set the params for typical blood decals.
 **/
simulated final function BloodDecalTimeVaryingParams_Default( out MaterialInstance out_MI_Decal )
{
	MaterialInstanceTimeVarying(out_MI_Decal).SetDuration( 45.0f );
}


/**
 * This will set the params for the DBNO decals.  Smearing, Fading Out, and blood drops
 *
 * The MITV points are defined inline for ease of use atm.  Will move to content eventually.
 **/
simulated final function BloodDecalTimeVaryingParams_DBNO( out MaterialInstance out_MI_Decal )
{
	local InterpCurveFloat Points;
	local InterpCurvePointFloat Point;

	Points.Points.length = 2;
	Points.Points[0] = Point;

	Point.InVal = 3.0f;
	Point.OutVal = 1.0f;
	Points.Points[1] = Point;

	MaterialInstanceTimeVarying(out_MI_Decal).SetScalarCurveParameterValue( 'FadeAmount', Points );
	MaterialInstanceTimeVarying(out_MI_Decal).SetScalarStartTime( 'FadeAmount', 30.0f );


	Points.Points.length = 2;
	Point.InVal = 0.0f;
	Point.OutVal = 0.0f;
	Points.Points[0] = Point;

	Point.InVal = 0.5f;
	Point.OutVal = 2.0f;
	Points.Points[1] = Point;

	MaterialInstanceTimeVarying(out_MI_Decal).SetScalarCurveParameterValue( 'SmearPan', Points );
	MaterialInstanceTimeVarying(out_MI_Decal).SetScalarStartTime( 'SmearPan', 0.0f );
}



/**
 * This will set the params for the wall decals.  Smearing, Fading Out, and blood drops
 *
 * The MITV points are defined inline for ease of use atm.  Will move to content eventually.
 **/
simulated final function BloodDecalTimeVaryingParams_Wall( out MaterialInstance out_MI_Decal )
{
	local InterpCurveFloat Points;
	local InterpCurvePointFloat Point;

	Points.Points.length = 2;
	Points.Points[0] = Point;

	Point.InVal = 3.0f;
	Point.OutVal = 1.0f;
	Points.Points[1] = Point;

	MaterialInstanceTimeVarying(out_MI_Decal).SetScalarCurveParameterValue( 'FadeAmount', Points );
	MaterialInstanceTimeVarying(out_MI_Decal).SetScalarStartTime( 'FadeAmount', 30.0f );

	Point.InVal = 0.0f;
	Point.OutVal = 0.0f;
	Points.Points[0] = Point;

	Point.InVal = 4.0f;
	Point.OutVal = -1.0f;
	Points.Points[1] = Point;

	MaterialInstanceTimeVarying(out_MI_Decal).SetScalarCurveParameterValue( 'DripPan', Points );
	MaterialInstanceTimeVarying(out_MI_Decal).SetScalarStartTime( 'DripPan', 0.5f );


	Points.Points.length = 2;
	Point.InVal = 0.0f;
	Point.OutVal = 0.0f;
	Points.Points[0] = Point;

	if (bIsMirrored)
	{
		Point.InVal = 0.2f;
		Point.OutVal = 1.0f;
		Points.Points[1] = Point;

		MaterialInstanceTimeVarying(out_MI_Decal).SetTextureParameterValue( 'SmearTex', Texture2D'Gear_Blood.Materials.BloodMask_02' );
	}
	else
	{
		Point.InVal = 0.2f;
		Point.OutVal = -1.0f;
		Points.Points[1] = Point;

		MaterialInstanceTimeVarying(out_MI_Decal).SetTextureParameterValue( 'SmearTex', Texture2D'Gear_Blood.BloodMask_02_Reverse' );
	}

	MaterialInstanceTimeVarying(out_MI_Decal).SetScalarCurveParameterValue( 'SmearPan', Points );
	MaterialInstanceTimeVarying(out_MI_Decal).SetScalarStartTime( 'SmearPan', 0.0f );
}







/**
 * This function takes a number of delegates that are used to control the core behavior of leaving a decal.
 *
 * DecalTrace:  What to trace against to get the Actor that has been hit.  (e.g. down from some location / out from a gun / etc.)
 * DecalChoice:  Which decal to actually use.  This could involve getting it from some list or determining based on LOD, etc.
 * TimeVaryingParms:  What params to modifiy over time (for fading in/out, having an over time effect, etc.)
 *
 * To use this just pass in the delegates/policies you want to use and voila a decal shall be placed into the world.
 *
 * @todo move this to some global object that exists so we can use the same code from anywhere
 **/
simulated final function LeaveADecal( delegate<DecalTrace> DecalTraceFunc, delegate<DecalChoice> DecalChoiceFunc, delegate<DecalTimeVaryingParams> DecalTimeVaryingParamsFunc, optional vector ForceStartLocation )
{
	local GearDecal GD;
	local DecalData DecalData;
	local float RotationToUseForDecal;

	local Actor TraceActor;
	local vector out_HitLocation;
	local vector out_HitNormal;
	local vector TraceDest;
	local vector TraceStart;
	local vector TraceExtent;
	local TraceHitInfo HitInfo;

	local float RandomScale;

	local bool bIsHorde;
	local bool bIsSplitScreen;
	local float DecalLifeSpan;

	if (!bSpawnBloodTrailDecals)
	{
		return;
	}


	DecalChoice = DecalChoiceFunc;
	DecalChoice( HitInfo, RotationToUseForDecal, DecalData );
	DecalChoice = None;

	if( DecalData.DecalMaterial == None )
	{
		//`warn( "DecalMaterial was none!!" );
		//ScriptTrace();
		return;
	}

	DecalTrace = DecalTraceFunc;
	DecalTrace( TraceStart, TraceDest, DecalData.RandomRadiusOffset, ForceStartLocation );
	DecalTrace = None;

	TraceActor = Trace( out_HitLocation, out_HitNormal, TraceDest, TraceStart, FALSE, TraceExtent, HitInfo, TRACEFLAG_Bullet );

`if(`notdefined(FINAL_RELEASE))
	if( bShowDebugInfo )
	{
		DrawDebugLine( TraceStart, TraceDest , 255, 1, 1, TRUE );
		`log( "About to spawn a decal: " $ DecalData.DecalMaterial );
		ScriptTrace();
	}
`endif

	if( TraceActor != None )
	{
		//`log( "Blood DecalMaterial: " $ DecalData.DecalMaterial );
		//`log( Material(MaterialInstanceTimeVarying(DecalData.DecalMaterial).Parent).bUsedWithDecals );

		RandomScale = GetRangeValueByPct( DecalData.RandomScalingRange, FRand() );
		DecalData.Width *= RandomScale;
		DecalData.Height *= RandomScale;

		GD = GearGRI(WorldInfo.GRI).GOP.GetDecal_Blood( out_HitLocation );
		if( GD != none )
		{
			bIsHorde = WorldInfo.GRI.IsCoopMultiplayerGame();
			bIsSplitScreen = class'Engine'.static.IsSplitScreen();
			DecalLifeSpan = (bIsHorde|| bIsSplitScreen) ? DecalData.LifeSpan : 999999.0;

			GD.MITV_Decal.SetParent( DecalData.DecalMaterial );
			// @TODO:  pass in a struct with the decal params you want to use
			WorldInfo.MyDecalManager.SetDecalParameters( 
				GD, 
				GD.MITV_Decal, 
				out_HitLocation, 
				rotator(-out_HitNormal),
				bIsHorde ? (DecalData.Width*0.80f) : DecalData.Width,
				bIsHorde ? (DecalData.Height*0.80f) : DecalData.Height,
				Max(DecalData.Thickness,class'GearDecal'.default.MinExplosionThickness), 
				(bIsHorde || bIsSplitScreen) ? FALSE : !DecalData.ClipDecalsUsingFastPath, 
				RotationToUseForDecal, 
				HitInfo.HitComponent, 
				TRUE, 
				FALSE, 
				HitInfo.BoneName, 
				INDEX_NONE, 
				INDEX_NONE, 
				DecalLifeSpan, 
				INDEX_NONE, 
				class'GearDecal'.default.DepthBias,
				DecalData.BlendRange 
				);

			TraceActor.AttachComponent( GD );
			LocationOfLastBloodTrail = out_HitLocation;

			// if horde or splitscren then fade decals out instead of leaving them around for ever
			GD.MITV_Decal.SetDuration( DecalLifeSpan );

			//DecalTimeVaryingParams = DecalTimeVaryingParamsFunc;
			//DecalTimeVaryingParams( GD.MITV_Decal );
			//DecalTimeVaryingParams = None;

			`if(`notdefined(FINAL_RELEASE))
				if( bShowDebugInfo )
				{
					`log( "  SPAWNED " );
					//FlushPersistentDebugLines();
					DrawDebugCoordinateSystem( out_HitLocation, rotator(-out_HitNormal), 3.0f, TRUE );
				}
				`endif
		}
	}
}

// Gear_Blood.Decals.BloodSplatter02 // thick splatter

//Gear_Blood.Materials.DM_BloodImpact_Var01_Wall_01  // circular hit wall drip
// MaterialInstanceConstant'Gear_Blood.Materials.DM_BloodImpact_Var01_Wall_02' // wall splat drip

// MaterialInstanceConstant'Gear_Blood.Materials.DM_BloodSmear_Wall_Straight_01'  // straight wall drip




defaultproperties
{
}
