class AICmd_Attack_Leviathan_Mouth extends AICommand_Base_Combat
	within GearAI_Leviathan;
	
var GearAI_Leviathan			 LeviAI;
var GearPawn_LocustLeviathanBase LeviPawn;
var bool						 bAttackComplete;

/** World time expiration if player doesn't finish stage */
var			float		FailAttackTime;
/** World time after completion that we can finish this command */
var			float		AllowCompleteTime;

	
function Pushed()
{
	LeviAI	 = GearAI_Leviathan(AIOwner);
	LeviPawn = GearPawn_LocustLeviathanBase(LeviAI.Pawn);

	Super.Pushed();

	LeviPawn.NumTentaclesHurt		= 0;
	LeviPawn.bMouthGrenadeSuccess	= FALSE;
	LeviPawn.bMouthClosedFail		= FALSE;

	FailAttackTime  = LeviPawn.WorldInfo.TimeSeconds + LeviPawn.DurationMouthAttack;
	bAttackComplete = FALSE;

	ChooseNextAttack();
	SetTimer( 1.f, TRUE, nameof(self.CheckStageComplete), self );
}

function Popped()
{
	Super.Popped();

	ClearTimer( 'ChooseNextAttack', self );
	ClearTimer( 'CheckStageComplete', self );

	ClearLatentAction( class'SeqAct_Leviathan_Mouth', !LeviPawn.bMouthGrenadeSuccess );

	// If grenade successful, open jaws and close 2 secs later
	// (So Leviathan can be pulled off barge)
	if( LeviPawn.bMouthGrenadeSuccess )
	{
		LeviPawn.OpenJaw( 2.f );
	}

	// Reset everything
	LeviPawn.NumTentaclesHurt		= 0;
	LeviPawn.bMouthGrenadeSuccess	= FALSE;
	LeviPawn.bMouthClosedFail		= FALSE;
	LeviPawn.StopAllTentacleAttacks();

	CurrentMouthSeq = None;
}

function bool CanAttack()
{
	if( LeviPawn == None || LeviPawn.bMouthGrenadeSuccess || LeviPawn.bMouthClosedFail || bAttackComplete || LeviAI == None )
	{
		return FALSE;
	}

	// If only one player in the mouth, stop attacks -- coop allow attacks to continue
	if( LeviPawn.bMouthOpen && ValidMouthTargets.Length < 2 )
	{
		return FALSE;
	}

	return TRUE;
}

function ChooseNextAttack()
{
	local int	Idx;
	local Actor Targ;

	//debug
	`AILog( GetFuncName()@CanAttack() );

	if( CanAttack() )
	{
		Targ = LeviAI.GetMouthTarget();
		Idx  = LeviPawn.GetNextAttackTentacle();
		if( Idx >= 0 && Targ != None )
		{
			//debug
			`AILog( "Start attack ..."@Idx@Targ );

			LeviPawn.StartTentacleAttack( Idx, Targ );
		}

		SetTimer( RandRange( LeviPawn.DelayBetweenMouthTentacleAttacks.X, LeviPawn.DelayBetweenMouthTentacleAttacks.Y), FALSE, nameof(self.ChooseNextAttack), self );
	}
}

function CheckStageComplete()
{
	if( !bAttackComplete )
	{
		bAttackComplete = LeviPawn.bMouthGrenadeSuccess || LeviPawn.bMouthClosedFail || (LeviAI.WorldInfo.TimeSeconds > FailAttackTime);
		AllowCompleteTime = WorldInfo.TimeSeconds + 0.25f; // delay a sec for mouth animation to work
	}

	if( bAttackComplete && !LeviPawn.AnyTentaclesAttacking() && WorldInfo.TimeSeconds > AllowCompleteTime )
	{
		Status = 'Success';
		PopCommand( self );
	}
}

defaultproperties
{
	InitialTransitionCheckTime=(X=0.f,Y=0.f)
}