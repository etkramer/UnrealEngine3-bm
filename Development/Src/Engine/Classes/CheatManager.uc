//=============================================================================
// CheatManager
// Object within playercontroller that manages "cheat" commands
// only spawned in single player mode
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================

class CheatManager extends Object within PlayerController
	native;

exec function ListDynamicActors()
{
`if(`notdefined(FINAL_RELEASE))
	local Actor A;
	local int i;

	ForEach DynamicActors(class'Actor',A)
	{
		i++;
		`log(i@A);
	}
	`log("Num dynamic actors: "$i);
`endif
}

exec function FreezeFrame(float delay)
{
	WorldInfo.Game.SetPause(Outer,Outer.CanUnpause);
	WorldInfo.PauseDelay = WorldInfo.TimeSeconds + delay;
}

exec function WriteToLog( string Param )
{
	`log("NOW! "$Param);
}

exec function KillViewedActor()
{
	if ( ViewTarget != None )
	{
		if ( (Pawn(ViewTarget) != None) && (Pawn(ViewTarget).Controller != None) )
			Pawn(ViewTarget).Controller.Destroy();
		ViewTarget.Destroy();
		SetViewTarget(None);
	}
}

/* Teleport()
Teleport to surface player is looking at
*/
exec function Teleport()
{
	local Actor		HitActor;
	local vector	HitNormal, HitLocation;
	local vector	ViewLocation;
	local rotator	ViewRotation;

	GetPlayerViewPoint( ViewLocation, ViewRotation );

	HitActor = Trace(HitLocation, HitNormal, ViewLocation + 1000000 * vector(ViewRotation), ViewLocation, true);
	if ( HitActor != None)
		HitLocation += HitNormal * 4.0;

	ViewTarget.SetLocation( HitLocation );
}

/*
Scale the player's size to be F * default size
*/
exec function ChangeSize( float F )
{
	Pawn.CylinderComponent.SetCylinderSize( Pawn.Default.CylinderComponent.CollisionRadius * F, Pawn.Default.CylinderComponent.CollisionHeight * F );
	Pawn.SetDrawScale(F);
	Pawn.SetLocation(Pawn.Location);
}

/* Stop interpolation
*/
exec function EndPath()
{
}

exec function Amphibious()
{
	Pawn.UnderwaterTime = +999999.0;
}

exec function Fly()
{
	if ( (Pawn != None) && Pawn.CheatFly() )
	{
		ClientMessage("You feel much lighter");
		bCheatFlying = true;
		Outer.GotoState('PlayerFlying');
	}
}

exec function Walk()
{
	bCheatFlying = false;
	if (Pawn != None && Pawn.CheatWalk())
	{
		Restart(false);
	}
}

exec function Ghost()
{
	if ( (Pawn != None) && Pawn.CheatGhost() )
	{
		bCheatFlying = true;
		Outer.GotoState('PlayerFlying');
	}
	else
	{
		bCollideWorld = false;
	}

	ClientMessage("You feel ethereal");
}

/* AllAmmo
	Sets maximum ammo on all weapons
*/
exec function AllAmmo();

exec function God()
{
	if ( bGodMode )
	{
		bGodMode = false;
		ClientMessage("God mode off");
		return;
	}

	bGodMode = true;
	ClientMessage("God Mode on");
}

/**
 * Some games have God Mode not actually be god mode but be "don't take damage mode".  So we need to have another
 * flag that says to not be affected by effects (e.g. momentum transfer, hit effects, etc.)
 **/
exec function AffectedByHitEffects()
{
	if ( bAffectedByHitEffects )
	{
		bAffectedByHitEffects = false;
		ClientMessage("EffectsAffect mode off");
		return;
	}

	bAffectedByHitEffects = true;
	ClientMessage("EffectsAffect Mode on");
}

exec function SloMo( float T )
{
	WorldInfo.Game.SetGameSpeed(T);
}

exec function SetJumpZ( float F )
{
	Pawn.JumpZ = F;
}

exec function SetGravity( float F )
{
	WorldInfo.WorldGravityZ = F;
}

exec function SetSpeed( float F )
{
	Pawn.GroundSpeed = Pawn.Default.GroundSpeed * f;
	Pawn.WaterSpeed = Pawn.Default.WaterSpeed * f;
}

