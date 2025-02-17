/** 
 * Bullet tracer projectile
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearProj_BulletTracer extends GearProj_Tracer;

var protected transient vector DestinationLoc;

var bool bAllowTracersMovingFromTarget;

function InitTracer(vector Start, vector End)
{
	local vector	Direction;
	local float		LifeTime;

	DestinationLoc = End;

	Direction = End - Start;
	super.Init( Normal(Direction) );

	LifeSpan	= 0.f;
	LifeTime	= VSize(Direction)/MaxSpeed-0.02f;
	SetTimer( LifeTime, FALSE, nameof(WakeProjectile) );

	SetHidden(False);
	Mesh.SetHidden(FALSE);
}

function Recycle()
{
	SetHidden(TRUE);
	Mesh.SetHidden(TRUE);
	bStasis = TRUE;

	bScaledUp		= TRUE;
	bScalingDown	= FALSE;

	SetPhysics( PHYS_None );

	ClearTimer( 'WakeProjectile' );
	Disable('Tick');
}

function Reset()
{
	Recycle();
}

/** Re-enables ticking.  Call once we're close to our destination, to "feel" for arrival in the last few frames. */
simulated function WakeProjectile()
{
	Enable('Tick');
}


simulated function Tick(float DeltaTime)
{
	super.Tick(DeltaTime);

	if ( ((DestinationLoc - Location) dot Velocity) < 0.f && !bAllowTracersMovingFromTarget )
	{
		// we passed the dest loc, lock to it and kill the tracer
		SetLocation(DestinationLoc);
		KillProjectile();
	}
}

simulated function KillProjectile()
{
	if( WorldInfo.NetMode != NM_DedicatedServer )
	{
		super.KillProjectile();
	}
	else
	{
		Recycle();
	}
}

simulated function ScaleDownFinishedNotify()
{
	Recycle();
}

simulated function ScaleUpFinishedNotify()
{
	super.ScaleUpFinishedNotify();
	if ( !IsTimerActive('WakeProjectile') )
	{
		// tracer woke up before going to sleep...
		Enable('Tick');
	}
}

/** Just in case. */
simulated event OutsideWorldBounds()
{
	Recycle();
}

/** Overridden because we don't want to explode on Pawns */
simulated function ProcessTouch(Actor Other, Vector HitLocation, Vector HitNormal)
{
	// Do not allow self damage
	if( Other == Instigator )
	{
		return;
	}

	// If can deal damage, then do so and destroy projectile. Otherwise, check if laser can bounce off.
	if( !Other.IsA('Projectile') ||	Other.bProjTarget )
	{
		if( Pawn(Other) != None )
		{
			// don't spawn impact effect on Pawns, just destroy.
			Recycle();
		}
		else
		{
			Explode(HitLocation, HitNormal);
		}
	}
}

defaultproperties
{
	bCollideWhenPlacing=FALSE
	Lifespan=0

	AccelRate=15000
	Speed=3500
	MaxSpeed=15000
	Damage=0
	MomentumTransfer=0
	MyDamageType=class'GDT_Ballistic'

	Begin Object Class=StaticMeshComponent Name=TracerMeshComp
    	StaticMesh=StaticMesh'COG_AssaultRifle.Mesh.S_Tracer_New'
		HiddenGame=FALSE
        CastShadow=FALSE
		CollideActors=FALSE
		BlockActors=FALSE
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=FALSE
		bDisableAllRigidBody=TRUE
        Scale3D=(X=1.0,Y=1.0,Z=1.0)
		bAcceptsLights=FALSE
		bAcceptsDynamicLights=FALSE
		bCastDynamicShadow=FALSE
		bAcceptsStaticDecals=FALSE
		bAcceptsDynamicDecals=FALSE
		bUseAsOccluder=FALSE
    End Object
	Mesh=TracerMeshComp
	Components.Add(TracerMeshComp)

	RemoteRole=ROLE_None
	bNetInitialRotation=TRUE

	bCollideActors=FALSE
	bCollideWorld=FALSE

	TracerDrawScale3D=(X=2.0f,Y=1.0f,Z=1.0f)
}
