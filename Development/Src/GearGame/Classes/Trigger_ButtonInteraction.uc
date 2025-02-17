/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class Trigger_ButtonInteraction extends Trigger;

event Touch(Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal)
{
	local Pawn P;
	local GearAI AI;

	// get AI to push the button
	P = Pawn(Other);
	if (P != None && P.Health > 0)
	{
		// tickers trigger buttons by exploding on them
		if (P.IsA('GearPawn_LocustTickerBase'))
		{
			P.Suicide();
		}
		else
		{
			AI = GearAI(P.Controller);
			if (AI != None)
			{
				class'AICmd_PushButton'.static.PushButton(AI, self);
			}
		}
	}
}

defaultproperties
{
	Begin Object Name=CollisionCylinder
		CollisionRadius=128.f
		CollisionHeight=128.f
	End Object

	SupportedEvents.Add(class'SeqEvt_ButtonPush')
	SupportedEvents.Remove(class'SeqEvent_Used')
}
