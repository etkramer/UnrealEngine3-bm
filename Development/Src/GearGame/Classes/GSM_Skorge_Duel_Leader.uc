
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Skorge_Duel_Leader extends GSM_InteractionPawnLeader_Base;

/** Flags to pack for replication */
const	DUEL_WIN	= 1;
const	DUEL_LOSE	= 2;
const	DUEL_DRAW	= 4;

/** TRUE if dueling properly finished. FALSE if it was aborted too early */
var bool				bProperlyFinished;
/** Duel animation */
var GearPawn.BodyStance	BS_SkorgeDuel_Staff, BS_SkorgeDuel_2Stick, BS_SkorgeDuel_1Stick;

var float	MoveEndTime;
var Array<CameraAnim>	CameraAnimStartList;
var CameraAnimInst		CameraAnimInstance;

simulated function GearPawn.BodyStance GetBS()
{
	local GearPawn_LocustSkorgeBase Skorge;

	Skorge = GearPawn_LocustSkorgeBase(PawnOwner);
	if( Skorge.Stage == SKORGE_Staff )
	{
		return BS_SkorgeDuel_Staff;
	}
	else
	if( Skorge.Stage == SKORGE_TwoStick )
	{
		return BS_SkorgeDuel_2Stick;
	}
	else
	{
		return BS_SkorgeDuel_1Stick;
	}
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	// Reset flag for new Duel.
	bProperlyFinished = FALSE;
	Super.SpecialMoveStarted(bForced, PrevMove);
}

function StartInteraction()
{
	// Play dueling animation on both characters
	PlayDuelingStartAnimation(PawnOwner);
	PlayDuelingStartAnimation(Follower);

	PawnOwner.SetTimer( 2.2f, FALSE, nameof(self.DuelingMiniGameStart), self );

	// Start movement
	MoveStart();
}

function MoveStart()
{
	local Vector	Marker, VectToVictim, ToVictimDir, Destination;
	local float		Distance;

	VectToVictim = Follower.Location - PawnOwner.Location;
	VectToVictim.Z = 0.f;
	ToVictimDir = Normal(VectToVictim);

	Marker = vect(1,0,0) * AlignDistance;
	Destination = Follower.Location + RelativeToWorldOffset(Rotator(-ToVictimDir), Marker);
	SetReachPreciseDestination(Destination);

	Distance = VSize2D(Destination - PawnOwner.Location);
	PawnOwner.GroundSpeed = (Distance / MoveEndTime);
	PawnOwner.SetTimer( MoveEndTime, FALSE, nameof(self.MoveEndTimeOut), self );

	// Make follower face his attacker
	Follower.SpecialMoves[Follower.SpecialMove].SetFacePreciseRotation( Rotator(-ToVictimDir), 0.25f);
	`log(PawnOwner.WorldInfo.TimeSeconds @ GetFuncName() @ "Distance:" @ Distance @ "GroundSpeed:" @ PawnOwner.GroundSpeed @ "MoveEndTime:" @ MoveEndTime);
}

function MoveEndTimeOut()
{
	// Cancel movement
	SetReachPreciseDestination(vect(0,0,0), TRUE);
	PawnOwner.GroundSpeed = PawnOwner.DefaultGroundSpeed;

	// This will trigger the automatic alignment of the Leader and Follower.
	bAlignPawns = TRUE;
}

/** Function called when the dueling mini game is ready to be started */
function DuelingMiniGameStart()
{
	// Let Leader know that we have now enterred the dueling stage!
	PawnOwner.DuelingMiniGameStartNotification();
	// Also let Follower know about it
	Follower.DuelingMiniGameStartNotification();

	if( PawnOwner.WorldInfo.NetMode != NM_Client )
	{
		// Auto lose quickly if dueling without Lancer
		if( Follower.Weapon != None && !Follower.Weapon.IsA( 'GearWeap_AssaultRifle' ) )
		{
			PawnOwner.SetTimer(  0.15f, FALSE, nameof( self .ServerLeaderEndOfChainsawDuelMiniGame),  self  );
		}
	}
}

function PlayDuelingStartAnimation(GearPawn APawn)
{
	// Play animation
	APawn.BS_Play(GetBS(), SpeedModifier, 0.2f/SpeedModifier, -1.f, FALSE, TRUE);

	if( APawn == PawnOwner )
	{
		// Set up a notify for this owner when animation is done playing.
		APawn.BS_SetAnimEndNotify(GetBS(), TRUE);
	}

	if( PlayerController(APawn.Controller) != None )
	{
		CameraAnimInstance = PlayRandomCameraAnim(APawn, CameraAnimStartList, SpeedModifier * 0.9f, 1.f, 0.2f/SpeedModifier, 0.f,,,,, TRUE);
	}

	// yell
	APawn.SoundGroup.PlayEffort(APawn, GearEffort_ChainsawDuel, true);
}

function StopDuelingAnimation(GearPawn APawn)
{
	APawn.BS_Stop( GetBS(), 0.2f );
	if( APawn == PawnOwner )
	{
		// turn off notify
		APawn.BS_SetAnimEndNotify(GetBS(), FALSE);
	}
}

/** Notification called when animation is done playing */
function BS_AnimEndNotify(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	if( PawnOwner.WorldInfo.NetMode != NM_Client )
	{
		ServerLeaderEndOfChainsawDuelMiniGame();
	}
}

/**
 * Called on Dueling Leader on the server when mini game ends,
 * to collect the scores, and decided on the fate of both players.
 */
function ServerLeaderEndOfChainsawDuelMiniGame()
{
	local INT						LeaderCount, FollowerCount, TieMargin;
	local EDifficultyLevel			DifficultyToUse;
	local GearPawn_LocustSkorgeBase	Skorge;

	// Dueling properly finished.
	bProperlyFinished = TRUE;

	Skorge = GearPawn_LocustSkorgeBase(PawnOwner);
	if( Skorge != None && Skorge.ForcedOutcome != 0 )
	{
		PawnOwner.ServerDoSpecialMove(SM_ChainSawAttack, TRUE, Follower, Skorge.ForcedOutcome);
		return;
	}

	// See who got the best score
	LeaderCount		= PawnOwner.DuelingMiniGameButtonPresses;
	FollowerCount	= Follower.DuelingMiniGameButtonPresses;

	DifficultyToUse = class'DifficultySettings'.static.GetLowestPlayerDifficultyLevel(PawnOwner.WorldInfo).default.DifficultyLevel;
	if (DifficultyToUse == DL_Casual)
	{
		TieMargin = 1;
	}
	// else TieMargin = 0;

	`log(PawnOwner.WorldInfo.TimeSeconds @ PawnOwner @ class @ GetFuncName() @ "LeaderCount:" @ LeaderCount @ "FollowerCount:" @ FollowerCount @ "TieMargin:" @ TieMargin);

	// We've got a Draw if score is within TieMargin
	if (LeaderCount - FollowerCount >= 0 && LeaderCount - FollowerCount <= TieMargin)
	{
		if (DifficultyToUse == DL_Insane)
		{
			// no ties on insane - skorge wins instead
			PawnOwner.ServerDoSpecialMove(SM_ChainSawAttack, TRUE, Follower, DUEL_WIN);
		}
		else
		{
			PawnOwner.ServerDoSpecialMove(SM_ChainSawAttack, TRUE, Follower, DUEL_DRAW);
		}
	}
	else if( LeaderCount > FollowerCount )
	{
		// Leader won
		PawnOwner.ServerDoSpecialMove(SM_ChainSawAttack, TRUE, Follower, DUEL_WIN);
	}
	else
	{
		// Follower won
		PawnOwner.ServerDoSpecialMove(SM_ChainSawAttack, TRUE, Follower, DUEL_LOSE);
	}
}

