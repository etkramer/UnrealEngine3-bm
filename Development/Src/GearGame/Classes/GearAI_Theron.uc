/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAI_Theron extends GearAI_Locust;

function TriggerAttackGUDS()
{
	// don't do attack event, let torquebowcharging event do it's thing instead
	if (GearWeap_BowBase(Pawn.Weapon) == None)
	{
		super.TriggerAttackGUDS();
	}
}