class LambentBrumakInfo extends Info
	notplaceable
	config(Pawn);

/** Points to the Lambent skeletal mesh actor in the level */
var()		SkeletalMeshActor			LambentBrumak;

/** Skeletal mesh component grabbed from the placed actor */
var			SkeletalMeshComponent		Mesh;
/** Mask for playing animations on separate blisters */
var			GearAnim_BlendPerBone		BlisterMask;

/** Health lambent has left before player wins the fight */
var	config	int							Health;

/** Amount of time it takes for a blister to fully boil */
var config	float						BlisterBoilTime;
/** Amount of time it takes for a blister to get to hardened state */
var config	float						BlisterActiveTime;
/** Amount of time it takes for a blister to harden fully */
var	config	float						BlisterHardenedTime;
/** Amount of time it takes for a blister to be fully wounded */
var	config	float						BlisterWoundedTime;

enum EBlisterName
{
	BLISTER_02_Neck,
	BLISTER_05_Arm,
	BLISTER_06_Hump,
	BLISTER_07_Hump2,
	BLISTER_08_Back,
	BLISTER_09_Back2,
	BLISTER_10_Belly,
	BLISTER_11_Hip,
	BLISTER_13_RtFoot,
};

enum EBlisterState
{
	EBS_Inactive,
	EBS_Active,
	EBS_Hardened,
	EBS_Wounded,
};
/** Array containing info about all blister states */
var				EBlisterState				BlisterState[9];
/** Matching array to catch replicated state changes */
var	repnotify	EBlisterState				ReplicatedBlisterState[9];
/** Array of timers for active blisters */
var				float						BlisterTimer[9];

var				InterpActor					BlisterInterpActor[9];

function OnLambentBrumakControl( SeqAct_LambentBrumakControl inAction )
{
	local int Idx;

	if( inAction.InputLinks[0].bHasImpulse )
	{
		for( Idx = 0; Idx < inAction.Blisters.Length; Idx++ )
		{
			// Activate the given blister
			SetBlisterState( inAction.Blisters[Idx], EBS_Active );
		}
	}
}

simulated function PostBeginPlay()
{
	local int Idx;

	Super.PostBeginPlay();

	CacheAnimNodes();
	
	// Init mask properly
	for( Idx = 0; Idx < ArrayCount(BlisterState); Idx++ )
	{
		PlayBlisterAnim( Idx );
	}
}

simulated function CacheAnimNodes()
{
	local AnimNodeSequence Seq;

	Mesh = LambentBrumak.SkeletalMeshComponent;
	BlisterMask	= GearAnim_BlendPerBone(Mesh.FindAnimNode('BlisterMask'));

	foreach Mesh.AllAnimNodes( class'AnimNodeSequence', Seq )
	{
		if( Seq.NodeName == 'Active' )
		{
			// Scale anim rate to match active blister time
			Seq.Rate = Seq.GetAnimPlaybackLength() / BlisterBoilTime;
		}
		else
		if( Seq.NodeName == 'Hardened' )
		{
			// Scale anim rate to match hardened blister time
			Seq.Rate = Seq.GetAnimPlaybackLength() / BlisterHardenedTime;
		}
		else
		if( Seq.NodeName == 'Wounded' )
		{
			// Scale anim rate to match hardened blister time
			Seq.Rate = Seq.GetAnimPlaybackLength() / BlisterWoundedTime;
		}
	}
}

simulated event ReplicatedEvent( Name VarName )
{
	local int Idx;

	Super.ReplicatedEvent( VarName );

	if( VarName == 'ReplicatedBlisterState' )
	{
		// If a blister state change occured
		for( Idx = 0; Idx < ArrayCount(ReplicatedBlisterState); Idx++ )
		{
			// Find any states that don't match when we currently see
			if( ReplicatedBlisterState[Idx] != BlisterState[Idx] )
			{
				// Resolve the difference
				BlisterState[Idx] = ReplicatedBlisterState[Idx];
				// and update the blister morph animation
				PlayBlisterAnim( Idx );
			}
		}
	}
}

simulated function Tick( float DeltaTime )
{
	local int BlisterIdx;

	Super.Tick( DeltaTime );

	if( Role == ROLE_Authority )
	{
		// Update active blisters to hardened if timer expires 
		for( BlisterIdx = 0; BlisterIdx < ArrayCount(BlisterTimer); BlisterIdx++ )
		{
			if( BlisterTimer[BlisterIdx] > 0.f )
			{
				BlisterTimer[BlisterIdx] -= DeltaTime;
				if( BlisterTimer[BlisterIdx] <= 0.f )
				{
					SetBlisterState( BlisterIdx, EBS_Hardened );
				}
			}
		}
	}
}

function SetBlisterState( int BlisterIdx, EBlisterState NewState )
{
	BlisterState[BlisterIdx] = NewState;
	if( BlisterState[BlisterIdx] == EBS_Active )
	{
		BlisterTimer[BlisterIdx] = BlisterActiveTime;
	}
	else
	{
		BlisterTimer[BlisterIdx] = 0.f;

		if( BlisterState[BlisterIdx] == EBS_Hardened )
		{
			BlisterInterpActor[BlisterIdx].TriggerEventClass( class'SeqEvent_LambentBlister', self, 0 );
		}
		else
		if( BlisterState[BlisterIdx] == EBS_Wounded )
		{
			BlisterInterpActor[BlisterIdx].TriggerEventClass( class'SeqEvent_LambentBlister', self, 1 );
		}
	}

	PlayBlisterAnim( BlisterIdx );
}

simulated function PlayBlisterAnim( int BlisterIdx )
{
	local AnimNodeBlendList BlendList;

	BlendList = AnimNodeBlendList(BlisterMask.Children[BlisterIdx+1].Anim);

//	`log( GetFuncName()@BlisterIdx@BlisterState[BlisterIdx]@BlendList@BlendList.NodeName);

	if( BlisterState[BlisterIdx] == EBS_Inactive )
	{
		// Disable the mask for this blister
		BlisterMask.SetMaskWeight( BlisterIdx, 0.f, 0.f );
	}
	else
	if( BlisterState[BlisterIdx] == EBS_Active )
	{
		// Enable the mask for this blister
		BlisterMask.SetMaskWeight( BlisterIdx, 1.f, 0.1f );

		// Play active morph anim
		BlendList.SetActiveChild( 0, 0.1f );
	}
	else
	if( BlisterState[BlisterIdx] == EBS_Hardened )
	{
		// Play hardened morph anim
		BlendList.SetActiveChild( 1, 0.1f );
	}
	else
	if( BlisterState[BlisterIdx] == EBS_Wounded )
	{
		// Play wounded morph anim
		BlendList.SetActiveChild( 2, 0.1f );
	}
}

function HandleDamage( int DamageAmount )
{
	Health -= DamageAmount;
	if( Health <= 0 )
	{
		// Fire kismet event to end the boss fight
	}
}

defaultproperties
{
	RemoteRole=ROLE_SimulatedProxy
	DrawScale=10.f

	SupportedEvents.Empty
	SupportedEvents(0)=class'SeqEvent_LambentBrumakKilled'
}