/** Notification when Follower is leaving his FollowerSpecialMove */
function OnFollowerLeavingSpecialMove()
{
	// On server, see if follower aborted move early, then we need to cancel dueling.
	if( PawnOwner.WorldInfo.NetMode != NM_Client && !bProperlyFinished && PawnOwner.IsGameplayRelevant() )
	{
		bProperlyFinished = TRUE;
		PawnOwner.ServerEndSpecialMove(PawnOwner.SpecialMove);
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	// On server, if leader aborted move early, then terminate follower as well.
	if( PawnOwner.WorldInfo.NetMode != NM_Client && !bProperlyFinished && Follower.IsGameplayRelevant() && Follower.IsDoingSpecialMove(FollowerSpecialMove) )
	{
		bProperlyFinished = TRUE;
		Follower.ServerEndSpecialMove(FollowerSpecialMove);
	}

	PawnOwner.PlaySound( SoundCue'Weapon_AssaultRifle.Dueling.ChainsawDuelingStop01Cue', TRUE );

	// Clear animations
	StopDuelingAnimation(PawnOwner);
	StopDuelingAnimation(Follower);

	// Clear timers
	PawnOwner.ClearTimer('StartClashEffect', Self);
	PawnOwner.ClearTimer('DuelingMiniGameStart', Self);
	PawnOwner.ClearTimer('ServerLeaderEndOfChainsawDuelMiniGame', Self);

	// Let our weapon know that the dueling mini game is over.
	PawnOwner.DuelingMiniGameEndNotification();
	Follower.DuelingMiniGameEndNotification();

	Super.SpecialMoveEnded(PrevMove, NextMove);
}

defaultproperties
{
	BS_SkorgeDuel_Staff=(AnimName[BS_FullBody]="Staff_Chainsaw_Duel_Start")
	BS_SkorgeDuel_2Stick=(AnimName[BS_FullBody]="2stick_Chainsaw_Duel_Start")
	BS_SkorgeDuel_1Stick=(AnimName[BS_FullBody]="1stick_Chainsaw_Duel_Start")
	MoveEndTime=1.25f

	CameraAnimStartList(0)=CameraAnim'COG_MarcusFenix.Camera_Anims.Staff_Chainsaw_Duel_Start_Cam_01'
	CameraAnimStartList(1)=CameraAnim'COG_MarcusFenix.Camera_Anims.Staff_Chainsaw_Duel_Start_Cam_02'

	DefaultAICommand=class'AICmd_Base_PushedBySpecialMove'
	FollowerSpecialMove=SM_ChainsawDuel_Follower
	AlignDistance=211.59f
}
