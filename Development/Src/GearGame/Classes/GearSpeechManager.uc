/**
 * Class: GearSpeechManager
 * Globally monitors and arbitrates character speech.
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearSpeechManager extends Actor
	native(Sound);

var transient protected float	LastUpdateTime;
var protected const float		TimeBetweenUpdates;

cpptext
{
	virtual void TickSpecial(FLOAT DeltaTime);
}

struct native ActiveDialogueLine
{
	/** Who is speaking */
	var Actor						Speaker;

	/** Who is being spoken to */
	var Actor						Addressee;

	/** Audio data */
	var	SoundCue					Audio;

	/** speech priority */
	var ESpeechPriority				Priority;
};


/** Stack of active dialogue lines. */
var array<ActiveDialogueLine>	DialogueStack;

var protected transient bool	bTrackDialogue;

function BeginDialogueTracking()
{
	//bTrackDialogue = TRUE;
	//DialogueStack.length = 0;		//empty
}

function EndDialogueTracking()
{
	//bTrackDialogue = FALSE;
	//DialogueStack.length = 0;		//empty
}

/** Call to notify the speech manager that this bit of dialogue is starting */
final event NotifyDialogueStart(Actor Speaker, Actor Addressee, SoundCue Audio, ESpeechPriority Pri)
{
	local ActiveDialogueLine Line;

	if (bTrackDialogue)
	{
		// fill in data
		Line.Speaker = Speaker;
		Line.Addressee = Addressee;
		Line.Audio = Audio;
		Line.Priority = Pri;

		// append
		DialogueStack[DialogueStack.length] = Line;
	}
}

/** Call to notify the speech manager that this bit of dialogue is finished */
final function NotifyDialogueFinish(Actor Speaker, SoundCue Audio)
{
	local ActiveDialogueLine Line;
	local int Idx;
	local bool bRemove;

	if (bTrackDialogue)
	{
		// find entry to remove
		foreach DialogueStack(Line, Idx)
		{
			if ( (Line.Speaker == Speaker) && (Line.Audio == Audio) )
			{
				bRemove = TRUE;
				break;
			}
		}

		// remove it
		if (bRemove)
		{
			DialogueStack.Remove(Idx, 1);
		}
		else
		{
		//	`warn( "Warning: SpeechManager got finish notification, but could not find line record." @ "Speaker:" @ Speaker @ "Addressee:" @ Addressee @ "SoundCue:" @ Audio );
		}
	}
}

simulated function bool IsSpeechActive(ESpeechPriority PriFilter)
{
	local int Idx;

	if (PriFilter > Speech_None)
	{
		for (Idx=0; Idx<DialogueStack.length; ++Idx)
		{
			if (DialogueStack[Idx].Priority == PriFilter)
			{
				return TRUE;
			}
		}

		return FALSE;
	}
	else
	{
		return DialogueStack.length > 0;
	}
			 
}

defaultproperties
{
	bTrackDialogue=TRUE
	TickGroup=TG_DuringAsyncWork
	TimeBetweenUpdates=3.f
}