exec function KillAll(class<actor> aClass)
{
	local Actor A;
`if(`notdefined(FINAL_RELEASE))
	local PlayerController PC;

	foreach WorldInfo.AllControllers(class'PlayerController', PC)
	{
		PC.ClientMessage("Killed all "$string(aClass));
	}
`endif

	if ( ClassIsChildOf(aClass, class'AIController') )
	{
		WorldInfo.Game.KillBots();
		return;
	}
	if ( ClassIsChildOf(aClass, class'Pawn') )
	{
		KillAllPawns(class<Pawn>(aClass));
		return;
	}
	ForEach DynamicActors(class 'Actor', A)
		if ( ClassIsChildOf(A.class, aClass) )
			A.Destroy();
}

// Kill non-player pawns and their controllers
function KillAllPawns(class<Pawn> aClass)
{
	local Pawn P;

	WorldInfo.Game.KillBots();
	ForEach DynamicActors(class'Pawn', P)
		if ( ClassIsChildOf(P.Class, aClass)
			&& !P.IsPlayerPawn() )
		{
			if ( P.Controller != None )
				P.Controller.Destroy();
			P.Destroy();
		}
}

exec function KillPawns()
{
	KillAllPawns(class'Pawn');
}

/**
 * Possess a pawn of the requested class
 */
exec function Avatar( name ClassName )
{
	local Pawn			P, TargetPawn, FirstPawn, OldPawn;
	local bool			bPickNextPawn;

	Foreach DynamicActors(class'Pawn', P)
	{
		if( P == Pawn )
		{
			bPickNextPawn = TRUE;
		}
		else if( P.IsA(ClassName) )
		{
			if( FirstPawn == None )
			{
				FirstPawn = P;
			}

			if( bPickNextPawn )
			{
				TargetPawn = P;
				break;
			}
		}
	}

	// if we went through the list without choosing a pawn, pick first available choice (loop)
	if( TargetPawn == None )
	{
		TargetPawn = FirstPawn;
	}

	if( TargetPawn != None )
	{
		// detach TargetPawn from its controller and kill its controller.
		TargetPawn.DetachFromController( TRUE );

		// detach player from current pawn and possess targetpawn
		if( Pawn != None )
		{
			OldPawn = Pawn;
			Pawn.DetachFromController();
		}

		Possess(TargetPawn, FALSE);

		// Spawn default controller for our ex-pawn (AI)
		if( OldPawn != None )
		{
			OldPawn.SpawnDefaultController();
		}
	}
	else
	{
		`log("Avatar: Couldn't find any Pawn to possess of class '" $ ClassName $ "'");
	}
}

