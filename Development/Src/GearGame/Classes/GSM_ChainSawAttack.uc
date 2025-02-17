
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_ChainSawAttack extends GSM_InteractionPawnLeader_Base;

/** Flag set if this special move was called as a result of a dueling. */
var bool				bFromDuel;
/** TRUE if dueling leader has won. FALSE if dueling follower did. */
var bool				bLeaderWonDuel;
/** Play attack from behind animation */
var bool				bAttackFromBehind;

/** Struct used to store the animations needed for height based 3 way blends */
struct  HeightAnims
{
	var()	Name	U;	// Up
	var()	Name	C;	// Center
	var()	Name	D;	// Down
};

/** Struct to store dueling animations. They are sync'd animations working in pairs. */
struct ChainsawAnim
{
	var()	HeightAnims	Leader;
	var()	HeightAnims	Follower;
	var()	FLOAT		MaxHeightDifference;
	var()	FLOAT		AlignDistance;
};

Enum EAnimType
{
	EDuelWin,
	EFrontAttack,
	EBackAttack
};

/** Height based animations for front chainsaw attack, back and duel */
var ChainsawAnim		Duel_Win, FrontAttack, BackAttack;

/** Played Chainsaw height based animation */
var	ChainsawAnim	PlayedAnim;
/** TRUE if we're playing a chainsaw height based animation */
var bool			bPlayingChainsawAnim;

/** TRUE if we've intentionally killed the victim. */
var bool			bIntentionallyKilledVictim;

// Camera Anims
var Array<CameraAnim>	FrontAttackCameraAnims, BackAttackCameraAnims;
var Array<CameraAnim>	CameraAnimDuelWin;

/** DBNO body stance animation */
var BodyStance			DBNOVictimAnim;

protected function bool InternalCanDoSpecialMove()
{
	// extra check to avoid potential weapon swap chainsaw exploits
	if (PawnOwner == None || GearWeap_AssaultRifle(PawnOwner.Weapon) == None)
	{
		return FALSE;
	}
	return Super.InternalCanDoSpecialMove();
}

function bool CanInteractWithPawn(GearPawn OtherPawn)
{
	local float ChainsawDuelFacingDotAngle;
	local vector LookDir;
	local vector PlayerDir;

	if (!InternalCanDoSpecialMove())
	{
		return FALSE;
	}

	// if dead or dueling, then disallow instigating a new attack
	if (PawnOwner.Health <= 0 || PawnOwner.IsChainsawDueling())
	{
		return FALSE;
	}

	// ignore pawns doing dodge moves. They can escape executions.
	if( OtherPawn.IsDoingADodgeMove() )
	{
		return FALSE;
	}

	// ignore invalid melee targets
	if( !PawnOwner.TargetIsValidMeleeTarget(OtherPawn,TRUE) || !OtherPawn.CanBeSpecialMeleeAttacked(PawnOwner) )
	{
		return FALSE;
	}

	// no chainsaw dueling if same team, even with friendly fire
	if (PawnOwner.GetTeamNum() == OtherPawn.GetTeamNum())
	{
		return FALSE;
	}

	// ignore god mode targets
	if (OtherPawn.Controller != None && OtherPawn.Controller.bGodMode)
	{
		return FALSE;
	}

	// if in single player, don't allow COG AI to be chainsawed unless DBNO
	if ( OtherPawn.Controller != None && 
		!OtherPawn.IsHumanControlled() && !OtherPawn.IsDBNO() && OtherPawn.GetTeamNum() == 0 &&
		GearGameSP_Base(OtherPawn.WorldInfo.Game) != None )
	{
		return false;
	}

	// Players facing each other test. Turn angle in degres to the result of a dot product.
	ChainsawDuelFacingDotAngle = 0.f;

	// Test if we're facing the Other Pawn
	LookDir		= Vector(PawnOwner.GetBaseAimRotation());
	LookDir.Z	= 0.f;
	PlayerDir	= OtherPawn.Location - PawnOwner.Location;
	PlayerDir.Z = 0.f;
	if( (Normal(PlayerDir) dot Normal(LookDir)) < ChainsawDuelFacingDotAngle )
	{
		return FALSE;
	}

	// Kidnappers... Chainsaw Hostage if taken from the front.
	if( OtherPawn.IsAKidnapper() )
	{
		PlayerDir	= Vector(OtherPawn.Rotation);
		PlayerDir.Z = 0.f;

		if( (Normal(PlayerDir) dot Normal(LookDir)) < 0.f )
		{
			return FALSE;
		}
	}

	// Hostages... Chainsaw Kidnapper if taken from the back
	if( OtherPawn.IsAHostage() )
	{
		PlayerDir	= Vector(OtherPawn.Rotation);
		PlayerDir.Z = 0.f;

		if( (Normal(PlayerDir) dot Normal(LookDir)) >= 0.f )
		{
			return FALSE;
		}
	}

	// all good
	return TRUE;
}

