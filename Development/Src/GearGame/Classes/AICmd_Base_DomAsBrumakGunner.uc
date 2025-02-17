class AICmd_Base_DomAsBrumakGunner extends AICommand_Base_Combat
	within GearAI_Cover;


function Pushed()
{
	Super.Pushed();

	GotoState( 'InCombat' );
}

state InCombat
{
Begin:
	//debug
	`AILog( "BEGIN TAG", 'State' );

	Stop;
/*
	if( !SelectTarget() )
	{
		//debug
		`AILog( "Failed to find target" );

		Sleep( 1.f );
	}

	if( HasValidTarget() )
	{
		if( !CanFireAt(FireTarget, Pawn.GetWeaponStartTraceLocation()) )
		{
			//debug
			`AILog("Unable to fire at target");
		}
		else
		{
			Focus = FireTarget;
			StartFiring();
			SetTimer( 1.5f, FALSE, nameof(StopFiring) );
			do
			{
				Sleep(0.25f);

				//debug
				`AILog( "Firing weapon...", 'Loop' );
			} until( !IsFiringWeapon() );
			ClearTimer( 'StopFiring' );
		}
	}
	else
	{
		//debug
		`AILog( "Not able to fire at enemy", 'Combat' );
	}

	Sleep(0.5f);
	Goto( 'Begin' );
*/
}

defaultproperties
{
	InitialTransitionCheckTime=(X=0.f,Y=0.f)
}