
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_ChainsawDuel_Leader extends GSM_InteractionPawnLeader_Base;

/** Animations */
var	GearPawn.BodyStance				BS_Engage_Leader, BS_Engage_Follower;
/** Camera animations. */
//var	Array<GearPC.CameraBoneAnimation>	CA_Engage, CA_Dueling;
/** TRUE if dueling properly finished. FALSE if it was aborted too early */
var bool							bProperlyFinished;
/** Maximum Height difference authored in the aim offse nodes */
var float							MaxHeightDifference;
/** Dueling effects */
var ParticleSystemComponent			ChainSawDuelClashEffect;
var PointLightComponent				ChainSawDuelClashLightComponent;
/** Plays while the dual is going**/
var AudioComponent                  AC_LoopingDuelSound;

/** Camera Animations */
var	CameraAnim						CameraAnimDuelStart, CameraAnimDuelLoop;
var transient CameraAnimInst		CameraAnimInstLeader, CameraAnimInstFollower;

function bool CanInteractWithPawn(GearPawn OtherPawn)
{
	local float ChainsawDuelFacingDotAngle;
	local vector LookDir;
	local vector PlayerDir;

	/*
	if (!OtherPawn.IsDoingSpecialMove(SM_ChainsawHold))
	{
		return FALSE;
	}
	*/

	// For Testing
	//return TRUE;

	if( OtherPawn.IsReloadingWeapon() )
	{
		return FALSE;
	}

	// ignore pawns doing dodge moves. They can escape executions.
	if( OtherPawn.IsDoingADodgeMove() )
	{
		return FALSE;
	}

	// Cannot duel someone who's involved in a chainsaw attack. Resort to a standard chainsaw attack.
	// Hostages and Kidnappers cannot Duel.
	if( OtherPawn.IsChainsawDueling() || OtherPawn.IsDoingSpecialMeleeAttack() || OtherPawn.IsAHostage() || OtherPawn.IsAKidnapper() )
	{
		//`LogSMExt(PawnOwner, " Failed SpecialMove checks, already involved in a chainsaw attack" @ OtherPawn.SpecialMove);
		return FALSE;
	}

    // no dueling teammates or chainsaw immune victims
	if (!PawnOwner.TargetIsValidMeleeTarget(OtherPawn,FALSE) || !OtherPawn.CanBeSpecialMeleeAttacked(PawnOwner))
	{
		//`LogSMExt(PawnOwner, " Failed TargetIsValidMeleeTarget or CanBeSpecialMeleeAttacked");
		return FALSE;
	}

	// no chainsaw dueling if same team, even with friendly fire
	if (PawnOwner.GetTeamNum() == OtherPawn.GetTeamNum())
	{
		return FALSE;
	}

	// must be holding a chainsaw
	if (GearWeap_AssaultRifle(OtherPawn.Weapon) == None)
	{
		//`LogSMExt(PawnOwner, " Failed holding a chainsaw" @ OtherPawn.Weapon);
		return FALSE;
	}

	// The other player has to be able to engage melee to be drawn into a duel
	// This checks for melee interrupt.
	// This exception is ChainSaw Hold, that's a valid way to be drawn into a duel.
	if( !OtherPawn.IsDoingSpecialMove(SM_ChainsawHold) && !OtherPawn.CanEngageMelee() )
	{
		return FALSE;
	}

	// Shields prevent attacks (but not meatshields)
	if ( OtherPawn.IsProtectedByShield(Normal(OtherPawn.Location - PawnOwner.Location),TRUE) )
	{
		return FALSE;
	}

	// Players facing each other test. Turn angle in degres to the result of a dot product.
	ChainsawDuelFacingDotAngle = 0.6f;

	// Test if we're facing the Other Pawn
	LookDir		= Vector(PawnOwner.GetBaseAimRotation());
	LookDir.Z	= 0.f;
	PlayerDir	= OtherPawn.Location - PawnOwner.Location;
	PlayerDir.Z = 0.f;
	if( (Normal(PlayerDir) dot Normal(LookDir)) < ChainsawDuelFacingDotAngle )
	{
		//`LogSMExt(PawnOwner, " Failed ChainsawDuelFacingDotAngle #1 check." @ (Normal(PlayerDir) dot Normal(LookDir)) @ ChainsawDuelFacingDotAngle );
		return FALSE;
	}

	// Test if the Other Pawn is facing us
	LookDir		= Vector(OtherPawn.GetBaseAimRotation());
	LookDir.Z	= 0.f;
	PlayerDir	= PawnOwner.Location - OtherPawn.Location;
	PlayerDir.Z = 0.f;
	if( (Normal(PlayerDir) dot Normal(LookDir)) < ChainsawDuelFacingDotAngle )
	{
		//`LogSMExt(PawnOwner, " Failed ChainsawDuelFacingDotAngle #2 check." @ (Normal(PlayerDir) dot Normal(LookDir)) );
		return FALSE;
	}
	//`LogSMExt(PawnOwner, " Duel is ON!");

	// Duel is on!
	return TRUE;
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced, PrevMove);

	// Reset flag for new Duel.
	bProperlyFinished = FALSE;

	if (PCOwner != None && PCOwner.MyGearHUD != None)
	{
		PCOwner.MyGearHUD.SetActionInfo(AT_SpecialMove, Action, PawnOwner.bIsMirrored);
	}
}

