/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class AIAvoidanceCylinder extends Actor
	placeable
	native(AI)
	implements(AIObjectAvoidanceInterface)
	dependson(AIAvoidanceCylinderComponent);

var() bool bEnabled;

var() editconst const AIAvoidanceCylinderComponent AvoidanceComp;

var() GearTeamInfo.EGearTeam TeamThatShouldFleeMe;

// for history, so we can track where they're coming from
var name OwnerName;

cpptext
{
	// AActor interface.
	virtual void EditorApplyScale(const FVector& DeltaScale, const FMatrix& ScaleMatrix, const FVector* PivotLocation, UBOOL bAltDown, UBOOL bShiftDown, UBOOL bCtrlDown);

}

function private native ForceReTouch();

simulated function PostBeginPlay()
{	
	AvoidanceComp.TeamThatShouldFleeMe = TeamThatShouldFleeMe;
	SetEnabled(bEnabled);
	Super.PostBeginPlay();
}

simulated function OnToggle(SeqAct_Toggle Action)
{
	// Turn ON
	if (Action.InputLinks[0].bHasImpulse)
	{
		bEnabled= true;
	}
	// Turn OFF
	else if (Action.InputLinks[1].bHasImpulse)
	{
		bEnabled= false;
	}
	// Toggle
	else if (Action.InputLinks[2].bHasImpulse)
	{
		bEnabled = !bEnabled;
	}

	SetEnabled(bEnabled);

}

function SetEnabled(bool bInEnabled)
{
	bEnabled = bInEnabled;
	AvoidanceComp.SetActorCollision(bEnabled,false,bEnabled);
	AvoidanceComp.SetEnabled(bEnabled);
	SetCollision(bEnabled);
}

function SetCylinderSize(float Radius,float Height)
{
	AvoidanceComp.SetCylinderSize(Radius,Height);
}

function SetAvoidanceTeam(EGearTeam TeamThatShouldAvoid)
{
	TeamThatShouldFleeMe = TeamThatShouldAvoid;
	AvoidanceComp.TeamThatShouldFleeMe = TeamThatShouldFleeMe;
}

event BaseChange()
{
	Super.BaseChange();
	
	if(Base != none)
	{
		OwnerName = Base.Name;
	}
}

/*
event Tick(float DeltaTime )
{
	local vector Offset;
	Super.Tick(DeltaTime);
	if(bEnabled)
	{
		Offset.Z = AvoidanceComp.CollisionHeight/2.f;
		DrawDebugCylinder(Location - Offset,Location+Offset,AvoidanceComp.CollisionRadius,10,255,0,0);
	}
}*/

function bool ShouldAvoid(GearAI AskingAI, AIAvoidanceCylinderComponent TriggeringComponent)
{
	local AIobjectAvoidanceInterface AvoidInt;
	// if we're based on something which implements this interface, let it arbitrate who should avoid
	if(Base != none)
	{
		AvoidInt = AIobjectAvoidanceInterface(Base);

		if(AvoidInt != none)
		{
			return AvoidInt.ShouldAvoid(AskingAI,TriggeringComponent);
		}
	}
	return true;
}

function bool ShouldEvade(GearAI AskingAI, AIAvoidanceCylinderComponent TriggeringComponent)
{
	local AIobjectAvoidanceInterface AvoidInt;
	// if we're based on something which implements this interface, let it arbitrate who should avoid
	if(Base != none)
	{
		AvoidInt = AIobjectAvoidanceInterface(Base);

		if(AvoidInt != none)
		{
			return AvoidInt.ShouldEvade(AskingAI,TriggeringComponent);
		}
	}
	return false;
}

function bool ShouldRoadieRun(GearAI AskingAI, AIAvoidanceCylinderComponent TriggeringComponent)
{
	local AIobjectAvoidanceInterface AvoidInt;
	// if we're based on something which implements this interface, let it arbitrate who should avoid
	if(Base != none)
	{
		AvoidInt = AIobjectAvoidanceInterface(Base);

		if(AvoidInt != none)
		{
			return AvoidInt.ShouldRoadieRun(AskingAI,TriggeringComponent);
		}
	}
	return true;
}


defaultproperties
{
	bEnabled=false

	Begin Object Class=SpriteComponent Name=Sprite
		Sprite=Texture2D'EditorResources.S_Actor'
		HiddenGame=False
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(Sprite)

	Begin Object Class=AIAvoidanceCylinderComponent Name=AvoidanceCollisionCylinder
		CollisionRadius=+00100.000000
		CollisionHeight=+0078.000000
		bEnabled=FALSE
	End Object
	CollisionComponent=AvoidanceCollisionCylinder
	AvoidanceComp=AvoidanceCollisionCylinder
	Components.Add(AvoidanceCollisionCylinder)

	bHidden=true
	bCollideActors=true
	bBlockActors=false
	bStatic=false
	bNoDelete=false

	TeamThatShouldFleeMe=TEAM_EVERYONE
}