class AICmd_Nemacyst_HomeInOnEnemy extends AICommand
	within GearAI_Nemacyst;

/** GoW global macros */
var GearPawn_LocustNemacystBase CachedPawnRef;

static function bool InitCommandUserActor( GearAI AI, Actor UserActor )
{
	local AICommand Cmd;

	if( AI != None )
	{
		Cmd = new(GearAI_Nemacyst(AI)) Default.Class;
		if( Cmd != None )
		{
			AI.Enemy = Pawn(UserActor);
			AI.PushCommand(Cmd);
			return TRUE;
		}
	}

	return FALSE;
}

function Pushed()
{
	Super.Pushed();
	Pawn.Mesh.SetRotation(rot(-16384,0,0));
	FoundEnemyAndHomingIn();
	CachedPawnRef = GearPawn_LocustNemacystBase(MyGearPawn);
	GotoState('Action_Homing');
}

function Popped()
{
	Super.Popped();

	Pawn.AirSpeed = Pawn.default.AirSpeed;
	Pawn.AccelRate = Pawn.default.Accelrate;
}

state Action_Homing `DEBUGSTATE
{

	
	function BeginState(Name PreviousStateName)
	{
		Super.BeginState(PreviousStateName);

		CachedPawnRef.AirSpeed = CachedPawnRef.ChargeMaxSpeed;
		CachedPawnRef.AccelRate = CachedPawnRef.ChargeMaxSpeed / CachedPawnRef.ChargeSpeedRampTime;
		//`log(GetFuncName()@GetStateName()@self@CachedPawnRef.ChargeMaxSpeed@CachedPawnRef.ChargeMaxSpeed / CachedPawnRef.ChargeSpeedRampTime);
	}

Begin:
	//debug
	`AILog( "BEGIN TAG", 'State' );

	Focus = Enemy;
	// if far enough away then swoop in first
	if( Abs(Enemy.Location.Z - Pawn.Location.Z) > 512.f)
	{
		//debug
		`AIlog("Swooping towards enemy, z delta:"@Abs(Enemy.Location.Z - Pawn.Location.Z));

		Sleep( GearPawn_LocustNemacystBase(Pawn).PlaySwoopAnimation() );

		GearPawn_LocustNemacystBase(Pawn).StopSwoopAnimation();
	}

	`AILog("Enemy:"@Enemy@Enemy.Health@"LOS:"@LineOfSightTo(Enemy));
	while (Enemy != None && Enemy.Health > 0 && LineOfSightTo(Enemy))
	{
		if( !CachedPawnRef.bStartAttack )
		{
			CachedPawnRef.SetAttackAnim( TRUE );
		}

		Focus = Enemy;
		// Torpedo spin
		RotationRate.Roll = 16384;
		Pawn.bRollToDesired = TRUE;

		// First time we spin, pick a direction randomly.
		if( SpinDir == 0.f )
		{
			SpinDir = FRand() > 0.5f ? 1.f : -1.f;
		}

		SetDestinationPosition( GetDestinationNearEnemy(), TRUE );

		//debug
		`AILog("Moving to enemy");

		MoveTo(GetDestinationPosition(),Enemy);

		//debug
		`AILog("- reached destination");

		if (VSize(Enemy.Location - Pawn.Location) <= 256.f)
		{
			//debug
			`AILog("Exploding on enemy");

			if (Pawn != None)
			{
				// bit of a hack to workaround our pawn being in a package lower than us
				Pawn.Landed(vector(Pawn.Rotation),Enemy);
			}
		}

		Pawn.bRollToDesired = FALSE;
	}

	if( Pawn.Health > 0 )
	{
		CachedPawnRef.SetAttackAnim( FALSE );
	}

	GotoState('DelaySuccess');



}