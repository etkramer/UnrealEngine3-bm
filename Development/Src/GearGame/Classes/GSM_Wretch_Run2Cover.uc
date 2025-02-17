
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Wretch_Run2Cover extends GSM_Run2Cover;

defaultproperties
{
	BS_TravelAnimation=(AnimName[BS_FullBody]="run_fwd2cov_start")
	BS_TravelAnimationMirrored=(AnimName[BS_FullBody]="run_fwd2cov_start") //@fixme laurent -- needs mirrored version
	BS_ImpactAnimation=(AnimName[BS_FullBody]="run_fwd2cov_stop")
}
