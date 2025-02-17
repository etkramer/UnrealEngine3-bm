
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Skorge_Duel_Outcome extends GSM_InteractionPawnLeader_Base;

var bool				bProperlyFinished;
var bool				bWon, bLost, bDraw;
var GearPawn.BodyStance	BS_SkorgeDuel_Win_Staff, BS_SkorgeDuel_Lose_Staff, BS_SkorgeDuel_Draw_Staff;
var GearPawn.BodyStance	BS_SkorgeDuel_Win_TwoStick, BS_SkorgeDuel_Lose_TwoStick, BS_SkorgeDuel_Draw_TwoStick;
var GearPawn.BodyStance	BS_SkorgeDuel_Win_OneStick, BS_SkorgeDuel_Lose_OneStick, BS_SkorgeDuel_Draw_OneStick;

var Array<CameraAnim>	CameraAnimWinList, CameraAnimDrawList, CameraAnimLoseList;

/** Unpack Special Move flags used for replication */
function UnpackSpecialMoveFlags()
{
	local GearAI_Skorge AI;

	// Player will auto lose if dueling without Lancer
	if( Follower != None && Follower.Weapon != None && !Follower.Weapon.IsA( 'GearWeap_AssaultRifle' ) )
	{
		bWon  = TRUE;
		bLost = FALSE;
		bDraw = FALSE;
	}
	else
	{
		bWon	= bool(PawnOwner.SpecialMoveFlags & class'GSM_Skorge_Duel_Leader'.const.DUEL_WIN);
		bLost	= bool(PawnOwner.SpecialMoveFlags & class'GSM_Skorge_Duel_Leader'.const.DUEL_LOSE);
		bDraw	= bool(PawnOwner.SpecialMoveFlags & class'GSM_Skorge_Duel_Leader'.const.DUEL_DRAW);
	}

	// Don't let player lose 
	AI = GearAI_Skorge(PawnOwner.Controller);
	if( AI != None && bWon && SeqAct_Skorge_ChargeAndDuel(AI.CurrentAttackSeq) != None && 
		SeqAct_Skorge_ChargeAndDuel(AI.CurrentAttackSeq).bForceDraw )
	{
		bWon  = FALSE;
		bLost = FALSE;
		bDraw = TRUE;
	}
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	bProperlyFinished = FALSE;
	Super.SpecialMoveStarted(bForced, PrevMove);
}


/** StartInteraction */
function StartInteraction()
{
	UnpackSpecialMoveFlags();
	PlayOutcomeAnimation();
}

function ClearPreviousCamera()
{
	local GSM_Skorge_Duel_Leader	SkorgeDuelSM;
	local GearPC	PCToPlay;

	SkorgeDuelSM = GSM_Skorge_Duel_Leader(PawnOwner.SpecialMoves[SM_ChainsawDuel_Leader]);
	if( SkorgeDuelSM.CameraAnimInstance != None )
	{
		SkorgeDuelSM.CameraAnimInstance.Stop(TRUE);
		SkorgeDuelSM.CameraAnimInstance = None;

		// Force a camera update, so CameraAnim can be fitted from default camera position, and not on top of previous camera anim.
		PCToPlay = GearPC(Follower.Controller);
		PCToPlay.PlayerCamera.UpdateCamera(0.f);
	}
}

