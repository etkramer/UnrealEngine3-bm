
/**
 * GSM_Wretch_GrabAttack
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Wretch_GrabAttack extends GSM_InteractionPawnLeader_Base;

/** Animations */
var	GearPawn.BodyStance	BS_Attacker, BS_Victim;
/** Offset of Marker from Pawn's location. Used to place Wretch to grab Victim properly. */
var vector				MarkerOffset, VectToVictim;
/** Handle Follower leaving his special move early */
var bool				bWantFollowerToLeaveHisSpecialMove;

/**
 * Test conditions to Attack Grab another Pawn
 */
function bool CanInteractWithPawn(GearPawn OtherPawn)
{
	if( PawnOwner.CoverType != CT_None )
	{
		return FALSE;
	}

	if( !OtherPawn.IsA('GearPawn_Infantry') 		// Can only attack infantry.
		|| PawnOwner.IsSameTeam(OtherPawn) 			// Can only attack enemies
		|| OtherPawn.Physics == PHYS_Falling 		// Victim cannot be falling.
		|| OtherPawn.Physics == PHYS_RigidBody 		// Victim cannot be a rag doll
		|| OtherPawn.bPlayedDeath 					// Victim cannot be already dead.
		|| OtherPawn.CarriedCrate != None			// Victim cannot be carrying a crate.
		|| OtherPawn.IsChainsawDueling() )			// Victim cannot be in a chainsaw duel
	{
		return FALSE;
	}

	// Wretch has to be somewhat facing his victim
	// Victim must not be facing wretch
	if( !PawnOwner.IsFacingOther(OtherPawn, 0.f) || OtherPawn.IsFacingOther(PawnOwner, 0.f) )
	{
		return FALSE;
	}

	return TRUE;
}


/** Notification called when Special Move starts */
function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	bWantFollowerToLeaveHisSpecialMove = FALSE;

	Super.SpecialMoveStarted(bForced,PrevMove);

	if( Follower.IsHumanControlled() )
	{
		Follower.TriggerEventClass( class'SeqEvt_WretchGrabbedPlayer', Follower );
	}	
}

/**
 * Begins the interaction.
 */
function StartInteraction()
{
	// Move kidnapper into position to grab hostage.
	MoveToMarkers();
}

/** Place both characters on markers, so they can connect and play their grab animation for attachment */
function MoveToMarkers()
{
	local Vector	LeaderDestination;
	local float		BlendInTime, AnimPlayRate;

	VectToVictim = Follower.Location - PawnOwner.Location;
	VectToVictim.Z = 0.f;
	VectToVictim = Normal(VectToVictim);

	LeaderDestination = Follower.Location + RelativeToWorldOffset(Rotator(-VectToVictim), MarkerOffset);

	BlendInTime = 0.2f;
	AnimPlayRate = 1.0f;

	// Start playing grab animation
	PlayGrab(AnimPlayRate, BlendInTime);

	SetReachPreciseDestination(LeaderDestination);
	SetFacePreciseRotation(Rotator(VectToVictim), BlendInTime);
	Follower.SpecialMoves[Follower.SpecialMove].SetFacePreciseRotation(Rotator(-VectToVictim), BlendInTime);
}

function PlayGrab(float PlayRate, float BlendInTime)
{
	// Play animation on both characters.
	PawnOwner.BS_Play(BS_Attacker, PlayRate, BlendInTime, -1.f);
	PawnOwner.BS_SetAnimEndNotify(BS_Attacker, TRUE);

	Follower.BS_Play(BS_Victim, PlayRate, BlendInTime, -1.f);
	Follower.RemoveAndSpawnAHelmet(Follower.Location - PawnOwner.Location, class'GearDamageType', FALSE );

	// Timer to trigger the grab, do this once we're fully blended in
	if( BlendInTime > 0.f )
	{
		PawnOwner.SetTimer( BlendInTime, FALSE, nameof(self.AttachWretchToVictim), self );
	}
	else
	{
		AttachWretchToVictim();
	}
}

function AttachWretchToVictim()
{
	local vector AttachLoc;

	// Cancel Leader movement, since Follower is now going to be attached.
	SetReachPreciseDestination(Vect(0,0,0), TRUE);
	PawnOwner.Acceleration = Vect(0,0,0);

	AttachLoc = PawnOwner.Location  - vect(0,0,1) * Follower.Mesh.Translation.Z + RelativeToWorldOffset(Rotator(VectToVictim), MarkerOffset);
	AttachFollowerToLeader('', AttachLoc, Rotator(-Vector(PawnOwner.Rotation)) );

	// Start minigame.
	ButtonMashingMiniGameStart();
}

function BS_AnimEndNotify(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
}

