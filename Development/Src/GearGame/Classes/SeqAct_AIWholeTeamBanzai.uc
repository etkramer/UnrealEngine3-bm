/**
* causes everyone on the selected team to charge
*
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class SeqAct_AIWholeTeamBanzai extends SequenceAction;

var() enum EChargeTeam
{
	ECT_COG,
	ECT_Locust,
	ECT_Both
} TeamThatShouldCharge;

event Activated()
{
	if(TeamThatShouldCharge == ECT_COG || TeamThatShouldCharge == ECT_Both)
	{
		class'AICmd_Attack_Banzai'.static.EveryoneAttack(GetWorldInfo(),0);
	}

	if(TeamThatShouldCharge == ECT_Locust || TeamThatShouldCharge == ECT_Both)
	{
		class'AICmd_Attack_Banzai'.static.EveryoneAttack(GetWorldInfo(),1);
	}
}

defaultproperties
{
	ObjName="AI: Force Team To Banzai Charge"
	ObjCategory="AI"
	bCallHandler=false
	VariableLinks.Empty
	TeamThatShouldCharge=ECT_Locust
}
