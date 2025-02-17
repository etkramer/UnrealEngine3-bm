
/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class GSM_PushObject extends GearSpecialMove
	native(SpecialMoves)
	config(Pawn);

/** GoW global macros */

/** Mantle up animation */
var()	GearPawn.BodyStance	BS_PushObject;
/** Percentage through the animation that the animation should continue if a button is pressed */
var()	float ContinuePercent;
/** Whether the special move should start checking for move continuation */
var		bool bCanCheckForContinueMove;
/** Whether the special move should continue after animation completes or not */
var		bool bContinueMove;
/** Length of the push animation */
var		float PushAnimLength;

protected function bool InternalCanDoSpecialMove()
{
	return( PawnOwner.IsInCover() && CoverLink_Pushable(PawnOwner.CurrentLink) != None );
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local InterpActor_Pushable PushObject;
	local GearHUD_Base WH;

	Super.SpecialMoveStarted(bForced,PrevMove);

	if ( PawnOwner.CurrentLink != None && PawnOwner.CurrentLink.Base != None )
	{
		PushObject = InterpActor_Pushable(PawnOwner.CurrentLink.Base);
		if ( PushObject != None )
		{
			PushObject.PushDirection = vector(PawnOwner.Rotation);
		}
		else
		{
			`Log("Error: Could not find the pushable object in specialmove!");
		}
	}
	else
	{
		`log("Error: Could not find coverlink or the pushable object in specialmove!");
	}

	StartPushAnim();
	ButtonPressed();

	WH = GearHUD_Base(GearPC(PawnOwner.Controller).myHUD);
	if ( WH != None )
	{
		WH.SetActionInfo(AT_SpecialMove, Action, PawnOwner.bIsMirrored);
	}
}

/**
* Start playing the push animation and start the timer for checking for continuation.
*/
simulated function StartPushAnim()
{
	// Play body stance animation.
	PushAnimLength = PawnOwner.BS_Play(BS_PushObject, SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier, TRUE);

	// prep move for checking for continuation.
	ResetContinueCheck();
}

/**
 *	Reset continue check.
 */
simulated function ResetContinueCheck()
{
	// set timer to check for button presses
	bCanCheckForContinueMove = false;
	bContinueMove = false;
	PawnOwner.SetTimer(  PushAnimLength*ContinuePercent, false, nameof(self.CheckForContinue),  self  );
	PawnOwner.SetTimer(  PushAnimLength, false, nameof(self.EndOfAnim),  self  );
}

/**
* Once this timer goes off we start looking for button presses to continue the move.
*/
simulated function CheckForContinue()
{
	bCanCheckForContinueMove = true;
}

/**
 * Once this timer goes off we must see if the button was pressed, if not stop the move.
 */
simulated function EndOfAnim()
{
	if ( !bContinueMove )
	{
		PawnOwner.EndSpecialMove();
	}
	else
	{
		ResetContinueCheck();
	}
}

/**
 * The Push button was pressed so exert force and check for continuation.
 */
simulated function ButtonPressed()
{
	local InterpActor_Pushable PushObject;

	if ( bCanCheckForContinueMove )
	{
		bContinueMove = true;
	}

	PushObject = InterpActor_Pushable(PawnOwner.CurrentLink.Base);
	if ( PushObject != None )
	{
		PushObject.Pushed();
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	PawnOwner.BS_Stop(BS_PushObject, 0.2f);
}

defaultproperties
{
	BS_PushObject=(AnimName[BS_FullBody]="AR_Cov_Mid_Push")

	bShouldAbortWeaponReload=TRUE
	bCanFireWeapon=FALSE
	bLockPawnRotation=TRUE

	ContinuePercent=0.75f

	Action={(
	ActionName=PushObject,
	IconAnimationSpeed=0.1f,
	ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=330,V=314,UL=45,VL=32),	// mash A button
									  (Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=376,V=314,UL=45,VL=32))),
						(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=75,V=432,UL=85,VL=63)))	),
	)}
}
