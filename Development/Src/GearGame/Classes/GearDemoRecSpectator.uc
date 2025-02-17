/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearDemoRecSpectator extends GearPC;

/** local copy of RealViewTarget as the C++ code might clear it in some cases we don't want to for demo spectators */
var PlayerReplicationInfo MyRealViewTarget;

/** if set, camera rotation is always forced to viewtarget rotation */
var config bool bLockRotationToViewTarget;

/** If set, automatically switches players every AutoSwitchPlayerInterval seconds */
var config bool bAutoSwitchPlayers;
/** Interval to use if bAutoSwitchPlayers is TRUE */
var config float AutoSwitchPlayerInterval;

/** player we should start out viewing, set during recording to the local player */
var repnotify PlayerReplicationInfo InitialViewPRI;

replication
{
	if (bNetInitial)
		InitialViewPRI;
}

simulated event PostBeginPlay()
{
	Super.PostBeginPlay();

	if ( PlayerReplicationInfo != None )
	{
		PlayerReplicationInfo.bOutOfLives = true;
	}
}

simulated event ReceivedPlayer()
{
	local PlayerController PC;

	Super.ReceivedPlayer();

	// DemoRecSpectators don't go through the login process, so manually set up initial info for the demo
	if (Role == ROLE_Authority && WorldInfo.Game != None)
	{
		ClientSetHUD(WorldInfo.Game.HUDType, WorldInfo.Game.ScoreBoardType);

		WorldInfo.Game.ReplicateStreamingStatus(self);

		// see if we need to spawn a CoverReplicator for this player
		if (WorldInfo.Game.GetCoverReplicator() != None)
		{
			SpawnCoverReplicator();
		}

		foreach LocalPlayerControllers(class'PlayerController', PC)
		{
			InitialViewPRI = PC.PlayerReplicationInfo;
			break;
		}
	}
}

function InitPlayerReplicationInfo()
{
	Super.InitPlayerReplicationInfo();
	PlayerReplicationInfo.PlayerName = "DemoRecSpectator";
	PlayerReplicationInfo.bIsSpectator = true;
	PlayerReplicationInfo.bOnlySpectator = true;
	PlayerReplicationInfo.bOutOfLives = true;
	PlayerReplicationInfo.bWaitingPlayer = false;
}

simulated event ReplicatedEvent(name VarName)
{
	local Pawn P;

	if (VarName == nameof(InitialViewPRI))
	{
		foreach WorldInfo.AllPawns(class'Pawn', P)
		{
			if (P.PlayerReplicationInfo == InitialViewPRI)
			{
				break;
			}
		}
		SetViewTarget((P != None) ? P : self); // P might be None if no valid target found in iterator
		MyRealViewTarget = InitialViewPRI;
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

function bool SetPause(bool bPause, optional delegate<CanUnpause> CanUnpauseDelegate = CanUnpause)
{
	// allow the spectator to pause demo playback
	if (WorldInfo.NetMode == NM_Client)
	{
		WorldInfo.Pauser = (bPause) ? PlayerReplicationInfo : None;
		return true;
	}
	else
	{
		return false;
	}
}

exec function SloMo(float NewTimeDilation)
{
	WorldInfo.DemoPlayTimeDilation = NewTimeDilation;
}

exec function ViewClass( class<actor> aClass, optional bool bQuiet, optional bool bCheat )
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
		SetViewTarget(first);
	}
	else
		SetViewTarget(self);
}

//==== Called during demo playback ============================================

exec function DemoViewNextPlayer()
{
	local Pawn P, Pick;
	local bool bFound;

	// view next player
	foreach WorldInfo.AllPawns(class'Pawn', P)
	{
		if (P.PlayerReplicationInfo != None)
		{
			if (Pick == None)
			{
				Pick = P;
			}
			if (bFound)
			{
				Pick = P;
				break;
			}
			else
			{
				bFound = (RealViewTarget == P.PlayerReplicationInfo || ViewTarget == P);
			}
		}
	}

	SetViewTarget(Pick);
}

unreliable server function ServerViewNextPlayer()
{
	DemoViewNextPlayer();
}
unreliable server function ServerViewPrevPlayer()
{
	DemoViewNextPlayer();
}

function SetViewTarget(Actor NewViewTarget, optional ViewTargetTransitionParams TransitionParams)
{
	Super.SetViewTarget(NewViewTarget, TransitionParams);

	// this check is so that a Pawn getting gibbed doesn't break finding that player again
	// must manually clear MyRealViewTarget when player controlled switch back to viewing self
	if (NewViewTarget != self)
	{
		MyRealViewTarget = RealViewTarget;
	}
}

unreliable server function ServerViewSelf(optional ViewTargetTransitionParams TransitionParams)
{
	Super.ServerViewSelf(TransitionParams);

	MyRealViewTarget = None;
}

unreliable server function ServerViewFirstSpectatorPoint()
{
	foreach DynamicActors(class'GearSpectatorPoint', CurrSpectatorPoint)
	{
		break;
	}

	if (CurrSpectatorPoint != None)
	{
		SetViewTarget(CurrSpectatorPoint);
		MyRealViewTarget = None;
	}
	else
	{
		// otherwise send them to a player
		DemoViewNextPlayer();
	}
}