/**
 * Pack flags needed for chainsaw attack special move.
 * These are replicated so they're consistent on all netmodes.
 * @param	bInFromDuel			If this special move as been triggered as a result of a chainsaw dueling.
 * @param	bInLeaderWonDuel	TRUE if the Dueling leader has won. FALSE if the Dueling follower has won.
 */
static function INT PackChainsawAttackFlags(bool bInFromDuel, optional bool bInLeaderWonDuel, optional GearPawn Attacker, optional GearPawn Victim)
{
	local INT		PackedFlags;
	local Vector	AttackerDir, VictimDir;
	local bool		bInAttackFromBehind;

	// If not from a duel and chainsawing a pawn, figure out if we're doing the chainsaw from behind animation
	if( !bInFromDuel && Attacker != None && Victim != None )
	{
		AttackerDir = Vector(Attacker.GetBaseAimRotation());
		VictimDir = Vector(Victim.Rotation);
		AttackerDir.Z = 0.f;
		VictimDir.Z = 0.f;

		if( (Normal(AttackerDir) dot Normal(VictimDir)) > 0 )
		{
			bInAttackFromBehind = TRUE;
		}
	}

	PackedFlags = int(bInFromDuel) | (int(bInLeaderWonDuel) << 1) | (int(bInAttackFromBehind) << 2);

	`log(GetFuncName() @ "PackedFlags:" @ PackedFlags @ "bInFromDuel:" @ bInFromDuel @ "bInLeaderWonDuel:" @ bInLeaderWonDuel @ "bInAttackFromBehind:" @ bInAttackFromBehind);
	return PackedFlags;
}

/** Unpack Special move flags */
function UnpackSpecialMoveFlags()
{
	bFromDuel = bool(PawnOwner.SpecialMoveFlags & 1);
	bLeaderWonDuel = bool(PawnOwner.SpecialMoveFlags & 2);
	bAttackFromBehind = bool(PawnOwner.SpecialMoveFlags & 4);

	`log(GetFuncName() @ "PackedFlags:" @ PawnOwner.SpecialMoveFlags @ "bFromDuel:" @ bFromDuel @ "bLeaderWonDuel:" @ bLeaderWonDuel @ "bAttackFromBehind:" @ bAttackFromBehind);
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	// Reset variables
	bIntentionallyKilledVictim = FALSE;
	bPlayingChainsawAnim = FALSE;

	// Unpack SpecialMove flags
	UnpackSpecialMoveFlags();

	// See if we should unlock the eGA_CrossedSwords achievement
	if( bFromDuel && PawnOwner.Controller.IsLocalPlayerController() )
	{
		GearPC(PawnOwner.Controller).ClientUpdateChainsawDuelWonProgress();
	}

	Super.SpecialMoveStarted(bForced, PrevMove);
	if( PawnOwner.WorldInfo.NetMode != NM_Client && Follower.IsGameplayRelevant() )
	{
		if (GearPRI(Follower.PlayerReplicationInfo) != None)
		{
			// and award the execution
			if (!Follower.bWasDBNO)
			{
				// translate the remaining health as damage done
				GearPRI(Follower.PlayerReplicationInfo).ScoreHit(GearPRI(PawnOwner.PlayerReplicationInfo),Follower.DefaultHealth,Follower.Health,Follower.Health,class'GDT_Chainsaw');
				// and insta kill
				GearPRI(PawnOwner.PlayerReplicationInfo).ScoreInstantKill(GearPRI(Follower.PlayerReplicationInfo),class'GDT_Chainsaw',GDT_NORMAL);
			}
			else
			{
				// otherwise it's just an execution
				GearPRI(PawnOwner.PlayerReplicationInfo).ScoreExecution(GearPRI(Follower.PlayerReplicationInfo),class'GDT_Chainsaw',GDT_NORMAL);
			}
		}
 	}

	PawnOwner.SoundGroup.PlayEffort(PawnOwner, GearEffort_ChainSawAttackEffort);

	if(Pawnowner.MyGearAI != none)
	{
		Pawnowner.MyGearAI.DoMeleeAttack();
	}
}

