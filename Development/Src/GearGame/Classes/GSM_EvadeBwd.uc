
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_EvadeBwd extends GSM_Evade;

protected function bool InternalCanDoSpecialMove()
{
	return InternalCanDoSpecialMoveEvade_Worker(SM_EvadeBwd);
}

simulated function PreProcessInput(GearPlayerInput Input)
{
	// call the super class's version
	Super.PreProcessInput(Input);

	// force running backwards
	Input.aBaseY	= -default.AccelScale;

	// strafe becomes turning
	//Input.aTurn		= -Input.aStrafe * default.EvadeTurnPct;
	//Input.aStrafe	= 0.f;
}



defaultproperties
{
	BS_EvadeStart=(AnimName[BS_FullBody]="AR_Evade_Bwd_Start")
	BS_EvadeEnd=(AnimName[BS_FullBody]="AR_Evade_Bwd_End")
}
