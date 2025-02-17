// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
class UTMutator_Handicap extends UTMutator;

var array< class<Inventory> > HandicapInventory;

/* called by GameInfo.RestartPlayer()
	change the players jumpz, etc. here
*/
function ModifyPlayer(Pawn P)
{
	local int i, HandicapNeed;
	local UTPawn PlayerPawn;
	local UTWeap_Enforcer Enforcer;

	PlayerPawn = UTPawn(P);
	if ( PlayerPawn == None )
	{
		return;
	}

	HandicapNeed = UTGame(WorldInfo.Game).GetHandicapNeed(PlayerPawn);

	if ( HandicapNeed > HandicapInventory.Length-1 )
	{
		// give a shieldbelt as well
		PlayerPawn.ShieldBeltArmor = Max(100, PlayerPawn.ShieldBeltArmor);
		HandicapNeed = HandicapInventory.Length - 1;
	}

	if ( HandicapNeed >= 1 )
	{
		PlayerPawn.VestArmor = Max(50, PlayerPawn.VestArmor);
	}
	if ( HandicapNeed >= 2 )
	{
		// always give an extra enforcer
		Enforcer = UTWeap_Enforcer(PlayerPawn.FindInventoryType(HandicapInventory[1]));
		if ( Enforcer != None )
		{
			Enforcer.DualMode = EDM_DualEquipping;
			Enforcer.BecomeDual();
		}
	}
	for ( i=2; i<HandicapNeed; i++ )
	{
		// Ensure we don't give duplicate items
		if (PlayerPawn.FindInventoryType( HandicapInventory[i] ) == None)
		{
			PlayerPawn.CreateInventory(HandicapInventory[i]);
		}
	}
	Super.ModifyPlayer(PlayerPawn);
}

defaultproperties
{
	GroupNames[0]="HANDICAP"

	HandicapInventory(0)=class'UTGame.UTWeap_Enforcer'
	HandicapInventory(1)=class'UTGame.UTWeap_RocketLauncher'
	//HandicapInventory(2)=class'UTGame.UTArmorPickup_Vest'
}