function StartInteraction()
{
	local GearWeap_AssaultRifle Rifle;

	// Make sure only the kidnapper kills the victim now.
	Follower.SpecialMoves[Follower.SpecialMove].bOnlyInteractionPawnCanDamageMe = TRUE;

	// tell the rifle to start the slashing audio/fx
	Rifle = GearWeap_AssaultRifle(PawnOwner.Weapon);
	if( Rifle != None )
	{
		Rifle.bAttackFromBehind = bAttackFromBehind;
		Rifle.StartSlashing();
	}

	// play the attack animation
	//@note - the anim will kick off notifies to trigger the fx
	if( Follower.PreviousSpecialMove == SM_DBNO )
	{
		// Special case for DBNO kills
		Follower.BS_Play(DBNOVictimAnim, 1.f, 0.33f, -1.f, TRUE);
		PlayHeightChainsawAnim(FrontAttack, EFrontAttack, TRUE);
		PlayExecutionCameraAnim(FrontAttackCameraAnims);
	}
	else if( bFromDuel )
	{
		PlayHeightChainsawAnim(Duel_Win,EDuelWin);
		PlayExecutionCameraAnim(CameraAnimDuelWin);
	}
	// Attack from behind
	else if( bAttackFromBehind )
	{
		PlayHeightChainsawAnim(BackAttack,EBackAttack);
		// For now play CameraAnim randomly. It should eventually handle collision.
		PlayExecutionCameraAnim(BackAttackCameraAnims);
		bAlignFollowerLookSameDirAsMe = TRUE;
	}
	// Front attack
	else
	{
		PlayHeightChainsawAnim(FrontAttack, EFrontAttack);
		// Play execution Camera Animation.
		PlayExecutionCameraAnim(FrontAttackCameraAnims);
	}
}

function PlayHeightChainsawAnim(ChainSawAnim InChainsawAnim, EAnimType AnimType, optional bool bSkipVictim)
{
	local float PlaybackLength;
	local GSM_ChainSawVictim VictimSM;

	// Save which animations we're playing
	PlayedAnim = InChainsawAnim;
	bPlayingChainsawAnim = TRUE;

	SetChainsawHeightAnims(PawnOwner, InChainsawAnim.Leader, 0.2f);

	if( !bSkipVictim )
	{
		VictimSM = GSM_ChainSawVictim(Follower.SpecialMoves[FollowerSpecialMove]);
		// if victim SM returns true, means we should call SetChainsawHeightAnims 
		if(VictimSM == none || VictimSM.SetChainsawAnims(Follower, InChainsawAnim.Follower, 0.2f, AnimType))
		{
			SetChainsawHeightAnims(Follower,InChainsawAnim.Follower,0.2f);
		}
	}

	// Setup physics to align pawns.
	bAlignPawns = TRUE;
	AlignDistance = InChainsawAnim.AlignDistance;

	if( PawnOwner.BlendByAimNode != None )
	{
		PlaybackLength = PawnOwner.BlendByAimNode.GetAnimPlaybackLength() - 0.2f;
		if( PlaybackLength > 0.f )
		{
			PawnOwner.SetTimer( PlaybackLength, FALSE, nameof(self.ChainsawAnimEnd), self );
		}
		else
		{
			ChainsawAnimEnd();
		}
	}
	else
	{
		ChainsawAnimEnd();
	}
}

