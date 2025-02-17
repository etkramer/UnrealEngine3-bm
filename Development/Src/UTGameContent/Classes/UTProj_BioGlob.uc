/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTProj_BioGlob extends UTProj_BioShot;

var() int			MaxRestingGlobStrength;
var repnotify int	GlobStrength;
var() float			GloblingSpeed;

var StaticMeshComponent GooLandedMesh;
var ParticleSystemComponent HitWallEffect;

var class<UTDamageType> GibDamageType;

replication
{
	if (bNetInitial)
		GlobStrength;
}

function InitBio(UTWeap_BioRifle_Content FiringWeapon, int InGlobStrength)
{
	// adjust speed
	InGlobStrength = Max(InGlobStrength, 1);
	Velocity = Normal(Velocity) * Speed * (0.4 + InGlobStrength)/(1.35*InGlobStrength);

	SetGlobStrength(InGlobStrength);
	RestTime = Default.RestTime + 0.6*InGlobStrength;
}

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	if (SkeletalMeshComponent(GooMesh) != None)
	{
		SkeletalMeshComponent(GooMesh).PlayAnim('Shake2', SkeletalMeshComponent(GooMesh).GetAnimLength('Shake2') * 0.5, true);
	}
}

simulated event ReplicatedEvent(name VarName)
{
	if (VarName == 'RemainingRestTime')
	{
		if (!IsInState('OnGround'))
		{
			Landed(vector(Rotation), None);
		}
	}
	else if (VarName == 'GlobStrength')
	{
		SetGlobStrength(GlobStrength);
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

/**
 * Sets the strength of this bio goo actor
 */
simulated function SetGlobStrength( int NewStrength )
{
	GlobStrength = Max(NewStrength,1);
	SetDrawScale(Sqrt((GlobStrength == 1) ? 1 : (GlobStrength + 1)) * default.DrawScale);
	if (GlobStrength > 4)
	{
		SetCollisionSize(CylinderComponent.CollisionRadius, CylinderComponent.CollisionHeight * 2.0);
	}

	// set different damagetype for charged shots
	if (GlobStrength > 1)
	{
		MyDamageType = default.MyDamageType;
	}
	else
	{
		MyDamageType = class'UTProj_BioShot'.default.MyDamageType;
	}
}

simulated function GrowCollision()
{
	local float CollRadius, CollHeight;

	CollRadius = 10 + 16 * Sqrt(GlobStrength);
	CollHeight = FMin(32, CollRadius);
	SetRotation(rot(0,0,0));
	CollisionComponent.SetTranslation((CollHeight/DrawScale) * SurfaceNormal);
	SetCollisionSize(CollRadius,CollHeight);
}

/**
 * Explode this glob
 */
simulated function Explode(Vector HitLocation, vector HitNormal)
{
	local float GlobVolume;

	if ( bExploded )
		return;

	bExploded = true;
	GlobVolume = Sqrt(GlobStrength);
	Damage = Default.Damage * GlobStrength;
	MomentumTransfer = Default.MomentumTransfer * GlobVolume;

	if ( ImpactedActor == None )
	{
		ImpactedActor = Base;
	}
	ProjectileHurtRadius(Damage, DamageRadius, MyDamageType, MomentumTransfer, HitLocation, HitNormal);
	SpawnExplosionEffects(HitLocation, HitNormal );
	Shutdown();
}

/**
 * Spawns several children globs
 */
simulated function SplashGloblings()
{
	local int g;
	local UTProj_BioShot NewGlob;
	local float GlobVolume;
	local Vector VNorm;

	if ( (GlobStrength > MaxRestingGlobStrength) && (UTPawn(Base) == None) )
	{
		if (Role == ROLE_Authority)
		{
			GlobVolume = Sqrt(GlobStrength);

			for (g=0; g<GlobStrength-MaxRestingGlobStrength; g++)
			{
				NewGlob = Spawn(class'UTProj_BioGlobling', self,, Location+GlobVolume*6*SurfaceNormal);
				if (NewGlob != None)
				{
					// init newglob
					NewGlob.Velocity = (GloblingSpeed + FRand()*150.0) * (SurfaceNormal + VRand()*0.8);
					if (Physics == PHYS_Falling)
					{
						VNorm = (Velocity dot SurfaceNormal) * SurfaceNormal;
						NewGlob.Velocity += (-VNorm + (Velocity - VNorm)) * 0.1;
					}
					NewGlob.InstigatorController = InstigatorController;
				}
			}
		}
		SetGlobStrength(MaxRestingGlobStrength);
	}
}

/**
  *  start dissolving pawn from goo - take damage over time, and give kill credit to goo launcher on any kill if enough damage left in glob
  * 1 glob strengths worth of damage per 0.18 seconds, and destroy glob when done.
  * be sure to gib on final damage if initial glob was > 4
  */
function DissolveFromGoo()
{
	local UTPawn UTP;

	UTP = UTPawn(Base);
	if (UTP == None)
	{
		Destroy();
	}
	else
	{
		if (UTP.AttachedProj != self)
		{
			if (UTP.AttachedProj == None)
			{
				UTP.AttachedProj = self;
			}
			else
			{
				UTP.TakeDamage(default.Damage * GlobStrength, UTP.AttachedProj.InstigatorController, UTP.AttachedProj.Location, vect(0,0,0), UTP.AttachedProj.MyDamageType);
				Destroy();
				return;
			}
		}
		UTP.TakeDamage(Damage, InstigatorController, Location, vect(0,0,0), MyDamageType);
		GlobStrength--;
		if (UTP.Health > 0 && GlobStrength > 0)
		{
			SetTimer(0.18, false, 'DissolveFromGoo');
		}
		else
		{
			if ( (UTP.Health <= 0) && (GlobStrength > 0) )
			{
				ProjectileHurtRadius(120, 220, UTP.AttachedProj.MyDamageType, MomentumTransfer, UTP.Location, vect(0,0,1));
			}
			UTP.AttachedProj = None;
			Destroy();
		}
	}
}

auto state Flying
{
	simulated event Landed(vector HitNormal, Actor FloorActor)
	{
		SurfaceNormal = HitNormal;
		SplashGloblings();
		// align and unhide the skeletal mesh version:
		if (WorldInfo.Netmode != NM_DedicatedServer && GooLandedMesh != none)
		{
			DetachComponent(GooMesh);
			AttachComponent(GooLandedMesh);
			HitWallEffect.ActivateSystem();
		}
		Super.Landed(HitNormal, FloorActor);
	}

	simulated function HitWall( Vector HitNormal, Actor HitWall, PrimitiveComponent WallComp )
	{
		SurfaceNormal = HitNormal;
		SplashGloblings();
		Super.HitWall(HitNormal, HitWall, WallComp);
	}

	simulated function ProcessTouch(Actor Other, Vector HitLocation, Vector HitNormal)
	{
		if ( !Other.bWorldGeometry )
		{
			RestTime = 1.0;
		}
		if ( UTPawn(Other) != None )
		{
			LifeSpan = 2.5;
			bCollideWorld = false;
			bProjTarget = false;
			SetCollision(false, false, false);
			SetPhysics(PHYS_None);
			SetLocation(0.25*HitLocation+0.75*Other.Location);
			SetBase(Other);
			SetDrawScale(Sqrt(GlobStrength) * default.DrawScale);
			SetOwner(Other);
			GooMesh.SetOwnerNoSee(true);

			if ( GlobStrength > 4 )
			{
				MyDamageType = GibDamageType;
			}
			DissolveFromGoo();
			GotoState('DissolvingPlayer');
		}
		else if ( !bExploded && (Other.bProjTarget || (UTProj_BioShot(Other) != None)) && (UTProj_BioGlob(Other) == None) )
		{
			SplashGloblings();
			Explode( HitLocation, HitNormal );
		}
   	}
}

simulated function MergeWithGlob(int AdditionalGlobStrength)
{
	SetGlobStrength(GlobStrength + AdditionalGlobStrength);
	if ( UTPawn(Base) == None )
	{
		RestTime += AdditionalGlobStrength;
		if (RemainingRestTime > 0.0)
		{
			RemainingRestTime += AdditionalGlobStrength;
		}
	}
	SplashGloblings();
}

state DissolvingPlayer
{
	simulated function ProcessTouch(Actor Other, Vector HitLocation, Vector HitNormal)
	{
	}

	simulated function AnimEnd(int Channel)
	{
	}
}

state OnGround
{
	simulated function Drip()
	{
		// @todo drop a globling
	}

	simulated function ProcessTouch(Actor Other, Vector HitLocation, Vector HitNormal)
	{
		local UTProj_BioGlob Glob;

		Glob = UTProj_BioGlob(Other);
		if ( Glob != None )
		{
			if (  (Glob.Owner != self) && (Owner != Glob) )
			{
				MergeWithGlob(GlobStrength);
				Glob.Destroy();
			}
		}
		else if ( !bExploded && (UTProj_BioGlobling(Other) == None)
					&& ((Other.bProjTarget && (Other != Base)) || (UTProj_BioShot(Other) != None)) )
		{
			Explode(Location, SurfaceNormal );
		}
   	}

	simulated function MergeWithGlob(int AdditionalGlobStrength)
	{
		SetGlobStrength(GlobStrength+AdditionalGlobStrength);
		SplashGloblings();
		GrowCollision();
		PlaySound(ImpactSound);
		bCheckedSurface = false;
		SetTimer(GetTimerCount() + AdditionalGlobStrength, false);
	}


	simulated function AnimEnd(int Channel)
	{
		local float DotProduct;

		if (!bCheckedSurface)
		{
			DotProduct = SurfaceNormal dot Vect(0,0,-1);
			if (DotProduct > 0.7)
			{
				//                PlayAnim('Drip', 0.66);
				SetTimer(DripTime, false, 'Drip');
				if (bOnMover)
				{
					Explode(Location, SurfaceNormal);
				}
			}
			else if (DotProduct > -0.5)
			{
				//                PlayAnim('Slide', 1.0);
				if (bOnMover)
				{
					Explode(Location, SurfaceNormal);
				}
			}
			bCheckedSurface = true;
		}
	}
}

defaultproperties
{
	Components.Empty();
	MaxRestingGlobStrength=6
	GlobStrength=1
	GloblingSpeed=200.0
	DamageRadius=120.0
	bProjTarget=true
	bNetTemporary=false
	DrawScale=0.5
	LifeSpan=20.0
	MyDamageType=class'UTDmgType_BioGoo_Charged'
	GibDamageType=class'UTDmgType_BioGooGib'
	bRotationFollowsVelocity=false

	Begin Object Name=CollisionCylinder
		CollisionRadius=20
		CollisionHeight=4
	End Object
	components.Add(CollisionCylinder)
	Explosionsound=SoundCue'A_Weapon_BioRifle.Weapon.A_BioRifle_FireAltImpactExplode_Cue'
	ImpactSound=SoundCue'A_Weapon_BioRifle.Weapon.A_BioRifle_FireAltImpactExplode_Cue'
	AmbientSound=SoundCue'A_Weapon_BioRifle.Weapon.A_BioRifle_Inair_Cue'

	Begin Object Class=StaticMeshComponent Name=ProjectileMeshFloor
		StaticMesh=StaticMesh'WP_BioRifle.Mesh.S_Bio_Blob_Chunk_Reduced'
		MaxDrawDistance=12000
		Scale=2.0
		bAcceptsDynamicDecals=FALSE
		bUseAsOccluder=FALSE
	End Object
	GooLandedMesh=ProjectileMeshFloor

	Begin Object class=AnimNodeSequence Name=MeshSequenceA
	End Object
	Begin Object Class=SkeletalMeshComponent Name=ProjectileSkelMeshAir
		SkeletalMesh=SkeletalMesh'WP_BioRifle.Mesh.SK_WP_Bio_Alt_Projectile_Blob'
		AnimSets(0)=AnimSet'WP_BioRifle.Anims.Bio_Alt_Blob_Projectile_Shake'
		Animations=MeshSequenceA
		bAcceptsDynamicDecals=FALSE
		bUseAsOccluder=FALSE
	End Object
	GooMesh=ProjectileSkelMeshAir
	Components.Add(ProjectileSkelMeshAir)

	FloorHit=none
	CeilingHit=none
	WallHit=none

	Begin Object Class=ParticleSystemComponent Name=HitWallFX
		Template=ParticleSystem'WP_BioRifle.Particles.P_WP_Bio_Alt_Blob_Impact'
		bAutoActivate=false
		SecondsBeforeInactive=1.0f
	End Object
	HitWallEffect=HitWallFX
	ProjExplosionTemplate=ParticleSystem'WP_BioRifle.Particles.P_WP_Bio_Alt_Blob_POP'
}
