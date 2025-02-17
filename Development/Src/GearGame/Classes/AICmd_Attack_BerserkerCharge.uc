class AICmd_Attack_BerserkerCharge extends AICommand_SpecialMove
	within GearAI_Berserker;


function Pushed()
{
	local Vector VectToTarg;

	super.Pushed();

	Focus = None;
	SetFocalPoint(ChargeLocation);
	StopFiring();

	// Update controller destination
	VectToTarg		 = ChargeLocation-Pawn.Location;
	VectToTarg.Z	 = 0;
	ChargeLocation  += Normal(VectToTarg) * 128.f;
	SetDestinationPosition( ChargeLocation, TRUE );

	// Tell Pawn in which direction we'd like to launch our attack
	Pawn.DesiredRotation = rotator(ChargeLocation - Pawn.Location);
	GearPawn_LocustBerserkerBase(Pawn).SetDesiredRootRotation(Pawn.DesiredRotation);

	ChargeBreakCount = default.ChargeBreakCount;
	ChargeHitList.Length = 0;

	// Preparing charge move - pawn needs to rotate towards target first
	bPreparingCharge	= TRUE;
	bPreparingMove		= TRUE;

	Pawn.SetAnchor( None );
}

function Popped()
{
	local GearPawn_LocustBerserkerBase P;

	super.Popped();

	bPreparingMove = FALSE;

	P = GearPawn_LocustBerserkerBase(Pawn);
	if (P != None)
	{
		P.EndSpecialMove();
		P.ClearDesiredRootRotation();
	}
}

auto state Command_SpecialMove
{
	function Tick( float DeltaTime )
	{
		Global.Tick( DeltaTime );

		if( !bPreparingCharge && bPreciseDestination )
		{
			if( VSizeSq(GetDestinationPosition()-ChargeLocation) > 400 )
			{
				// Lerp destination to charge location so she doesn't snap toward enemy
				SetDestinationPosition( VLerp( GetDestinationPosition(), ChargeLocation, DeltaTime ), TRUE );
				DesiredRotation	= Rotator(GetDestinationPosition()-Pawn.Location);
				DesiredRotation.Pitch = 0;
			}
			else
			{
				SetDestinationPosition( ChargeLocation, TRUE );
			}
		}
	}

	function ESpecialMove GetSpecialMove()
	{
		return SM_Berserker_Charge;
	}

	function DoChargeSlide()
	{
		//debug
		`AILog( GetFuncName() );

		if( MyGearPawn != None )
		{
			NextHearTime = WorldInfo.TimeSeconds + default.NextHearTime;
			MyGearPawn.DoSpecialMove( SM_Berserker_Slide, TRUE );
		}
	}

Begin:
	//debug
	`AILog( "BEGIN TAG"@GetStateName(), 'State' );

	FinishRotation();

	if( ExecuteSpecialMove() )
	{
		// Wait for pawn to be aligned to target
		if( bPreparingCharge )
		{
			do
			{
				sleep(0.1f);
			} until( !bPreparingCharge || GearPawn(Pawn) == None || GearPawn(Pawn).SpecialMove == SM_None );
		}

		do
		{
			//debug
			`AILog( "Waiting for SM to end"@GearPawn(Pawn).SpecialMove, 'Loop' );

			Sleep(0.1f);

			if( GearPawn(Pawn).IsDoingSpecialMove(SM_Berserker_Charge) )
			{
				if( !bPreciseDestination )
				{
					DoChargeSlide();
				}
			}
		} until (!bPreparingMove || GearPawn(Pawn) == None || GearPawn(Pawn).SpecialMove == SM_None);
		FinishedSpecialMove();
		Sleep( GetPostSpecialMoveSleepTime() );
	}
	else
	{
		//debug
		`AILog("Failed to do special move");

		Sleep(0.5f);
	}
	PopCommand(self);
}
