class InterpActor_LeviathanEye extends InterpActor
	placeable;

/** Amount of damage to take before squeezing eyes shut */
var()	int	DamageThreshold;
/** Amount of health this eye has left */
var		int	Health;

/** Leviathan pawn eye is based on */
var GearPawn_LocustLeviathanBase LeviPawn;

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	// Grab leviathan reference
	LeviPawn = GearPawn_LocustLeviathanBase(Base);
	// Set default health
	Health = DamageThreshold;
	// Make sure collision blocks weapons
	SetCollisionType(COLLIDE_BlockWeapons);
}

event TakeDamage( int DamageAmount, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser )
{
	Super.TakeDamage( DamageAmount, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, DamageCauser );

	// If Levi eyes are open
	if( LeviPawn != None && LeviPawn.bEyeOpen )
	{
		// Update health
		Health -= DamageAmount;
		// If eye takes enough daamge
		if( Health <= 0 )
		{
			// Reset health
			Health = DamageThreshold;
			// Sqeeze eyes shut
			LeviPawn.CloseEyes( TRUE );
		}
	}
}

defaultproperties
{
	Begin Object Name=StaticMeshComponent0
		StaticMesh=StaticMesh'z-explosionvolumes.SphereFlameCloudy'
		Materials(0)=Material'base_color_materials.Glow-4'
	End Object
}