
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_EvadeFwd extends GSM_Evade;

protected function bool InternalCanDoSpecialMove()
{
	return InternalCanDoSpecialMoveEvade_Worker(SM_EvadeFwd);
}

simulated function PreProcessInput(GearPlayerInput Input)
{
	// call the super class's version
	super.PreProcessInput(Input);

	// force running forwards
	Input.aBaseY	= default.AccelScale;

	// strafe becomes turning
	//Input.aTurn		= Input.aStrafe * default.EvadeTurnPct;
	//Input.aStrafe	= 0.f;
}


defaultproperties
{
	BS_EvadeStart=(AnimName[BS_FullBody]="AR_Evade_Fwd_Start")
	BS_EvadeEnd=(AnimName[BS_FullBody]="AR_Evade_Fwd_End")
}
