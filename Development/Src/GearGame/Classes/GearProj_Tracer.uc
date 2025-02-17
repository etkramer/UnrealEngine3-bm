/** 
 * Tracer Projectile
 * Base class for tracer-like projectile types.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearProj_Tracer extends GearProjectile;

/** Acceleration rate of projectile */
var()	float				AccelRate;
/** Mesh used by projectile */
var()	PrimitiveComponent	Mesh;
/** Time in seconds taken by the mesh to grow */
var()	float				MeshScaleUpTime;
/** Time in seconds taken by the mesh to shrink.  Calculated programamatically. */
var		transient float		MeshScaleDownTime;
/** Has mesh fully scaled up? */
var		bool				bScaledUp;
/** Is Mesh scaling down? */
var		bool				bScalingDown;
/** Current mesh scale. */
var		float				CurScale;

/** This is the draw scale 3D for this tracer **/
var vector TracerDrawScale3D;

/** Check on various replicated data and act accordingly. */
simulated event ReplicatedEvent( name VarName )
{
	super.ReplicatedEvent( VarName );

	if ( (VarName == 'Velocity') || (VarName == 'InitialFrameOfRefVelocity') )
	{
		CalcTracerAccel();
	}
}

simulated protected function CalcTracerAccel()
{
	local vector AccelDir;

	// if adding base vel, we still want to accelerate along the local-frame component of the velocity
	AccelDir = bAddBaseVelocity ? (Velocity - InitialFrameOfRefVelocity) : Velocity;
	Acceleration = Normal(AccelDir) * AccelRate;
}

/** Overridden to set initial acceleration */
function Init(vector Direction)
{
	super.Init( Direction );

	CalcTracerAccel();
	bScaledUp		= FALSE;
	bScalingDown	= FALSE;
	CurScale		= 0.f;

	Enable('Tick');
	
	// force update on first frame
	Tick(0.f);
}


/**
 * If not on dedicated server, then freeze projectile and scale staticmesh down.
 * Because mesh can be big, and just destroying it is too harsh.
 * On Dedicated server, destroy right away.
 */

simulated function KillProjectile()
{
	if( WorldInfo.NetMode != NM_DedicatedServer )
	{
		// calculate this so the tracer's apparent speed stays the same.
		// i.e. the tail doesn't appear to slow down just because the head stopped.
		MeshScaleDownTime = Mesh.Bounds.BoxExtent.X * 2.f / VSize(Velocity);

		// Delay projectile's destruction until it has fully shrunk
		Enable('Tick');	
		SetPhysics( PHYS_None );
		bScaledUp		= true;
		bScalingDown	= true;
	}
	else
	{
		Destroy();
	}
}

/** Projectile's destruction */
simulated function Explode(vector HitLocation, vector HitNormal)
{
	SpawnImpactEffect( HitLocation, HitNormal );
	KillProjectile();
}

simulated function SpawnImpactEffect( vector HitLocation, vector HitNormal );

simulated function ScaleDownFinishedNotify()
{
	Destroy();
}

simulated function ScaleUpFinishedNotify()
{
	bScaledUp		= TRUE;
	Mesh.SetScale3D( default.Mesh.Scale3D );
	Disable('Tick');
}

simulated function Tick( float DeltaTime )
{
	local Vector	NewScale;

	super.Tick( DeltaTime );

	if( Mesh != None )
	{
		NewScale = default.Mesh.Scale3D;

		// If scaling down
		if( bScalingDown )
		{
			CurScale -= DeltaTime / MeshScaleDownTime;

			if (CurScale > 0.f)
			{
				NewScale.X *= CurScale;
				Mesh.SetScale3D( NewScale );
			}
			else
			{
				ScaleDownFinishedNotify();
			}
		}
		// If hasn't been scaled up yet, then do it
		else if( !bScaledUp )
		{
			CurScale += DeltaTime / MeshScaleUpTime;

			if( CurScale < 1.f )
			{
				NewScale.X *= CurScale;
				Mesh.SetScale3D( NewScale );
			}
			else
			{
				ScaleUpFinishedNotify();
			}
		}
	}
}


defaultproperties
{
	TickGroup=TG_DuringAsyncWork
	MeshScaleUpTime=0.15f

	// remove light environment from tracers
	Begin Object Name=MyLightEnvironment
		bEnabled=FALSE
	End Object
	Components.Remove(MyLightEnvironment)
	ProjLightEnvironment=None

	TracerDrawScale3D=(X=1.0f,Y=1.0f,Z=1.0f)
}