function PlayOutcomeAnimation()
{
	local GearPawn_LocustSkorgeBase Skorge;

	ClearPreviousCamera();

	if( bWon )	// Skorge wins, marcus lose
	{
		PlayRandomCameraAnim(Follower, CameraAnimLoseList, SpeedModifier, 1.f, 0.f, 0.2f,,,,, TRUE);
	}
	else if( bLost ) // skorge lose, marcus wins
	{
		PlayRandomCameraAnim(Follower, CameraAnimWinList, SpeedModifier, 1.f, 0.f, 0.2f,,,,, TRUE);
	}
	else
	{
		PlayRandomCameraAnim(Follower, CameraAnimDrawList, SpeedModifier, 1.f, 0.f, 0.2f,,,,, TRUE);
	}

	Skorge = GearPawn_LocustSkorgeBase(PawnOwner);
	if( Skorge.Stage == SKORGE_Staff )
	{
		if( bWon )
		{
			PawnOwner.BS_Play(BS_SkorgeDuel_Win_Staff, SpeedModifier, 0.f, 0.2f);
			PawnOwner.BS_SetRootBoneAxisOptions(BS_SkorgeDuel_Win_Staff, RBA_Translate, RBA_Translate, RBA_Translate);
			PawnOwner.BS_SetAnimEndNotify(BS_SkorgeDuel_Win_Staff, TRUE);

			Follower.BS_Play(BS_SkorgeDuel_Lose_Staff, SpeedModifier, 0.f, 0.2f);
			Follower.BS_SetRootBoneAxisOptions(BS_SkorgeDuel_Lose_Staff, RBA_Translate, RBA_Translate, RBA_Translate);
		}
		else if( bLost )
		{
			PawnOwner.BS_Play(BS_SkorgeDuel_Lose_Staff, SpeedModifier, 0.f, 0.2f);
			PawnOwner.BS_SetRootBoneAxisOptions(BS_SkorgeDuel_Lose_Staff, RBA_Translate, RBA_Translate, RBA_Translate);
			PawnOwner.BS_SetAnimEndNotify(BS_SkorgeDuel_Lose_Staff, TRUE);

			Follower.BS_Play(BS_SkorgeDuel_Win_Staff, SpeedModifier, 0.f, 0.2f);
			Follower.BS_SetRootBoneAxisOptions(BS_SkorgeDuel_Win_Staff, RBA_Translate, RBA_Translate, RBA_Translate);

			Skorge.SetTimer( 1.f, FALSE, nameof(Skorge.RegisterStageChange));
		}
		else
		{
			PawnOwner.BS_Play(BS_SkorgeDuel_Draw_Staff, SpeedModifier, 0.f, 0.2f);
			PawnOwner.BS_SetRootBoneAxisOptions(BS_SkorgeDuel_Draw_Staff, RBA_Translate, RBA_Translate, RBA_Translate);
			PawnOwner.BS_SetAnimEndNotify(BS_SkorgeDuel_Draw_Staff, TRUE);

			Follower.BS_Play(BS_SkorgeDuel_Draw_Staff, SpeedModifier, 0.f, 0.2f);
			Follower.BS_SetRootBoneAxisOptions(BS_SkorgeDuel_Draw_Staff, RBA_Translate, RBA_Translate, RBA_Translate);
		}
	}
	else
	if( Skorge.Stage == SKORGE_TwoStick )
	{
		if( bWon )
		{
			PawnOwner.BS_Play(BS_SkorgeDuel_Win_TwoStick, SpeedModifier, 0.f, 0.2f);
			PawnOwner.BS_SetRootBoneAxisOptions(BS_SkorgeDuel_Win_TwoStick, RBA_Translate, RBA_Translate, RBA_Translate);
			PawnOwner.BS_SetAnimEndNotify(BS_SkorgeDuel_Win_TwoStick, TRUE);

			Follower.BS_Play(BS_SkorgeDuel_Lose_TwoStick, SpeedModifier, 0.f, 0.2f);
			Follower.BS_SetRootBoneAxisOptions(BS_SkorgeDuel_Lose_TwoStick, RBA_Translate, RBA_Translate, RBA_Translate);
		}
		else if( bLost )
		{
			PawnOwner.BS_Play(BS_SkorgeDuel_Lose_TwoStick, SpeedModifier, 0.f, 0.2f);
			PawnOwner.BS_SetRootBoneAxisOptions(BS_SkorgeDuel_Lose_TwoStick, RBA_Translate, RBA_Translate, RBA_Translate);
			PawnOwner.BS_SetAnimEndNotify(BS_SkorgeDuel_Lose_TwoStick, TRUE);

			Follower.BS_Play(BS_SkorgeDuel_Win_TwoStick, SpeedModifier, 0.f, 0.2f);
			Follower.BS_SetRootBoneAxisOptions(BS_SkorgeDuel_Win_TwoStick, RBA_Translate, RBA_Translate, RBA_Translate);

			Skorge.SetTimer( 1.f, FALSE, nameof(Skorge.RegisterStageChange));
		}
		else
		{
			PawnOwner.BS_Play(BS_SkorgeDuel_Draw_TwoStick, SpeedModifier, 0.f, 0.2f);
			PawnOwner.BS_SetRootBoneAxisOptions(BS_SkorgeDuel_Draw_TwoStick, RBA_Translate, RBA_Translate, RBA_Translate);
			PawnOwner.BS_SetAnimEndNotify(BS_SkorgeDuel_Draw_TwoStick, TRUE);

			Follower.BS_Play(BS_SkorgeDuel_Draw_TwoStick, SpeedModifier, 0.f, 0.2f);
			Follower.BS_SetRootBoneAxisOptions(BS_SkorgeDuel_Draw_TwoStick, RBA_Translate, RBA_Translate, RBA_Translate);
		}
	}
	else
	{
		if( bWon )
		{
			PawnOwner.BS_Play(BS_SkorgeDuel_Win_OneStick, SpeedModifier, 0.f, 0.2f);
			PawnOwner.BS_SetRootBoneAxisOptions(BS_SkorgeDuel_Win_OneStick, RBA_Translate, RBA_Translate, RBA_Translate);
			PawnOwner.BS_SetAnimEndNotify(BS_SkorgeDuel_Win_OneStick, TRUE);

			Follower.BS_Play(BS_SkorgeDuel_Lose_OneStick, SpeedModifier, 0.f, 0.2f);
			Follower.BS_SetRootBoneAxisOptions(BS_SkorgeDuel_Lose_OneStick, RBA_Translate, RBA_Translate, RBA_Translate);
		}
		else if( bLost )
		{
			PawnOwner.BS_Play(BS_SkorgeDuel_Lose_OneStick, SpeedModifier, 0.f, 0.2f);
			PawnOwner.BS_SetRootBoneAxisOptions(BS_SkorgeDuel_Lose_OneStick, RBA_Translate, RBA_Translate, RBA_Translate);
			PawnOwner.BS_SetAnimEndNotify(BS_SkorgeDuel_Lose_OneStick, TRUE);

			Follower.BS_Play(BS_SkorgeDuel_Win_OneStick, SpeedModifier, 0.f, 0.2f);
			Follower.BS_SetRootBoneAxisOptions(BS_SkorgeDuel_Win_OneStick, RBA_Translate, RBA_Translate, RBA_Translate);
		}
		else
		{
			PawnOwner.BS_Play(BS_SkorgeDuel_Draw_OneStick, SpeedModifier, 0.f, 0.2f);
			PawnOwner.BS_SetRootBoneAxisOptions(BS_SkorgeDuel_Draw_OneStick, RBA_Translate, RBA_Translate, RBA_Translate);
			PawnOwner.BS_SetAnimEndNotify(BS_SkorgeDuel_Draw_OneStick, TRUE);

			Follower.BS_Play(BS_SkorgeDuel_Draw_OneStick, SpeedModifier, 0.f, 0.2f);
			Follower.BS_SetRootBoneAxisOptions(BS_SkorgeDuel_Draw_OneStick, RBA_Translate, RBA_Translate, RBA_Translate);
		}
	}

	PawnOwner.Mesh.RootMotionMode = RMM_Accel;
	Follower.Mesh.RootMotionMode = RMM_Accel;
}