function StartInteraction()
{
	// Sync looping animations.
	if( Follower.AnimTreeRootNode != None && PawnOwner.AnimTreeRootNode != None )
	{
		Follower.AnimTreeRootNode.ForceGroupRelativePosition('Sync_Idle', PawnOwner.AnimTreeRootNode.GetGroupRelativePosition('Sync_Idle'));
	}

	// Play dueling animation on both characters
	PlayDuelingStartAnimation(PawnOwner, BS_Engage_Leader);
	PlayDuelingStartAnimation(Follower, BS_Engage_Follower);

	PawnOwner.SetTimer( 1.33f, FALSE, nameof(self.DuelingMiniGameStart), self );

	// Position Pawns
	MoveToMarkers();
}

/** Align Pawns with each other */
function MoveToMarkers()
{
	// This will trigger the automatic alignment of the Leader and Follower.
	bAlignPawns = TRUE;
}

function Tick(float DeltaTime)
{
	local float HeightDifferencePct;

	// Adjust height difference for position aimoffset nodes.
	if( PawnOwner != None && Follower != None )
	{
		HeightDifferencePct = (Follower.Location.Z - PawnOwner.Location.Z) / (2.f * MaxHeightDifference);
		PawnOwner.PositionAdjustAimOffsetPct = vect2d(0.f, HeightDifferencePct);
		Follower.PositionAdjustAimOffsetPct = vect2d(0.f, -HeightDifferencePct);
	}

	Super.Tick(DeltaTime);
/*
	// DEBUGGING
	if( PawnOwner != None )
	{
		PawnOwner.DrawDebugCoordinateSystem(PawnOwner.Location, PawnOwner.Rotation, 25.f, FALSE);
		PawnOwner.DrawDebugSphere(PawnOwner.Location + Vector(PawnOwner.Rotation) * AlignDistance, 8.f, 8, 255, 0, 0, FALSE);
	}

	if( Follower != None )
	{
		Follower.DrawDebugCoordinateSystem(Follower.Location, Follower.Rotation, 25.f, FALSE);
		Follower.DrawDebugSphere(Follower.Location + Vector(Follower.Rotation) * AlignDistance, 8.f, 8, 255, 0, 0, FALSE);
	}
*/
}

/*
function DrawHUD(HUD H)
{
	local Canvas	Canvas;
	local String	Text;
	local FLOAT		XL, YL, Distance, HeightDifference;

	Canvas = H.Canvas;

	HeightDifference = (Follower.Location.Z - PawnOwner.Location.Z) / 2.f;
	Distance = VSize2D(Follower.Location - PawnOwner.Location);

	Text = "Distance:" @ Distance @ "HeightDifference:" @ HeightDifference @ "HeightDifferencePct:" @ (HeightDifference / MaxHeightDifference);
	Canvas.StrLen(Text, XL, YL);
	Canvas.SetPos((H.SizeX - XL)/2.f, H.SizeY/2.f + YL * 2.f);
	Canvas.SetDrawColor(255,0,0);
	Canvas.DrawText(TEXT, FALSE);
}
*/

function StartClashEffect()
{
	if( ChainSawDuelClashEffect != None )
	{
		PawnOwner.Mesh.AttachComponent(ChainSawDuelClashEffect, 'b_MF_IK_Hand_Root', vect(0,0,0), rot(0,0,0));
		ChainSawDuelClashEffect.SetHidden( FALSE );
		ChainSawDuelClashEffect.ActivateSystem();
	}

	PawnOwner.PlaySound( SoundCue'Weapon_AssaultRifle.Dueling.ChainsawDuelingStart01Cue', FALSE );

	if( ChainSawDuelClashLightComponent != None )
	{
		PawnOwner.Mesh.AttachComponent(ChainSawDuelClashLightComponent, 'b_MF_IK_Hand_Root', vect(0,0,0), rot(0,0,0));
		ChainSawDuelClashLightComponent.SetEnabled(TRUE);
	}

	if( AC_LoopingDuelSound == none )
	{
		AC_LoopingDuelSound = PawnOwner.CreateAudioComponent( SoundCue'Weapon_AssaultRifle.Dueling.ChainsawDuelingLoop01Cue', FALSE, TRUE, TRUE );
	}

	if( AC_LoopingDuelSound != none )
	{
		AC_LoopingDuelSound.FadeIn( 0.1f, 1.0f );
	}
}