exec function Summon( string ClassName )
{
	local class<actor> NewClass;
	local vector SpawnLoc;

	`log( "Fabricate " $ ClassName );
	NewClass = class<actor>( DynamicLoadObject( ClassName, class'Class' ) );
	if( NewClass!=None )
	{
		if ( Pawn != None )
			SpawnLoc = Pawn.Location;
		else
			SpawnLoc = Location;
		Spawn( NewClass,,,SpawnLoc + 72 * Vector(Rotation) + vect(0,0,1) * 15 );
	}
}

/**
 * Give a specified weapon to the Pawn.
 * If weapon is not carried by player, then it is created.
 * Weapon given is returned as the function's return parmater.
 */
exec function Weapon GiveWeapon( String WeaponClassStr )
{
	Local Weapon		Weap;
	local class<Weapon> WeaponClass;

	WeaponClass = class<Weapon>(DynamicLoadObject(WeaponClassStr, class'Class'));
	Weap		= Weapon(Pawn.FindInventoryType(WeaponClass));
	if( Weap != None )
	{
		return Weap;
	}
	return Weapon(Pawn.CreateInventory( WeaponClass ));
}

exec function PlayersOnly()
{
	WorldInfo.bPlayersOnly = !WorldInfo.bPlayersOnly;
}

/** Util for fracturing meshes within an area of the player. */
exec function DestroyFractures(optional float Radius)
{
	local FracturedStaticMeshActor FracActor;

	if(Radius == 0.0)
	{
		Radius = 256.0;
	}

	foreach CollidingActors(class'FracturedStaticMeshActor', FracActor, Radius, Pawn.Location, TRUE)
	{
		if(FracActor.Physics == PHYS_None)
		{
			// Make sure the impacted fractured mesh is visually relevant
			FracActor.BreakOffPartsInRadius(Pawn.Location, Radius, 500.0, TRUE);
		}
	}
}

/** Util for ensuring at least one piece is broken of each FSM in level */
exec function FractureAllMeshes()
{
	local FracturedStaticMeshActor FracActor;

	foreach AllActors(class'FracturedStaticMeshActor', FracActor)
	{
		FracActor.HideOneFragment();
	}
}

/** This will break all Fractured meshes in the map in a way to maximize memory usage **/
exec function FractureAllMeshesToMaximizeMemoryUsage()
{
	local FracturedStaticMeshActor FracActor;

	foreach AllActors(class'FracturedStaticMeshActor', FracActor)
	{
		FracActor.HideFragmentsToMaximizeMemoryUsage();
	}
}



// ***********************************************************
// Navigation Aids (for testing)

// remember spot for path testing (display path using ShowDebug)
exec function RememberSpot()
{
	if ( Pawn != None )
		SetDestinationPosition( Pawn.Location );
	else
		SetDestinationPosition( Location );
}

// ***********************************************************
// Changing viewtarget

exec function ViewSelf(optional bool bQuiet)
{
	Outer.ResetCameraMode();
	if ( Pawn != None )
		SetViewTarget(Pawn);
	else
		SetViewtarget(outer);
	if (!bQuiet )
		ClientMessage(OwnCamera, 'Event');

	FixFOV();
}

exec function ViewPlayer( string S )
{
	local Controller P;

	foreach WorldInfo.AllControllers(class'Controller', P)
	{
		if ( P.bIsPlayer && (P.PlayerReplicationInfo.GetPlayerAlias() ~= S ) )
		{
			break;
		}
	}

	if ( P.Pawn != None )
	{
		ClientMessage(ViewingFrom@P.PlayerReplicationInfo.GetPlayerAlias(), 'Event');
		SetViewTarget(P.Pawn);
	}
}

exec function ViewActor( name ActorName)
{
	local Actor A;

	ForEach AllActors(class'Actor', A)
		if ( A.Name == ActorName )
		{
			SetViewTarget(A);
	    SetCameraMode('ThirdPerson');
			return;
		}
}

exec function ViewFlag()
{
	local AIController C;

	foreach WorldInfo.AllControllers(class'AIController', C)
	{
		if (C.PlayerReplicationInfo != None && C.PlayerReplicationInfo.bHasFlag)
		{
			SetViewTarget(C.Pawn);
			return;
		}
	}
}

exec function ViewBot()
{
	local actor first;
	local bool bFound;
	local AIController C;

	foreach WorldInfo.AllControllers(class'AIController', C)
	{
		if (C.Pawn != None && C.PlayerReplicationInfo != None)
		{
			if (bFound || first == None)
			{
				first = C;
				if (bFound)
				{
					break;
				}
			}
			if (C.PlayerReplicationInfo == RealViewTarget)
			{
				bFound = true;
			}
		}
	}

	if ( first != None )
	{
		`log("view "$first);
		SetViewTarget(first);
		SetCameraMode( 'ThirdPerson' );
		FixFOV();
	}
	else
		ViewSelf(true);
}

exec function ViewClass( class<actor> aClass )
{
	local actor other, first;
	local bool bFound;

	first = None;

	ForEach AllActors( aClass, other )
	{
		if ( bFound || (first == None) )
		{
			first = other;
			if ( bFound )
				break;
		}
		if ( other == ViewTarget )
			bFound = true;
	}

	if ( first != None )
	{
		if ( Pawn(first) != None )
			ClientMessage(ViewingFrom@First.GetHumanReadableName(), 'Event');
		else
			ClientMessage(ViewingFrom@first, 'Event');
		SetViewTarget(first);
		FixFOV();
	}
	else
		ViewSelf(false);
}

exec function Loaded()
{
	if( WorldInfo.Netmode!=NM_Standalone )
		return;

    AllWeapons();
    AllAmmo();
}

/* AllWeapons
	Give player all available weapons
*/
exec function AllWeapons()
{
	// subclass me
}

/** streaming level debugging */

function SetLevelStreamingStatus(name PackageName, bool bShouldBeLoaded, bool bShouldBeVisible)
{
	local PlayerController PC;
	local int i;

	if (PackageName != 'All')
	{
		foreach WorldInfo.AllControllers(class'PlayerController', PC)
		{
			PC.ClientUpdateLevelStreamingStatus(PackageName, bShouldBeLoaded, bShouldBeVisible, FALSE );
		}
	}
	else
	{
		foreach WorldInfo.AllControllers(class'PlayerController', PC)
		{
			for (i = 0; i < WorldInfo.StreamingLevels.length; i++)
			{
				PC.ClientUpdateLevelStreamingStatus(WorldInfo.StreamingLevels[i].PackageName, bShouldBeLoaded, bShouldBeVisible, FALSE );
			}
		}
	}
}

