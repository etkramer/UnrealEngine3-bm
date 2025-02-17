
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_Sire_ChainsawVictim extends GSM_ChainsawVictim;

var BodyStance BS_ReactAnim;
function bool SetChainsawAnims(GearPawn GP, out GSM_ChainsawAttack.HeightAnims Anims, float BlendTime, EAnimType AnimType)
{
	PawnOwner.BS_Play(BS_ReactAnim,1.f,0.f,0.1f,TRUE);
	return false; 
}
defaultproperties
{
	BS_ReactAnim=(AnimName[BS_FullBody]="chainsaw_react")
}