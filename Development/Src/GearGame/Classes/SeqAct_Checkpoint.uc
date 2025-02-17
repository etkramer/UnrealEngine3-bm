/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_Checkpoint extends SeqAct_Latent;

/** classes of pawns in the player's squad to teleport. If empty, all squad members are teleported. */
var() array< class<Pawn> > TeleportClasses;
/** if squadmates are more than this far away, they will be teleported */
var() float TeleportDistance;
/** if squadmates are in one of these volumes, they will be teleported */
var() array<Volume> TeleportVolumes;

var() bool bOverrideSavedChapter;
var() EChapterPoint SavedChapterOverride;

/** human player to use as teleport target for squadmates that are too far away */
var Actor TeleportTargetPlayer;

/** time we were activated - for waiting one tick before triggering "Finished" output */
var transient float ActivationTime;

event Activated()
{
	local GearPC PC;

	if (bOverrideSavedChapter)
	{
		GearGameSP_Base(GetWorldInfo().Game).CurrentChapter = SavedChapterOverride;
	}

	foreach GetWorldInfo().LocalPlayerControllers(class'GearPC', PC)
	{
		PC.OnCheckpoint(self);
		break;
	}

	OutputLinks[0].bHasImpulse = true;

	ActivationTime = GetWorldInfo().TimeSeconds;
}

event Deactivated()
{
	if (OutputLinks.length > 1) // compat check
	{
		OutputLinks[1].bHasImpulse = true;
	}
}

static event int GetObjClassVersion()
{
	return Super.GetObjClassVersion() + 2;
}

/** @return whether the given Pawn is in one of the teleport volumes */
final function bool ShouldTeleport(Pawn TestPawn, vector TeleportLocation)
{
	local int i;

	if (TeleportDistance > 0.0 && VSize(TestPawn.Location - TeleportLocation) > TeleportDistance)
	{
		return true;
	}
	else if (TeleportVolumes.length > 0)
	{
		for (i = 0; i < TeleportVolumes.length; i++)
		{
			if (TeleportVolumes[i].Encompasses(TestPawn))
			{
				return false;
			}
		}

		return true;
	}
	else
	{
		return false;
	}
}

event bool Update(float DeltaTime)
{
	return (ActivationTime == GetWorldInfo().TimeSeconds);
}

defaultproperties
{
	ObjName="Checkpoint"
	ObjCategory="Gear"
	bCallHandler=false
	bAutoActivateOutputLinks=false

	InputLinks(0)=(LinkDesc="Save")
	InputLinks(1)=(LinkDesc="Load")
	OutputLinks(0)=(LinkDesc="Out")
	OutputLinks(1)=(LinkDesc="Finished")
	VariableLinks.Empty()
	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Teleport Target",PropertyName=TeleportTargetPlayer,MaxVars=1)

	TeleportDistance=2048.0
	ActivationTime=-100000.0
}