exec function StreamLevelIn(name PackageName)
{
	SetLevelStreamingStatus(PackageName, true, true);
}

exec function OnlyLoadLevel(name PackageName)
{
	SetLevelStreamingStatus(PackageName, true, false);
}

exec function StreamLevelOut(name PackageName)
{
	SetLevelStreamingStatus(PackageName, false, false);
}

/**
 * Toggle between debug camera/player camera without locking gameplay and with locking
 * local player controller input.
 */
exec function ToggleDebugCamera()
{
	local PlayerController PC;
	local DebugCameraController DCC;

	foreach WorldInfo.AllControllers(class'PlayerController', PC)
	{
		if ( PC.bIsPlayer && PC.IsLocalPlayerController() )
		{
			DCC = DebugCameraController(PC);
			if( DCC!=none && DCC.OryginalControllerRef==none )
			{
				//dcc are disabled, so we are looking for normal player controller
				continue;
			}
			break;
		}
	}

	if( DCC!=none && DCC.OryginalControllerRef!=none )
	{
		DCC.DisableDebugCamera();
	}
	else if( PC!=none )
	{
		PC.EnableDebugCamera();
	}
}

exec function TestLevel()
{
	local Actor A, Found;
	local bool bFoundErrors;

	ForEach AllActors(class'Actor', A)
	{
		bFoundErrors = bFoundErrors || A.CheckForErrors();
		if ( bFoundErrors && (Found == None) )
			Found = A;
	}

	if ( bFoundErrors )
	{
		`log("Found problem with "$Found);
		assert(false);
	}
}

`if(`notdefined(FINAL_RELEASE))
/**
 * Logs the current session state for the game type and online layer
 */
exec function DumpOnlineSessionState()
{
	local int PlayerIndex;
	local UniqueNetId NetId;

	if (WorldInfo.NetMode != NM_Client)
	{
		`Log("");
		`Log("GameInfo state");
		`Log("-------------------------------------------------------------");
		`Log("");
		// Log game info data
		`Log("Class: "$WorldInfo.Game.Class.Name);
		// Log player count information
		`Log("  MaxPlayersAllowed: "$WorldInfo.Game.MaxPlayersAllowed);
		`Log("  MaxPlayers: "$WorldInfo.Game.MaxPlayers);
		`Log("  NumPlayers: "$WorldInfo.Game.NumPlayers);
		`Log("  MaxSpectatorsAllowed: "$WorldInfo.Game.MaxSpectatorsAllowed);
		`Log("  MaxSpectators: "$WorldInfo.Game.MaxSpectators);
		`Log("  NumSpectators: "$WorldInfo.Game.NumSpectators);
		`Log("  NumBots: "$WorldInfo.Game.NumBots);

		`Log("  bUseSeamlessTravel: "$WorldInfo.Game.bUseSeamlessTravel);
		`Log("  bRequiresPushToTalk: "$WorldInfo.Game.bRequiresPushToTalk);
		`Log("  bHasNetworkError: "$WorldInfo.Game.bHasNetworkError);

		`Log("  OnlineGameSettingsClass: "$WorldInfo.Game.OnlineGameSettingsClass);
		`Log("  OnlineStatsWriteClass: "$WorldInfo.Game.OnlineStatsWriteClass);

		`Log("  bUsingArbitration: "$WorldInfo.Game.bUsingArbitration);
		if (WorldInfo.Game.bUsingArbitration)
		{
			`Log("  bHasArbitratedHandshakeBegun: "$WorldInfo.Game.bHasArbitratedHandshakeBegun);
			`Log("  bNeedsEndGameHandshake: "$WorldInfo.Game.bNeedsEndGameHandshake);
			`Log("  bIsEndGameHandshakeComplete: "$WorldInfo.Game.bIsEndGameHandshakeComplete);
			`Log("  bHasEndGameHandshakeBegun: "$WorldInfo.Game.bHasEndGameHandshakeBegun);
			`Log("  ArbitrationHandshakeTimeout: "$WorldInfo.Game.ArbitrationHandshakeTimeout);
			`Log("  Number of pending arbitration PCs: "$WorldInfo.Game.PendingArbitrationPCs.Length);
			// List who we are waiting of for arbitration
			for (PlayerIndex = 0; PlayerIndex < WorldInfo.Game.PendingArbitrationPCs.Length; PlayerIndex++)
			{
				`Log("    Player: "$WorldInfo.Game.PendingArbitrationPCs[PlayerIndex].PlayerReplicationInfo.PlayerName$" PC ("$WorldInfo.Game.PendingArbitrationPCs[PlayerIndex].Name$")");
			}
			`Log("  Number of arbitration PCs: "$WorldInfo.Game.ArbitrationPCs.Length);
			// List all of the players that have completed arbitration
			for (PlayerIndex = 0; PlayerIndex < WorldInfo.Game.ArbitrationPCs.Length; PlayerIndex++)
			{
				`Log("    Player: "$WorldInfo.Game.ArbitrationPCs[PlayerIndex].PlayerReplicationInfo.PlayerName$" PC ("$WorldInfo.Game.ArbitrationPCs[PlayerIndex].Name$")");
			}
		}
	}
	// Log the PRI information for comparison with the online session state
	if (WorldInfo.GRI != None)
	{
		`Log("  Number of PRI players: "$WorldInfo.GRI.PRIArray.Length);
		// List the PRIs that are here to find mismatches between the online state
		for (PlayerIndex = 0; PlayerIndex < WorldInfo.GRI.PRIArray.Length; PlayerIndex++)
		{
			NetId = WorldInfo.GRI.PRIArray[PlayerIndex].UniqueId;
			`Log("    Player: "$WorldInfo.GRI.PRIArray[PlayerIndex].PlayerName$" UID ("$class'OnlineSubsystem'.static.UniqueNetIdToString(NetId)$") PC ("$WorldInfo.GRI.PRIArray[PlayerIndex].Owner$")");
		}
		`Log("");
	}

	// Log the online session state
	if (OnlineSub != None)
	{
		OnlineSub.DumpSessionState();
	}
}
`endif

