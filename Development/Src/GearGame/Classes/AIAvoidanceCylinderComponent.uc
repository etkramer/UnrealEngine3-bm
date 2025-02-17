/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AIAvoidanceCylinderComponent extends CylinderComponent
	native(AI)
	dependson(GearTeamInfo);

var native const Map{ANavigationPoint*,UBOOL} LinkedNavigationPoints;
var native const Map{UReachSpec*,UBOOL} LinkedReachSpecs;
var transient vector LastReLinkLocation;
var() float UpdateThreshold;
var() private bool bEnabled;

var() GearTeamInfo.EGearTeam TeamThatShouldFleeMe;

event ForceReLink()
{
	LastReLinkLocation=GetPosition() + vect(1,0,0) * UpdateThreshold * 10.f;
}

event SetEnabled(bool bInEnabled)
{
	local Actor CurrentTouch;
	local GearAI GAI;
	local GearPawn GP;
	if(!bEnabled && bInEnabled)
	{
		bEnabled = true;
		foreach Owner.Touching(CurrentTouch)
		{
			GP = GearPawn(CurrentTouch);
			if(GP != none)
			{
				GAI = GearAI(GP.Controller);

				if(GAI != none)
				{
					//`log("SetEnabled");
					AIEnteredAvoidanceCylinder(GAI);
				}

			}
		}
	}
	else
	{
		bEnabled = bInEnabled;
	}

}

native final function bool IsNavPointWithin(NavigationPoint Pt);
native final function bool DoesSpecIntersect(ReachSpec Spec);
/** @return whether there are currently any avoidance cylinders that would affect the passed in team
 * (pass TEAM_EVERYONE to return true if any cylinders exist at all)
 */
native static final function bool BuildListOfAffectingCylinders(GearAI AskingAI, out array<AIAvoidanceCylinderComponent> InAffectingCylinders);

cpptext
{
	static TDoubleLinkedList<UAIAvoidanceCylinderComponent*> GAvoidanceCylinders;
	public:
		virtual void Attach()
		{
			Super::Attach();

			if (GIsGame)
			{
				GAvoidanceCylinders.AddHead(this);
			}
		}
		virtual void Detach(UBOOL bWillReattach)
		{
			Super::Detach(bWillReattach);
			GAvoidanceCylinders.RemoveNode(this);
		}

		static TDoubleLinkedList<UAIAvoidanceCylinderComponent*>& GetList()
		{
			return GAvoidanceCylinders;
		}

		/** Called whenever isNavPointWithin or DoesSpecIntersect is called, and will update specs and navpoints if we have moved sufficiently far away */
		void ConditionalReLinkToSpecsAndNavPts();

		virtual void OverrideTraceFlagsForNonCollisionComponentChecks( DWORD& Flags )
		{
			// we only care about pawns!
			Flags = TRACE_Pawns;
		}
}

native function bool ShouldAIAvoidMe(GearAI AskingAI);

// script shouldavoid gets called after all the base conditions are met, gives script another chance to say no, don't call this fromt eh outside
protected event bool InternalShouldAIAvoidMe(GearAI AskingAI)
{
	local AIObjectAvoidanceInterface AvoidInt;

	// if the object implements the interface, and doesn't want an avoid, bail
	AvoidInt = AIObjectAvoidanceInterface(Owner);
	if(AvoidInt != none && !AvoidInt.ShouldAvoid(AskingAI,self))
	{
		return false;
	}

	return true;
}

protected function bool ShouldAIEvade(GearAI AskingAI)
{
	local AIObjectAvoidanceInterface AvoidInt;

	// if the object implements the interface, and doesn't want an avoid, bail
	AvoidInt = AIObjectAvoidanceInterface(Owner);
	if(AvoidInt != none && !AvoidInt.ShouldEvade(AskingAI,self))
	{
		return false;
	}

	return true;
}

protected function bool ShouldAIRoadieRun(GearAI AskingAI)
{
	local AIObjectAvoidanceInterface AvoidInt;

	// if the object implements the interface, and doesn't want an avoid, bail
	AvoidInt = AIObjectAvoidanceInterface(Owner);
	if(AvoidInt != none && !AvoidInt.ShouldRoadieRun(AskingAI,self))
	{
		return false;
	}

	return true;
}

function AIEnteredAvoidanceCylinder(GearAI EnteringAI)
{

	//ScriptTrace();
	//`log(GetFuncname()@EnteringAI);
	if(ShouldAIAvoidme(EnteringAI))
	{
		if(ShouldAIEvade(EnteringAI))
		{
			EnteringAI.ReactionManager.NudgeChannel(Owner,'EnteredEvadeAvoidanceZone');
		}
		else if(ShouldAIRoadieRun(EnteringAI))
		{
			EnteringAI.ReactionManager.NudgeChannel(Owner,'EnteredRoadieRunAvoidanceZone');
		}
		else
		{
			EnteringAI.ReactionManager.NudgeChannel(Owner,'EnteredAvoidanceZone');
		}
	}
	
}




defaultproperties
{
	LastReLinkLocation=(x=-65535)
	BlockZeroExtent=false
	BlockNonZeroExtent=true
	CollideActors=true
	AlwaysCheckCollision=true
	bEnabled=true
	UpdateThreshold=50.f
	TeamThatShouldFleeMe=TEAM_EVERYONE
}