/** Function called when the dueling mini game is ready to be started */
function ButtonMashingMiniGameStart()
{
	local GearPC		GPC;
	local GearHUD_Base	MyGearHUD;

	// Start it on victim only. Don't care about wretchies
	Follower.TriggerMiniGameStartNotification();

	GPC = GearPC(Follower.Controller);
	if( GPC != None)
	{
		MyGearHUD = GearHUD_Base(GPC.myHUD);
		if( MyGearHUD != None )
		{
			MyGearHUD.SetActionInfo(AT_SpecialMove, Action, Follower.bIsMirrored);
		}
	}

	// Interaction Leader on the server is the one getting the timer to end the dueling mini game
	if( PawnOwner.WorldInfo.NetMode != NM_Client )
	{
		PawnOwner.SetTimer( 2.5f, FALSE, nameof(self.ServerLeaderEndOfButtonMashingMiniGame), self );
	}
}

function ButtonMashingMiniGameEnd()
{
	// End mini game.
	Follower.TriggerMiniGameEndNotification();
}

/**
 * Called on Leader on the server when mini game ends,
 * to collect the scores, and decided on the fate of both players.
 */
function ServerLeaderEndOfButtonMashingMiniGame()
{
	local float	Score;
	local bool	bPlayerWins;

	// properly finished.
	bWantFollowerToLeaveHisSpecialMove = TRUE;

	if (Follower.InGodMode() || !Follower.IsHumanControlled())
	{
		bPlayerWins = true;
	}
	else if (Follower.DuelingMiniGameButtonPresses > 0)
	{
		Score = 2.f / Follower.DuelingMiniGameButtonPresses;
		bPlayerWins = FRand() <= (1.f - FClamp(Score, 0.f, 1.f));
	}
	else
	{
		bPlayerWins = FALSE;
	}

	// Player gets rid of wretch
	if( bPlayerWins )
	{
		// Kill wretch!
		PawnOwner.TearOffMomentum = Normal(PawnOwner.Location - Follower.Location);
		PawnOwner.Died(Follower.Controller, class'GDT_Wretch_Melee', PawnOwner.Location);
	}
	// Player gets killed by wretch
	else
	{
		// Kill Player!
		KillPlayer();
	}
}

function KillPlayer()
{
	if( Follower != None && !Follower.bPlayedDeath )
	{
		Follower.TearOffMomentum = Normal(Follower.Location - PawnOwner.Location);
		Follower.Died(PawnOwner.Controller, class'GDT_Wretch_Melee', Follower.Location);
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	ButtonMashingMiniGameEnd();

	// Reset stuffs
	PawnOwner.BS_SetAnimEndNotify(BS_Attacker, FALSE);

	// Make sure timer is cleared
	PawnOwner.ClearTimer('AttachWretchToVictim', Self);

	PawnOwner.BS_Stop(BS_Attacker, 0.2f);
	Follower.BS_Stop(BS_Victim, 0.2f);

	// Take care of the hostage  if he's still around
	if( Follower != None && !Follower.bDeleteMe )
	{
		// If the follower has been attached, detach it.
		if( Follower.IsBasedOn(PawnOwner) )
		{
			// Detach Follower from Leader.
			DetachPawn(Follower);
		}

		// We're now expecting the victim to leave his special move
		bWantFollowerToLeaveHisSpecialMove = TRUE;

		// If Follower is still a victim, end his special move
		if( Follower.IsDoingSpecialMove(FollowerSpecialMove) )
		{
			Follower.EndSpecialMove(FollowerSpecialMove);
		}
	}

	Super.SpecialMoveEnded(PrevMove, NextMove);
}

/**
 * Victim is leaving his special move, catch cases where we didn't expect this from hapening
 * This could happen when the victim logs out, types 'suicice', or is killed by some other event.
 */
function OnFollowerLeavingSpecialMove()
{
	if( !bWantFollowerToLeaveHisSpecialMove )
	{
		bWantFollowerToLeaveHisSpecialMove = TRUE;

		// Handle this from the server.
		if( PawnOwner.Role == Role_Authority )
		{
			// Kill player
			KillPlayer();
			PawnOwner.ServerEndSpecialMove();
		}
	}
}

defaultproperties
{
	bShouldAbortWeaponReload=TRUE
	bCanFireWeapon=FALSE
	bDisableLook=FALSE

	bDisableMovement=TRUE
	bLockPawnRotation=TRUE

	DefaultAICommand=class'AICmd_Base_PushedBySpecialMove'
	bDisableAI=TRUE

	MarkerOffset=(X=17.32,Y=0.00,Z=0.00)
	FollowerSpecialMove=SM_CQC_Victim
	BS_Attacker=(AnimName[BS_FullBody]="Wretch_AttachPose")
	BS_Victim=(AnimName[BS_FullBody]="Marcus_Wretch_Jump")

	Action={(
		ActionName="WretchGrabAttack",
		IconAnimationSpeed=0.1f,
		ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.HUD.HUD_ActionIcons2',U=0,V=128,UL=83,VL=38),	// L/R mash
										  (Texture=Texture2D'Warfare_HUD.HUD.HUD_ActionIcons2',U=0,V=166,UL=83,VL=38))) )
	)}
}