`if(`notdefined(FINAL_RELEASE))
/**
 * Logs the current muting state of the server
 */
exec function DumpVoiceMutingState()
{
	local UniqueNetId NetId;
	local PlayerController PC;
	local int MuteIndex;

	`Log("");
	`Log("Voice state");
	`Log("-------------------------------------------------------------");
	`Log("");
	// Log the online view of the voice state
	if (OnlineSub != None)
	{
		OnlineSub.DumpVoiceRegistration();
	}
	// Only the server has the information
	if (WorldInfo.NetMode != NM_Client)
	{
		`Log("Muting state");
		// For each player list their gameplay mutes and system wide mutes
		foreach WorldInfo.AllControllers(class'PlayerController', PC)
		{
			`Log("  Player: "$PC.PlayerReplicationInfo.PlayerName);
			`Log("    Gameplay mutes: ");
			for (MuteIndex = 0; MuteIndex < PC.GameplayVoiceMuteList.Length; MuteIndex++)
			{
				NetId = PC.GameplayVoiceMuteList[MuteIndex];
				`Log("      "$class'OnlineSubsystem'.static.UniqueNetIdToString(NetId));
			}
			`Log("    System mutes: ");
			for (MuteIndex = 0; MuteIndex < PC.VoiceMuteList.Length; MuteIndex++)
			{
				NetId = PC.VoiceMuteList[MuteIndex];
				`Log("      "$class'OnlineSubsystem'.static.UniqueNetIdToString(NetId));
			}
			`Log("    Voice packet filter: ");
			for (MuteIndex = 0; MuteIndex < PC.VoicePacketFilter.Length; MuteIndex++)
			{
				NetId = PC.VoicePacketFilter[MuteIndex];
				`Log("      "$class'OnlineSubsystem'.static.UniqueNetIdToString(NetId));
			}
			`Log("");
		}
	}
}
`endif

/**
 * Changes the OS specific logging level
 *
 * @param DebugLevel the new debug level to use
 */
exec function SetOnlineDebugLevel(int DebugLevel)
{
	if (OnlineSub != None)
	{
		OnlineSub.SetDebugSpewLevel(DebugLevel);
	}
}

exec function Online(int Command, optional int Parameter)
{
    if (OnlineSub != none)
    {
        OnlineSub.OnlineDebugCommand(Command, Parameter);
    }
}

defaultproperties
{
}
