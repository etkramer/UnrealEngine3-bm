/**
 * Custom decal class for Gears to simplify the process a bit.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearDecal extends DecalComponent
	native
	dependson(GearTypes);

cpptext
{
	virtual void Tick(FLOAT DeltaTime)
	{
		Super::Tick(DeltaTime);
		LifeSpan -= DeltaTime;
		if (LifeSpan <= 0.f)
		{
			DetachFromAny();
		}
		else
		{
			eventTick(DeltaTime);
		}
	}
}

/** Debug logging for decals, just uncomment the following line and all the debug logging will be enabled */
//`define DEBUG_DECALS
`if(`isdefined(DEBUG_DECALS))
    `define DecalLog(text)     `Log("["$self@GetFuncName()$"]"@`text,,'Decal');
`else
    `define DecalLog(text)
`endif

/** Pawn that owns us basically */
var Pawn Instigator;

/** Should each decal randomly alter rotation? */
var bool bRandomizeRotation;

/** Should each decal randomly scale the width/height? */
var bool bRandomizeScaling;
var vector2d RandomScalingRange;

/** Whether or not the decals should use the fast path to attach **/
var bool ClipDecalsUsingFastPath;

var float LifeSpan;

/** The vast majority of our decals have an MITV/MIC attached to them.  So we are just going to store that here.  So we don't have to keep allocating it **/
var MaterialInstanceTimeVarying MITV_Decal;

/** minimum thickness for explosion decals */
var float MinExplosionThickness;
/** minimum thickness for impact decals */
var float MinImpactThickness;

/** Because perf is slow for attached decals to FracturedMeshes we are going to have a percentage change to spawn them. **/
var float PercentToSpawnOnFracturedMesh;

/**
 * Stub for any decals that have time-based functionality (blood pools, etc).
 */
simulated event Tick(float DeltaTime);

/**
 * Contains any shared initialization between all attachment methods.
 */
simulated protected function SharedAttachmentInit(Actor TargetAttachment, vector RayDir)
{
	local float RandomScale;
    `DecalLog("Target:"@TargetAttachment);
	if (bRandomizeRotation)
	{
		DecalRotation = FRand() * 360.f;
	}
	if (bRandomizeScaling)
	{
		RandomScale = GetRangeValueByPct(RandomScalingRange,FRand());
		Width *= RandomScale;
		Height *= RandomScale;
	}
}

/**
 * Simplified interface for attaching decals to surfaces (bsp/meshes).
 */
final simulated function AttachToSurface(GearPawn Pawn, vector TraceLocation, vector TraceDirection, float TraceMagnitude)
{
	local Actor HitActor;
	local vector HitL, HitN;
	local TraceHitInfo HitInfo;

	if (Pawn != None)
    {
		Instigator = Pawn;
		HitActor = Pawn.Trace(HitL, HitN, TraceLocation + TraceDirection * TraceMagnitude * 2.f, TraceLocation, FALSE, vect(0,0,0), HitInfo, class'Actor'.const.TRACEFLAG_PhysicsVolumes);
		//Pawn.DrawDebugLine( TraceLocation, TraceLocation + TraceDirection * TraceMagnitude * 2.f, 255, 0, 0, TRUE);

		if (HitActor != None)
		{
			`DecalLog("Hit:"@HitActor);
			// if we've hit the world,
			if (HitActor.bStatic)
			{
				// redirect to the GRI so that we're attached to a non-static actor (and will properly receive ticks)
				HitActor = Pawn.WorldInfo.GRI;
			}
			Location = HitL;
			Orientation = rotator(-HitN);
			DecalRotation = 0;
			HitComponent = HitInfo.HitComponent;
			HitBone = HitInfo.BoneName;

			if( ClipDecalsUsingFastPath == TRUE )
			{
			    HitNodeIndex = HitInfo.Item;
			    HitLevelIndex = HitInfo.LevelIndex;
				bNoClip = FALSE;
			}
			else
			{
				HitNodeIndex = INDEX_NONE;
				HitLevelIndex = INDEX_NONE;
				bNoClip = TRUE;
			}

			FracturedStaticMeshComponentIndex = HitInfo.Item;

			//`log( HitActor @ "HitInfo.HitComponent" @ HitInfo.HitComponent @ "HitInfo.Item" @ HitInfo.Item @ "HitInfo.LevelIndex" @ HitInfo.LevelIndex );

			// shared init
			SharedAttachmentInit(HitActor,TraceDirection);
			// actual attachment
			HitActor.AttachComponent(self);
		}
		else
		{
			`DecalLog("Failed to hit anything");
		}
	}
}

/**
 * Helpful method of statically creating decals and attaching.
 */
final static simulated function GearDecal StaticAttachToSurface(class<GearDecal> DecalClass, GearPawn Pawn, vector TraceLocation, vector TraceDirection, float TraceMagnitude, const out ImpactInfo Impact, DecalData SourceDecalInfo)
{
	local GearDecal Decal;

	if (Pawn != None)
	{
		Decal = GearGRI(Pawn.WorldInfo.GRI).GOP.GetDecal_Bullet( Impact.HitLocation );
		if( Decal != none )
		{
			if (SourceDecalInfo.bIsValid)
			{
				Decal.CopyFromSource(SourceDecalInfo, Impact);
			}

			Decal.AttachToSurface(Pawn,TraceLocation,TraceDirection,TraceMagnitude);
		}
	}
	return Decal;
}
/**
 * Simplified interface for attaching decals from a ImpactInfo struct.
 */
final simulated function AttachFromImpact(Pawn InInstigator, const out ImpactInfo Impact)
{
	local Actor HitActor;

	`DecalLog("InInstigator:"@InInstigator@", HitActor:"@Impact.HitActor);

	HitActor = (Impact.HitActor.bStatic) ? Impact.HitActor.WorldInfo.GRI : Impact.HitActor;

	// we do a percent check here to see if we should spawn on the fractured static mesh or not
	if( ( FracturedStaticMeshActor(HitActor) != None ) && ( FRand() > PercentToSpawnOnFracturedMesh ) )
	{
		return;
	}

	Instigator = InInstigator;
	Location = Impact.HitLocation;
	Orientation = rotator(-Impact.HitNormal);
	DecalRotation = 0;
	HitComponent = Impact.HitInfo.HitComponent;
	HitBone = Impact.HitInfo.BoneName;

	if( ClipDecalsUsingFastPath == TRUE )
	{
		HitNodeIndex = Impact.HitInfo.Item;
		HitLevelIndex = Impact.HitInfo.LevelIndex;
	}
	else
	{
		HitNodeIndex = INDEX_NONE;
		HitLevelIndex = INDEX_NONE;
	}

	FracturedStaticMeshComponentIndex = Impact.HitInfo.Item;

	//`log( HitActor @ "Impact.HitInfo.HitComponent" @ Impact.HitInfo.HitComponent @ "Impact.HitInfo.Item" @ Impact.HitInfo.Item @ "Impact.HitInfo.LevelIndex" @ Impact.HitInfo.LevelIndex );

	// shared init
	SharedAttachmentInit(HitActor, Impact.RayDir);
	// actual attachment
	HitActor.AttachComponent(self);
}

/**
 * Helpful method of statically creating decals and attaching via ImpactInfo.
 */
final static simulated function GearDecal StaticAttachFromImpact(class<GearDecal> DecalClass, Pawn InInstigator, const out ImpactInfo Impact, optional DecalData SourceDecalInfo)
{
	local GearDecal Decal;
	local GearPawn GP;

	if (Impact.HitActor != None)
	{
		Decal = GearGRI(Impact.HitActor.WorldInfo.GRI).GOP.GetDecal_Bullet( Impact.HitLocation );
		if( Decal != none )
		{
			if (SourceDecalInfo.bIsValid)
			{
				Decal.CopyFromSource(SourceDecalInfo, Impact);
			}

			// okie here is our dirty hack for keeping a max number of decals on a pawn
			GP = GearPawn(Impact.HitActor);
			if( GP != none )
			{
				// so don't attach a decal if we are going to go over max
				if( ( GP.bAllowHitImpactDecalsOnSkelMesh == FALSE ) 
					|| ( GP.NumDecalsAttachedCurr+1 >= GP.MAX_DECALS_ATTACHED ) 
					|| ( GP.WorldInfo.GRI.IsCoopMultiplayerGame() ) // we do not want deccals on SkelMeshes in horde as it is just chaos and death
					|| ( class'Engine'.static.IsSplitScreen() )  // in SplitScreen do not attach decals to people as the RenderThread is over worked
					)
				{
					return none;
				}
				// otherwise go ahead and attach and increment
				else
				{
					GP.NumDecalsAttachedCurr++;
					if( ++GP.CurrDecalIdx >= GP.MAX_DECALS_ATTACHED )
					{
						GP.CurrDecalIdx = 0;
					}

					GP.DecalsAttached[GP.CurrDecalIdx] = Impact.HitActor.WorldInfo.TimeSeconds;
					if( !GP.IsTimerActive( 'DecalRemovalTimer' ) )
					{
						GP.SetTimer( 2.0f, FALSE, nameof(GP.RemoveDecalsFromSelf) );
					}
				}
			}

			Decal.AttachFromImpact(InInstigator,Impact);
			//Decal.SetLightEnvironment( Pawn.LightEnvironment );
		}
	}


	return Decal;
}

/**
 * Copy the relevant properties from a DecalInfo source (GearPhysicaGlobalBloodInfolMaterial).
 */
final simulated protected function CopyFromSource( DecalData Source, const out ImpactInfo Impact )
{
	local float DecalLifeSpan;

	// if we did not hit a pawn then set lifetime to be infi
	if( GearPawn(Impact.HitActor) == none )
	{
		DecalLifeSpan = 999999.0;
	}
	// else set it to be smaller
	else
	{	
		DecalLifeSpan = 15; //Source.LifeSpan;
	}

	if( MaterialInstanceTimeVarying(Source.DecalMaterial) != none )
	{
		//@todo need to cache these
		MITV_Decal.SetParent( Source.DecalMaterial );
		MITV_Decal.SetDuration( DecalLifeSpan );
		SetDecalMaterial(MITV_Decal);
	}
	else
	{
		SetDecalMaterial(Source.DecalMaterial);
	}

	Width = Source.Width;
	Height = Source.Height;
	FarPlane = Max(Source.Thickness,MinImpactThickness) * 0.5;
	NearPlane = -FarPlane;

	bRandomizeRotation = Source.bRandomizeRotation;
	bRandomizeScaling = TRUE;
	RandomScalingRange = Source.RandomScalingRange;
	ClipDecalsUsingFastPath = Source.ClipDecalsUsingFastPath;

	LifeSpan = DecalLifeSpan;
}


/** This is a nice helper function which is used to return a random decal from a passed in array **/
function static DecalData GetRandomDecalMaterial( const array<DecalData> DecalDataArray )
{
	local DecalData DecalData;

	if( DecalDataArray.Length > 0 )
	{
		return DecalDataArray[ Rand( DecalDataArray.Length ) ];
	}

	return DecalData;
}



defaultproperties
{
	MaxDrawDistance=4000

	// slightly larger depth bias (2X of default) to prevent z-fighting with static level-placed decals
	DepthBias=-0.0001

	BackfaceAngle=-0.001			
	bProjectOnBackfaces=TRUE

	// this is needed a the owner of this decal is "hidden" as it is a global entity @see UDecalComponent::IsEnabled()
	bIgnoreOwnerHidden=TRUE 
	bStaticDecal=FALSE
	LifeSpan=999999.0

	BlendRange=(X=80,Y=90)

	MinExplosionThickness=40
	MinImpactThickness=10

	PercentToSpawnOnFracturedMesh=0.60f
}