function SetChainsawHeightAnims(GearPawn GP, HeightAnims Anims, float BlendTime)
{
	if( GP.BlendByAimNode != None )
	{
		GP.BlendByAimNode.AnimName_CU = Anims.U;
		GP.BlendByAimNode.AnimName_CC = Anims.C;
		GP.BlendByAimNode.AnimName_CD = Anims.D;
		// Make sure it's vertical blend only.
		GP.BlendByAimNode.HorizontalRange = vect2d(0,0);
		// Clear translation
		GP.BlendByAimNode.bZeroRootTranslation = TRUE;
		// Update Animations
		GP.BlendByAimNode.CheckAnimsUpToDate();
		// Start animation
		GP.BlendByAimNode.PlayAnim();
	}
	else
	{
		`Warn(GetFuncName() @ "BlendByAimNode==None!!! Cannot set proper dueling animations!");
		ScriptTrace();
	}

	// Turn on blend
	if( GP.BlendByAimToggle != None )
	{
		GP.BlendByAimToggle.SetActiveChild(1, BlendTime);
	}
	else
	{
		`Warn(GetFuncName() @ "BlendByAimToggle==None!!! Cannot turn on animation!");
		ScriptTrace();
	}
}

function ChainsawAnimEnd()
{
	bPlayingChainsawAnim = FALSE;
	bAlignPawns = FALSE;

	// Clear Timer just in case.
	PawnOwner.ClearTimer('ChainsawAnimEnd', Self);

	// Turn off blend
	if( PawnOwner.BlendByAimToggle != None )
	{
		PawnOwner.BlendByAimToggle.SetActiveChild(0, 0.2f);
	}
	// Turn off blend
	if( Follower.BlendByAimToggle != None )
	{
		Follower.BlendByAimToggle.SetActiveChild(0, 0.2f);
	}

	// Stop special move if not already doing so.
	if( !PawnOwner.bEndingSpecialMove )
	{
		PawnOwner.EndSpecialMove();
	}
}

function Tick(float DeltaTime)
{
	local float HeightDifferencePct, FollowerZ;

	// Adjust height difference for position aimoffset nodes.
	if( bPlayingChainsawAnim && PawnOwner != None && Follower != None && PawnOwner.IsGameplayRelevant() && Follower.IsGameplayRelevant() )
	{
		FollowerZ = Follower.Location.Z;
		// If doing the DBNO death version, then chainsaw a bit lower because the victim is on its knees.
		if( Follower.PreviousSpecialMove == SM_DBNO )
		{
			FollowerZ -= Follower.GetCollisionHeight();
		}
		else if( Follower.IsA('GearPawn_LocustWretchBase') )
		{
			FollowerZ -= Follower.GetCollisionHeight();
		}

		HeightDifferencePct = (FollowerZ - PawnOwner.Location.Z) / (2.f * PlayedAnim.MaxHeightDifference);
		PawnOwner.PositionAdjustAimOffsetPct = vect2d(0.f, HeightDifferencePct);
		Follower.PositionAdjustAimOffsetPct = vect2d(0.f, -HeightDifferencePct);
	}

	Super.Tick(DeltaTime);
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

	Text = "Distance:" @ Distance @ "HeightDifference:" @ HeightDifference @ "HeightDifferencePct:" @ (HeightDifference / class'GSM_ChainsawDuel_Leader'.default.MaxHeightDifference);
	Canvas.StrLen(Text, XL, YL);
	Canvas.SetPos((H.SizeX - XL)/2.f, H.SizeY/2.f + YL * 2.f);
	Canvas.SetDrawColor(255,0,0);
	Canvas.DrawText(TEXT, FALSE);
}
*/

