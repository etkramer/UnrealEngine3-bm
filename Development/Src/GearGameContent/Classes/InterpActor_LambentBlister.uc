class InterpActor_LambentBlister extends InterpActor
	placeable;

/** Index to the blister to handle damage for */
var()	EBlisterName Blister;
/** Amount of damage to take before blister pops open */
var()	int	DamageThreshold;
/** Amount of health this blister has left */
var		int	Health;

/** Info handling lambent brumak blisters */
var LambentBrumakInfo Info;

simulated function PostBeginPlay()
{
	local LambentBrumakInfo LBI;

	Super.PostBeginPlay();

	// Grab LambentBrumakInfo reference
	foreach WorldInfo.AllActors( class'LambentBrumakInfo', LBI )
	{
		Info = LBI;
		break;
	}
	Info.BlisterInterpActor[Blister] = self;

	// Set default health
	Health = DamageThreshold;
	// Make sure collision blocks weapons
	SetCollisionType(COLLIDE_BlockWeapons);
}

event TakeDamage( int DamageAmount, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser )
{
	Super.TakeDamage( DamageAmount, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, DamageCauser );

	// If blister is open
	if( Info != None && 
		(Info.BlisterState[Blister] == EBS_Active  ||
		 Info.BlisterState[Blister] == EBS_Wounded) )
	{
		if( Health > 0 )
		{
			// Update health
			Health -= DamageAmount;
			// If blister takes enough damage to pop open
			if( Health <= 0 )
			{
				Info.SetBlisterState( Blister, EBS_Wounded );
			}
		}
		// Just pass on damage to info for handling of sack of hit points
		Info.HandleDamage( DamageAmount );
	}
}

defaultproperties
{
	Begin Object Name=StaticMeshComponent0
		StaticMesh=StaticMesh'z-explosionvolumes.SphereFlameCloudy'
		Materials(0)=Material'base_color_materials.Glow-4'
	End Object

	DamageThreshold=100

	SupportedEvents.Add(class'SeqEvent_LambentBlister')
}