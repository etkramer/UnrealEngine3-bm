
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_ChainsawVictim extends GSM_InteractionPawnFollower_Base
	dependson(GSM_ChainsawAttack);

function InteractionStarted()
{
	Super.InteractionStarted();

	// If we're in god mode or have unlimited health, we're not going to die, so keep inventory.
	if( !PawnOwner.InGodMode() && !PawnOwner.bUnlimitedHealth )
	{
		PawnOwner.TossInventory(PawnOwner.Weapon);
	}

	if (PCOwner != None && PCOwner.IsLocalPlayerController())
	{
		PawnOwner.SetTimer(0.4f,FALSE,nameof(SpawnBlood),self);
	}
}

function SpawnBlood()
{
	PCOwner.ClientSpawnCameraLensEffect( class'Emit_CameraBlood_Chainsaw' );
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	// safety kill, if somehow GSM_ChainsawAttack fails to kill the owner
	if( PawnOwner != None && PawnOwner.WorldInfo.NetMode != NM_Client && PawnOwner.IsGameplayRelevant() && !PawnOwner.bUnlimitedHealth )
	{
		`warn("victim special move forcing Died(), this shouldn't happen");
		ScriptTrace();
		if( Leader != None )
		{
			PawnOwner.Died(Leader.Controller, class'GDT_Chainsaw', PawnOwner.Location);
		}
		else
		{
			PawnOwner.Died(None, class'GDT_Chainsaw', PawnOwner.Location);
		}
	}
}


function bool SetChainsawAnims(GearPawn GP, out GSM_ChainsawAttack.HeightAnims Anims, float BlendTime, EAnimType AnimType)
{
	return true; // true indicates SetChainsawHeightAnims should be called for us on the attacking GSM
}

defaultproperties
{
}