/** Notification called when animation is done playing */
function BS_AnimEndNotify(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	bProperlyFinished = TRUE;

	// End follower first, because when we end our special move, we lose the reference to it.
	if( PawnOwner.WorldInfo.NetMode != NM_Client )
	{
		Follower.ServerEndSpecialMove(FollowerSpecialMove);
		PawnOwner.ServerEndSpecialMove(PawnOwner.SpecialMove);
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
	local int LatentIdx;
	local SeqAct_Skorge_ChargeAndDuel SeqAct;
	local GearAI_Skorge AI;
	local GearPawn_LocustSkorgeBase	SkorgePawn;

	// Tell Skorge AI about the result and finish the latent kismet action
	AI = GearAI_Skorge(PawnOwner.Controller);
	if( AI != None )
	{
		SeqAct = SeqAct_Skorge_ChargeAndDuel(AI.CurrentAttackSeq);
		if( SeqAct != None )
		{
			if( bLost )		 { SeqAct.DuelResult = 0; }
			else if( bWon )  { SeqAct.DuelResult = 1; }
			else			 { SeqAct.DuelResult = 2; }
			
			LatentIdx = AI.LatentActions.Find( SeqAct );
			if( LatentIdx >= 0 )
			{
				AI.LatentActions.Remove( LatentIdx, 1 );
			}
		}
	}

	SkorgePawn = GearPawn_LocustSkorgeBase(PawnOwner);
	if( SkorgePawn != None )
	{
		/** Be safe, force chainsaw sounds to stop after duel is finished */
		SkorgePawn.ForceStopChainsaw();
	}

	// On server, if leader aborted move early, then terminate follower as well.
	if( PawnOwner.WorldInfo.NetMode != NM_Client && !bProperlyFinished && Follower.IsGameplayRelevant() && Follower.IsDoingSpecialMove(FollowerSpecialMove) )
	{
		bProperlyFinished = TRUE;
		Follower.ServerEndSpecialMove(FollowerSpecialMove);
	}

	// Stop animations.
	PawnOwner.BS_Stop(BS_SkorgeDuel_Win_Staff, 0.2f);
	PawnOwner.BS_Stop(BS_SkorgeDuel_Lose_Staff, 0.2f);
	PawnOwner.BS_Stop(BS_SkorgeDuel_Draw_Staff, 0.2f);
	PawnOwner.BS_Stop(BS_SkorgeDuel_Win_TwoStick, 0.2f);
	PawnOwner.BS_Stop(BS_SkorgeDuel_Lose_TwoStick, 0.2f);
	PawnOwner.BS_Stop(BS_SkorgeDuel_Draw_TwoStick, 0.2f);
	PawnOwner.BS_Stop(BS_SkorgeDuel_Win_OneStick, 0.2f);
	PawnOwner.BS_Stop(BS_SkorgeDuel_Lose_OneStick, 0.2f);
	PawnOwner.BS_Stop(BS_SkorgeDuel_Draw_OneStick, 0.2f);
	PawnOwner.Mesh.RootMotionMode = PawnOwner.default.Mesh.RootMotionMode;
	PawnOwner.BS_SetAnimEndNotify(BS_SkorgeDuel_Draw_Staff, TRUE);
	PawnOwner.BS_SetAnimEndNotify(BS_SkorgeDuel_Draw_TwoStick, TRUE);
	PawnOwner.BS_SetAnimEndNotify(BS_SkorgeDuel_Draw_OneStick, TRUE);

	Follower.BS_Stop(BS_SkorgeDuel_Win_Staff, 0.2f);
	Follower.BS_Stop(BS_SkorgeDuel_Lose_Staff, 0.2f);
	Follower.BS_Stop(BS_SkorgeDuel_Draw_Staff, 0.2f);
	Follower.BS_Stop(BS_SkorgeDuel_Win_TwoStick, 0.2f);
	Follower.BS_Stop(BS_SkorgeDuel_Lose_TwoStick, 0.2f);
	Follower.BS_Stop(BS_SkorgeDuel_Draw_TwoStick, 0.2f);
	Follower.BS_Stop(BS_SkorgeDuel_Win_OneStick, 0.2f);
	Follower.BS_Stop(BS_SkorgeDuel_Lose_OneStick, 0.2f);
	Follower.BS_Stop(BS_SkorgeDuel_Draw_OneStick, 0.2f);
	Follower.Mesh.RootMotionMode = Follower.default.Mesh.RootMotionMode;

	if( bWon )
	{
		Follower.Died( PawnOwner.Controller, class'GDT_Chainsaw', Follower.Location );
	}

	Super.SpecialMoveEnded(PrevMove, NextMove);
}

defaultproperties
{
	BS_SkorgeDuel_Win_Staff=(AnimName[BS_FullBody]="Staff_Chainsaw_Duel_Win")
	BS_SkorgeDuel_Lose_Staff=(AnimName[BS_FullBody]="Staff_Chainsaw_Duel_Lose")
	BS_SkorgeDuel_Draw_Staff=(AnimName[BS_FullBody]="Staff_Chainsaw_Duel_Draw")

	BS_SkorgeDuel_Win_TwoStick=(AnimName[BS_FullBody]="2stick_Chainsaw_Duel_Win")
	BS_SkorgeDuel_Lose_TwoStick=(AnimName[BS_FullBody]="2stick_Chainsaw_Duel_Lose")
	BS_SkorgeDuel_Draw_TwoStick=(AnimName[BS_FullBody]="2stick_Chainsaw_Duel_Draw")

	BS_SkorgeDuel_Win_OneStick=(AnimName[BS_FullBody]="1stick_Chainsaw_Duel_Win")
	BS_SkorgeDuel_Lose_OneStick=(AnimName[BS_FullBody]="1stick_Chainsaw_Duel_Lose")
	BS_SkorgeDuel_Draw_OneStick=(AnimName[BS_FullBody]="1stick_Chainsaw_Duel_Draw")

	CameraAnimDrawList(0)=CameraAnim'COG_MarcusFenix.Camera_Anims.Staff_Chainsaw_Duel_Draw_Cam_01'
	CameraAnimDrawList(1)=CameraAnim'COG_MarcusFenix.Camera_Anims.Staff_Chainsaw_Duel_Draw_Cam_02'

	CameraAnimWinList(0)=CameraAnim'COG_MarcusFenix.Camera_Anims.Staff_Chainsaw_Duel_Win_Cam_01'
	CameraAnimWinList(1)=CameraAnim'COG_MarcusFenix.Camera_Anims.Staff_Chainsaw_Duel_Win_Cam_02'

	CameraAnimLoseList(0)=CameraAnim'COG_MarcusFenix.Camera_Anims.Staff_Chainsaw_Duel_Lose_Cam_01'
	CameraAnimLoseList(1)=CameraAnim'COG_MarcusFenix.Camera_Anims.Staff_Chainsaw_Duel_Lose_Cam_02'

	FollowerSpecialMove=SM_ChainsawDuel_Follower
	DefaultAICommand=class'AICmd_Base_PushedBySpecialMove'
}