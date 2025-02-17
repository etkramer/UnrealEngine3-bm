class InterpActor_RaftBase extends InterpActor_GearBasePlatform
	native
	notplaceable;

var protected StaticMeshComponent EntrywayCollision;


simulated function AlignComponentProperly(StaticMeshComponent SMC)
{
	SMC.SetTranslation(StaticMeshComponent.Translation);
	SMC.SetRotation(StaticMeshComponent.Rotation);
	SMC.SetScale(StaticMeshComponent.Scale);
	SMC.SetScale3D(StaticMeshComponent.Scale3D);
}

simulated function PostBeginPlay()
{
	local StaticMeshComponent C;

	super.PostBeginPlay();

	foreach ComponentList(class'StaticMeshComponent', C)
	{
		if (C != StaticMeshComponent)
		{
			AlignComponentProperly(C);
		}
	}
}

simulated function OnRaftControl(SeqAct_RaftControl Action)
{
	if (Action.InputLinks[0].bHasImpulse)
	{
		// 0 is Allow Entry
		DetachComponent(EntrywayCollision);
	}
	else
	{
		// 1 is Disallow Entry
		AttachComponent(EntrywayCollision);
		AlignComponentProperly(EntrywayCollision);
	}
}

defaultproperties
{
	BlockRigidBody=TRUE
	bCollideActors=TRUE
	bBlockActors=TRUE

	Begin Object Name=MyLightEnvironment
		bEnabled=TRUE
	End Object

	// this obj is designed solely to provide collision for the "doorway" where the player gets on
	Begin Object Class=StaticMeshComponent Name=EntryWayCollision0
	End Object
	EntrywayCollision=EntryWayCollision0

	// this obj is designed solely to provide collision for rigid bodies
	Begin Object Class=StaticMeshComponent Name=RBCollision0
	End Object
	Components.Add(RBCollision0)

	bAlwaysConfineToClampedBase=TRUE
}