function PlayDuelingStartAnimation(GearPawn APawn, BodyStance InBodyStance)
{
// 	local GearPC	AGearPC;
	local float		EngageTime;

	// Play Animation
	EngageTime = APawn.BS_Play(InBodyStance, SpeedModifier, 0.2f/SpeedModifier, 0.2f, FALSE, TRUE);

	// If this is the leader...
	if( APawn == PawnOwner )
	{
		// Set timer to start clash effect
		PawnOwner.SetTimer( FMax(EngageTime-1.f, 0.1f), FALSE, nameof(self.StartClashEffect), self );

		// Set animation end notify
		APawn.BS_SetAnimEndNotify(InBodyStance, TRUE);
	}

	PlayCameraAnim(APawn, CameraAnimDuelStart);

	// yell
	APawn.SoundGroup.PlayEffort(APawn, GearEffort_ChainsawDuel, true);
}

/** Override to catch camera instance */
function CameraAnimInst PlayCameraAnim
(
	GearPawn		PawnToPlay,
	CameraAnim		InCameraAnim,
	optional float	Rate=1.f,
	optional float	Scale=1.f,
	optional float	BlendInTime,
	optional float	BlendOutTime,
	optional bool	bLoop,
	optional bool	bRandomStartTime,
	optional float	Duration,
	optional bool	bSingleInstance
)
{
	if( PawnToPlay == PawnOwner )
	{
		CameraAnimInstLeader = Super.PlayCameraAnim(PawnToPlay, InCameraAnim, Rate, Scale, BlendInTime, BlendOutTime, bLoop, bRandomStartTime, Duration, bSingleInstance);
		return CameraAnimInstLeader;
	}

	CameraAnimInstFollower = Super.PlayCameraAnim(PawnToPlay, InCameraAnim, Rate, Scale, BlendInTime, BlendOutTime, bLoop, bRandomStartTime, Duration, bSingleInstance);
	return CameraAnimInstFollower;
}

/** End of engage animation */
function BS_AnimEndNotify(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	if( CameraAnimInstLeader != None )
	{
		CameraAnimInstLeader.Stop(TRUE);
		CameraAnimInstLeader = None;
	}

	if( CameraAnimInstFollower != None )
	{
		CameraAnimInstFollower.Stop(TRUE);
		CameraAnimInstFollower = None;
	}

	// Play camera looping animation
	PlayCameraAnim(PawnOwner, CameraAnimDuelLoop,,,,, TRUE);
	PlayCameraAnim(Follower, CameraAnimDuelLoop,,,,, TRUE);
}