unreliable server function ServerViewNextSpectatorPoint()
{
	ViewSpectatorPoint(true);
}

unreliable server function ServerViewPrevSpectatorPoint()
{
	ViewSpectatorPoint(false);
}

function ViewSpectatorPoint(bool bNext)
{
	local GearSpectatorPoint NextSpectatorPoint;
	local array<GearSpectatorPoint> SpectatorPointList;
	local int Index;

	foreach DynamicActors(class'GearSpectatorPoint', NextSpectatorPoint)
	{
		SpectatorPointList.AddItem(NextSpectatorPoint);
	}

	if (SpectatorPointList.length > 0)
	{
		Index = SpectatorPointList.Find(CurrSpectatorPoint);
		if (Index != INDEX_NONE)
		{
			if (bNext)
			{
				Index = (Index + 1) % SpectatorPointList.length;
			}
			else if (Index == 0)
			{
				Index = SpectatorPointList.length - 1;
			}
			else
			{
				Index--;
			}
		}
		else
		{
			Index = 0;
		}
		CurrSpectatorPoint = SpectatorPointList[Index];
		SetViewTarget(CurrSpectatorPoint);
		MyRealViewTarget = None;
	}
	else
	{
		DemoViewNextPlayer();
	}
}

/** used to construct complete AILogs from demo playback */
//@note: the demorec driver doesn't like it when this is "reliable" (too much data?) but it doesn't matter because
//	the demo connection never runs out of bandwidth
unreliable client function RecordAILog(GearAI AI, string FinalStr)
{
	if (AI != None)
	{
		if (AI.AILogFile == None)
		{
			AI.AILogFile = Spawn(class'FileLog');
			AI.AILogFile.OpenLog(AI, ".ailog");
		}
		AI.AILogFile.Logf(FinalStr);
	}
}

auto state Spectating
{
	function BeginState(Name PreviousStateName)
	{
		Super.BeginState(PreviousStateName);

		if( bAutoSwitchPlayers )
		{
			SetTimer( AutoSwitchPlayerInterval, true, nameof(DemoViewNextPlayer) );
		}
	}

	exec function StartFire(optional byte FireModeNum)
	{
		DemoViewNextPlayer();
	}

	event PlayerTick( float DeltaTime )
	{
		local Pawn P;

		Global.PlayerTick( DeltaTime );

		// attempt to find a player to view.
		if (Role == ROLE_AutonomousProxy)
		{
			if (RealViewTarget == None && MyRealViewTarget != None)
			{
				RealViewTarget = MyRealViewTarget;
			}

			// reacquire ViewTarget if the player switched Pawns
			if ( RealViewTarget != None && RealViewTarget != PlayerReplicationInfo &&
				(Pawn(ViewTarget) == None || Pawn(ViewTarget).PlayerReplicationInfo != RealViewTarget) )
			{
				foreach WorldInfo.AllPawns(class'Pawn', P)
				{
					if (P.PlayerReplicationInfo == RealViewTarget)
					{
						SetViewTarget(P);
						break;
					}
				}
			}

			if (Pawn(ViewTarget) != None)
			{
				TargetViewRotation = ViewTarget.Rotation;
				TargetViewRotation.Pitch = Pawn(ViewTarget).RemoteViewPitch << 8;
			}
		}
	}

	function UpdateRotation(float DeltaTime)
	{
		local rotator NewRotation;

		if (bLockRotationToViewTarget)
		{
			SetRotation(ViewTarget.Rotation);
		}
		else
		{
			Super.UpdateRotation(DeltaTime);
		}

		if (Rotation.Roll != 0)
		{
			NewRotation = Rotation;
			NewRotation.Roll = 0;
			SetRotation(NewRotation);
		}
	}
}

state ScreenshotMode
{
	function UpdateRotation(float DeltaTime)
	{
		Super(GearPC).UpdateRotation(DeltaTime);
	}

	function BeginState(Name PreviousStateName)
	{
		local vector CamLoc;
		local rotator CamRot;
		LastPawn = MyGearPawn;
		Super.BeginState(PreviousStateName);
		// get the current camera position so we can start free cam in the same spot
		GetPlayerViewPoint(CamLoc,CamRot);
		SetViewTarget(self);
		SetLocation(CamLoc);
		SetRotation(CamRot);
	}

	function PlayerMove(float DeltaTime)
	{
		if (!IsButtonActive(GB_LeftTrigger))
		{
			Super.PlayerMove(DeltaTime);
		}
	}

	function EndState(Name NextStateName)
	{
		Super.EndState(NextStateName);
		SetViewTarget(LastPawn);
	}

	function PlayerTick(float DeltaTime )
	{
		Super(GearPC).PlayerTick(DeltaTime);
	}
};

defaultproperties
{
	RemoteRole=ROLE_AutonomousProxy
	bDemoOwner=1
}