/** Notification called when we should kill the victim */
function ChainsawAnimNotify()
{
	bIntentionallyKilledVictim = TRUE;

	// Kill Victim
	DetachAndKillVictim();
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	local GearWeap_AssaultRifle Rifle;

	if( bPlayingChainsawAnim )
	{
		ChainsawAnimEnd();
	}

	// just to be safe, add an extra stop to the chainsaw (in case we get interrupted before the anim notify gets kicked off)
	Rifle = GearWeap_AssaultRifle(PawnOwner.Weapon);
	if( Rifle != None )
	{
		Rifle.StopRipping();
		Rifle.StopBlade();
		Rifle.StopBloodTrails( PawnOwner );
	}

	// Make sure victim is detached and dead
	if( !bIntentionallyKilledVictim )
	{
		bIntentionallyKilledVictim = TRUE;
		DetachAndKillVictim();
	}

	// check to see if the player was holding fire
	if (PCOwner != None && PCOwner.IsLocalPlayerController() && PCOwner.IsButtonActive(GB_RightTrigger,TRUE))
	{
		PCOwner.bFire = 1;
	}

	Super.SpecialMoveEnded(PrevMove, NextMove);
}

function DetachAndKillVictim()
{
	if( Follower != None )
	{
		// Turn off forced alignment.
		bAlignPawns = FALSE;

		if( PawnOwner.WorldInfo.NetMode != NM_Client && Follower.IsGameplayRelevant() )
		{
			if( !Follower.InGodMode() && !Follower.bUnlimitedHealth )
			{
				Follower.TearOffMomentum = Normal(Follower.Location - PawnOwner.Location);
				Follower.Died(PawnOwner.Controller, class'GDT_Chainsaw', Follower.Location);
			}
			else
			{
				Follower.ServerEndSpecialMove();
			}
		}
	}
}

/** Notification when Follower is leaving his FollowerSpecialMove */
function OnFollowerLeavingSpecialMove()
{
	if( !bIntentionallyKilledVictim )
	{
		// Make sure victim is detached and dead
		DetachAndKillVictim();

		// If victim has been kicked out of her special move unintentionally, then abort attack.
		if( PawnOwner.WorldInfo.NetMode != NM_Client && !PawnOwner.bEndingSpecialMove )
		{
			PawnOwner.ServerEndSpecialMove();
		}
	}
}


defaultproperties
{
	AlignDistance=101.10f
	FollowerSpecialMove=SM_ChainsawVictim

	DBNOVictimAnim=(AnimName[BS_FullBody]="AR_Melee_Saw_Death_Low")
	FrontAttack=(Leader=(D="AR_Melee_Saw_A_Above",C="AR_Melee_Saw_A",U="AR_Melee_Saw_A_Below"),Follower=(D="AR_Melee_Saw_Death_Above",C="AR_Melee_Saw_Death",U="AR_Melee_Saw_Death_Below"),MaxHeightDifference=51.70f,AlignDistance=108.20f)
	FrontAttackCameraAnims(0)=CameraAnim'COG_MarcusFenix.Camera_Anims.AR_Melee_Saw_A_Cam01'
	FrontAttackCameraAnims(1)=CameraAnim'COG_MarcusFenix.Camera_Anims.AR_Melee_Saw_A_Cam01'

	BackAttack=(Leader=(D="AR_Melee_Saw_Behind_Above",C="AR_Melee_Saw_Behind",U="AR_Melee_Saw_Behind_Below"),Follower=(D="AR_Melee_Saw_BehindDeath_Above",C="AR_Melee_Saw_BehindDeath",U="AR_Melee_Saw_BehindDeath_Below"),MaxHeightDifference=50.52f,AlignDistance=110.56f)
	BackAttackCameraAnims(0)=CameraAnim'COG_MarcusFenix.Camera_Anims.AR_Melee_Saw_A_Cam01'
	BackAttackCameraAnims(1)=CameraAnim'COG_MarcusFenix.Camera_Anims.AR_Melee_Saw_A_Cam01'

	CameraAnimDuelWin(0)=CameraAnim'COG_MarcusFenix.Camera_Anims.Duel_Win_Cam01'
	Duel_Win=(Leader=(D="Duel_Win_Above",C="Duel_Win",U="Duel_Win_Below"),Follower=(D="Duel_Lose_Above",C="DUEL_LOSE",U="Duel_Lose_Below"),MaxHeightDifference=67.97f,AlignDistance=138.65f)

	bDisablePOIs=TRUE
}