/** Function called when the dueling mini game is ready to be started */
function DuelingMiniGameStart()
{
	// Let Leader know that we have now enterred the dueling stage!
	PawnOwner.DuelingMiniGameStartNotification();
	// Let also know Follower at the same time
	Follower.DuelingMiniGameStartNotification();

	// Interaction Leader on the server is the one getting the timer to end the dueling mini game
	if( PawnOwner.WorldInfo.NetMode != NM_Client )
	{
		`log(PawnOwner.WorldInfo.TimeSeconds @ PawnOwner @ class @ GetFuncName() @ "Setting timer" @ PawnOwner.ChainsawDuelMiniGameDuration);
		PawnOwner.SetTimer( PawnOwner.ChainsawDuelMiniGameDuration, FALSE, nameof(self.ServerLeaderEndOfChainsawDuelMiniGame), self );
	}
}

/**
 * Called on Dueling Leader on the server when mini game ends,
 * to collect the scores, and decided on the fate of both players.
 */
function ServerLeaderEndOfChainsawDuelMiniGame()
{
	local bool bIsCoopOrHorde, bLeaderWins, bLeaderAdvantage;
	local float LeaderPct;

	// Dueling properly finished.
	bProperlyFinished = TRUE;

	if (Follower.SpecialMove == SM_ChainsawDuel_Follower && GSM_ChainsawDuel_Follower(Follower.SpecialMoves[Follower.SpecialMove]).PrevSpecialMove != SM_ChainsawHold)
	{
		bLeaderAdvantage = TRUE;
	}

	// clamp the presses to prevent hax
	if (Follower.DuelingMiniGameButtonPresses > 25)
	{
		Follower.DuelingMiniGameButtonPresses = 0;
	}
	if (PawnOwner.DuelingMiniGameButtonPresses > 25)
	{
		PawnOwner.DuelingMiniGameButtonPresses = 0;
	}

	//`log("chainsaw duel resolution"@PawnOwner@Follower);

	bIsCoopOrHorde = (PawnOwner.WorldInfo.Game.IsA('GearGameSP') || PawnOwner.WorldInfo.Game.IsA('GearGameHorde'));
	// always a small chance of a draw
	if( FRand() > 0.95f )
	{
    	// @STATS - Track the tie
		GearPRI(Follower.PlayerReplicationInfo).AggregateChainSawDuel(false,true);
		GearPRI(PawnOwner.PlayerReplicationInfo).AggregateChainSawDuel(false,true);

		Follower.ServerDoSpecialMove(SM_ChainsawDuel_Draw, TRUE, PawnOwner);
		PawnOwner.ServerDoSpecialMove(SM_ChainsawDuel_Draw, TRUE, Follower);
	}
	else
	{
		/* useful for debugging
		PawnOwner.DuelingMiniGameButtonPresses = RandRange(8,12);
		Follower.DuelingMiniGameButtonPresses = RandRange(8,12);
		*/
		// if in co-op or Horde, cog always win, because losing the game and having to reload due to random chance is not fun
		if (bIsCoopOrHorde && PawnOwner.GetTeamNum() == 0 && PawnOwner.DuelingMiniGameButtonPresses > 0)
		{
			bLeaderWins = true;
		}
		else if (bIsCoopOrHorde && Follower.GetTeamNum() == 0 && !Follower.IsHumanControlled())
		{
			bLeaderWins = false;
		}
		// if neither pushed then rand
		else if (PawnOwner.DuelingMiniGameButtonPresses == 0 && Follower.DuelingMiniGameButtonPresses == 0)
		{
			bLeaderWins = FRand() > 0.5f;
		}
		// if one didn't push then pick the one who did
		else if (PawnOwner.DuelingMiniGameButtonPresses == 0 || Follower.DuelingMiniGameButtonPresses == 0)
		{
			bLeaderWins = PawnOwner.DuelingMiniGameButtonPresses > Follower.DuelingMiniGameButtonPresses;
		}
		else
		{
			// initial chance based on ratio of button presses
			LeaderPct = PawnOwner.DuelingMiniGameButtonPresses/float(PawnOwner.DuelingMiniGameButtonPresses + Follower.DuelingMiniGameButtonPresses);
			`log("initial pct:"@LeaderPct@PawnOwner.DuelingMiniGameButtonPresses@(1.f-LeaderPct)@Follower.DuelingMiniGameButtonPresses);
			// increase the delta in favor of whoever has more presses
			LeaderPct += (LeaderPct - (1.f - LeaderPct)) * 3.f;
			`log("delta skew:"@LeaderPct);
			if (bLeaderAdvantage)
			{
				LeaderPct += 0.25f;
			}
			`log("advantage:"@LeaderPct);
			// otherwise roll against the leader's share of button presses
			bLeaderWins = FRand() <= FMin(1.f,LeaderPct);
		}
		if (bLeaderWins)
		{
			GearPRI(Follower.PlayerReplicationInfo).AggregateChainSawDuel(false,false);
			GearPRI(PawnOwner.PlayerReplicationInfo).AggregateChainSawDuel(true,false);

			// Leader won
			PawnOwner.ServerDoSpecialMove(SM_ChainsawAttack, TRUE, Follower, class'GSM_ChainsawAttack'.static.PackChainsawAttackFlags(TRUE, TRUE));
		}
		else
		{
			GearPRI(Follower.PlayerReplicationInfo).AggregateChainSawDuel(true,false);
			GearPRI(PawnOwner.PlayerReplicationInfo).AggregateChainSawDuel(false,false);

			// Follower won
			Follower.ServerDoSpecialMove(SM_ChainsawAttack, TRUE, PawnOwner, class'GSM_ChainsawAttack'.static.PackChainsawAttackFlags(TRUE, FALSE));
		}
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

	if( ChainSawDuelClashEffect != None )
	{
		ChainSawDuelClashEffect.DeactivateSystem();
		if( PawnOwner.Mesh.IsComponentAttached(ChainSawDuelClashEffect) )
		{
			PawnOwner.Mesh.DetachComponent(ChainSawDuelClashEffect);
		}
	}
	if( ChainSawDuelClashLightComponent != None )
	{
		ChainSawDuelClashLightComponent.SetEnabled(FALSE);
		if( PawnOwner.Mesh.IsComponentAttached(ChainSawDuelClashLightComponent) )
		{
			PawnOwner.Mesh.DetachComponent(ChainSawDuelClashLightComponent);
		}
	}

	if( AC_LoopingDuelSound != none )
	{
		AC_LoopingDuelSound.FadeOut( 0.1f, 0.0f );
		AC_LoopingDuelSound = None;
	}

	PawnOwner.PlaySound( SoundCue'Weapon_AssaultRifle.Dueling.ChainsawDuelingStop01Cue', TRUE );

	// Clear timers
	PawnOwner.ClearTimer('StartClashEffect', Self);
	PawnOwner.ClearTimer('DuelingMiniGameStart', Self);
	PawnOwner.ClearTimer('ServerLeaderEndOfChainsawDuelMiniGame', Self);

	// Let our weapon know that the dueling mini game is over.
	PawnOwner.DuelingMiniGameEndNotification();
	Follower.DuelingMiniGameEndNotification();

	if( CameraAnimInstLeader != None )
	{
		CameraAnimInstLeader.Stop(TRUE);
		CameraAnimInstLeader = None;
	}

	if( CameraAnimInstFollower != None )
	{
		CameraAnimInstFollower.Stop(TRUE);
		CameraAnimInstFollower = None;
	}

	Super.SpecialMoveEnded(PrevMove, NextMove);
}

defaultproperties
{
	FollowerSpecialMove=SM_ChainsawDuel_Follower
	AlignDistance=138.65f
	MaxHeightDifference=67.97f

	BS_Engage_Leader=(AnimName[BS_FullBody]="Duel_Start_Master")
	BS_Engage_Follower=(AnimName[BS_FullBody]="Duel_Start_Slave")

	CameraAnimDuelStart=CameraAnim'COG_MarcusFenix.Camera_Anims.Duel_Start_Cam01'
	CameraAnimDuelLoop=CameraAnim'COG_MarcusFenix.Camera_Anims.Duel_Loop_Cam01'

	Begin Object Class=ParticleSystemComponent Name=ChainSawDuelClashEffect0
	    bAutoActivate=FALSE
		Template=ParticleSystem'Effects_Gameplay.Chainsaw.P_Chainsaw_Duel_Clash_Looping'
		TickGroup=TG_PostUpdateWork
	End Object
	ChainSawDuelClashEffect=ChainSawDuelClashEffect0

	Begin Object Class=LightFunction Name=ChainSawDuelClashLightFunction0
		SourceMaterial=Material'Effects_Gameplay.Materials.M_Chainsaw_Duel_Lightfunction'
    End Object

    Begin Object Class=PointLightComponent Name=ChainSawDuelClashLightComponent0
		bEnabled=FALSE
		LightColor=(R=252,G=207,B=148,A=255)
		Brightness=7
		Radius=200
		Function=ChainSawDuelClashLightFunction0
    End Object
    ChainSawDuelClashLightComponent=ChainSawDuelClashLightComponent0

	Action={(
		ActionName=SawDuel,
		IconAnimationSpeed=0.1f,
		ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.HUD.HUD_ActionIcons2',U=83,V=0,UL=45,VL=32),	// mash B button
										  (Texture=Texture2D'Warfare_HUD.HUD.HUD_ActionIcons2',U=83,V=33,UL=45,VL=32))),
							(ActionIcons=((Texture=Texture2D'Warfare_HUD.HUD.HUD_ActionIcons2',U=0,V=0,UL=83,VL=66),	// saw
										  (Texture=Texture2D'Warfare_HUD.HUD.HUD_ActionIcons2',U=0,V=68,UL=83,VL=66))) ),
		)}

	bDisablePOIs=TRUE

//Chainsaw Dueling:

//Weapon_AssaultRifle.Dueling.ChainsawDuelingStart01Cue
//Initial impact of two chainsaws colliding

//Weapon_AssaultRifle.Dueling.ChainsawDuelingLoop01Cue
//Looping grinding\ripping sound of two dueling chainsaws, start with start and stop with stop

//Weapon_AssaultRifle.Dueling.ChainsawDuelingStop01Cue
//Two dueling chainsaws release from each other
//AC_LoopingDuelSound


}
