class GearControlHelper extends Info
	native(Sequence)
	abstract;
	
cpptext
{
	void AddSpecToList( UReachSpec* newSpec, TArray<FCrossLevelReachSpec>& BlockedList );
	UReachSpec* GetSpec( INT Idx, TArray<FCrossLevelReachSpec>& BlockedList );
	FCrossLevelReachSpec* GetRef( ANavigationPoint* Start, ANavigationPoint* End, TArray<FCrossLevelReachSpec>& BlockedList, INT Height, INT Radius );

	void SetMatineePosition( UBOOL bEnd, TArray<USeqAct_Interp*>& MatineeList );
}

struct immutablewhencooked native CrossLevelReachSpec
{
	var() ActorReference	Start;
	var() ActorReference	End;
	var() class<ReachSpec>	SpecClass;
	var() ReachSpec			Spec;	// Only used temporarily
	var() INT				Height;
	var() INT				Radius;

	structcpptext
	{
		explicit FCrossLevelReachSpec( UReachSpec* InSpec )
		{
			Start.Actor = InSpec->Start;
			Start.Guid  = *InSpec->Start->GetGuid();
			End			= InSpec->End;
			SpecClass	= InSpec->GetClass();
			Spec		= InSpec;

			Height		= InSpec->CollisionHeight;
			Radius		= InSpec->CollisionRadius;
		}
	}
};

var SeqAct_Latent SeqObj;

var bool bHasCrossLevelPaths;

native function SetBlockingVolumeCollision( bool bCollide, out array<BlockingVolume> BlockingVolumes );

defaultproperties
{
	Components.Remove(Sprite)
}


