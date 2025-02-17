/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTVehicleCTFGame extends UTCTFGame
	abstract;

// Returns whether a mutator should be allowed with this gametype
static function bool AllowMutator( string MutatorClassName )
{
	if ( (MutatorClassName ~= "UTGame.UTMutator_Instagib") || (MutatorClassName ~= "UTGame.UTMutator_WeaponsRespawn")
		|| (MutatorClassName ~= "UTGame.UTMutator_LowGrav") )
	{
		return false;
	}
	return Super.AllowMutator(MutatorClassName);
}

defaultproperties
{
	MapPrefixes[0]="VCTF"
	Acronym="VCTF"

	bAllowHoverboard=true
	bAllowTranslocator=false
	bStartWithLockerWeaps=true

	// Class used to write stats to the leaderboard
	OnlineStatsWriteClass=class'UTGame.UTLeaderboardWriteCTF'
	OnlineGameSettingsClass=class'UTGameSettingsVCTF'
	bMidGameHasMap=true
}
