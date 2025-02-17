//=============================================================================
// The Basic constraint actor class.
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================

class RB_ConstraintActor extends Actor
    abstract
	placeable
	native(Physics);

cpptext
{
	virtual void physRigidBody(FLOAT DeltaTime) {};
	virtual void PostEditMove(UBOOL bFinished);
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void CheckForErrors(); // used for checking that this constraint is valid buring map build

	virtual void InitRBPhys();
	virtual void TermRBPhys(FRBPhysScene* Scene);

	void UpdateConstraintFramesFromActor();
}

// Actors joined effected by this constraint (could be NULL for 'World')
var() Actor												ConstraintActor1;
var() Actor												ConstraintActor2;

var() editinline export noclear RB_ConstraintSetup		ConstraintSetup;
var() editinline export noclear RB_ConstraintInstance	ConstraintInstance;

// Disable collision between actors joined by this constraint.
var() const bool						bDisableCollision;

var() bool								bUpdateActor1RefFrame;
var() bool								bUpdateActor2RefFrame;

// Used if joint is a pulley to define pivot locations using actors in the level.
var(Pulley) Actor					PulleyPivotActor1;
var(Pulley) Actor					PulleyPivotActor2;

native final function SetDisableCollision(bool NewDisableCollision);
native final function InitConstraint(Actor Actor1, Actor Actor2, optional name Actor1Bone, optional name Actor2Bone, optional float BreakThreshold);
native final function TermConstraint();

/**
 * When destroyed using Kismet, break the constraint.
 */
simulated function OnDestroy(SeqAct_Destroy Action)
{
	TermConstraint();
}

defaultproperties
{
	TickGroup=TG_PostAsyncWork

	Begin Object Class=SpriteComponent Name=Sprite
		Sprite=Texture2D'EditorResources.S_Actor'
		HiddenGame=True
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(Sprite)

	Begin Object Class=RB_ConstraintInstance Name=MyConstraintInstance
	End Object
	ConstraintInstance=MyConstraintInstance

	Begin Object Class=RB_ConstraintDrawComponent Name=MyConDrawComponent
	End Object
	Components.Add(MyConDrawComponent)

	bDisableCollision=false

	SupportedEvents.Add(class'SeqEvent_ConstraintBroken')

	bUpdateActor1RefFrame=true
	bUpdateActor2RefFrame=true

	bCollideActors=false
	bHidden=True
	DrawScale=0.5
	bEdShouldSnap=true

	Physics=PHYS_RigidBody
	bStatic=false
	bNoDelete=true
}
