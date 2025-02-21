/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Represents a "character" that can talk but isn't physically present in the world.
 */

class GearRemoteSpeaker extends ReplicationInfo
	native(Sound)
	abstract
	notplaceable;

/** Which GUDBank-based class I should be using for...umm.. GUD data. */
var array<string>	MasterGUDBankClassNames;

/** Index of the GUDBankClass currently being used by this pawn. */
var transient int	LoadedGUDBank;

/** true if currently speaking, false otherwise */
var transient bool	bSpeaking;

/** Data for a spoken line that has a delay.  Cached here until the line is really played. */
struct native RemoteSpeakerDelayedLine
{
	var GearPawn	Addressee;
	var SoundCue	Audio;
	var bool		bSuppressSubtitle;
	var float		DelayTime;
	var ESpeakLineBroadcastFilter MPBroadcastFilter;
	var string		DebugText;
	var ESpeechPriority Priority;
};
var repnotify RemoteSpeakerDelayedLine DelayedLineParams;

/** handle to audio for speech that is currently playing */
var AudioComponent	CurrentlySpeakingLine;

/** Team this speaker is on */
var int				TeamIndex;

/** Whether or not this remote speaker is able to speak. */
var bool			bEnabled;

/** TRUE if this pawn cannot speak GUDS lines temporarily. */
var protected transient bool	bMuteGUDS;


replication
{
	if (bNetDirty)
		DelayedLineParams;
};

/** Cause this pawn to speak the specified line.  Addressee can be None.  */
simulated event bool RemoteSpeakLine(GearPawn Addressee, SoundCue Audio, String DebugText, optional float DelaySec, optional bool bSuppressSubtitle, optional ESpeakLineBroadcastFilter MPBroadcastFilter, optional ESpeechPriority Priority)
{
	if (bEnabled && !bSpeaking)
	{
		DelayedLineParams.Addressee = Addressee;
		DelayedLineParams.Audio = Audio;
		DelayedLineParams.DebugText = DebugText;
		DelayedLineParams.bSuppressSubtitle = bSuppressSubtitle;
		DelayedLineParams.DelayTime = DelaySec;
		DelayedLineParams.MPBroadcastFilter = MPBroadcastFilter;
		DelayedLineParams.Priority = Priority;

		if (DelaySec > 0.f)
		{
			// play later
			SetTimer( DelaySec, FALSE, nameof(PlayQueuedSpeakLine) );
		}
		else
		{
			// play now!
			PlayQueuedSpeakLine();
		}

		bNetDirty = true; // have to force because struct member assignment doesn't do it automatically
		bForceNetUpdate = TRUE;

		return TRUE;
	}

	return FALSE;
}

simulated event ReplicatedEvent(name VarName)
{
	if (VarName == 'DelayedLineParams')
	{
		if (DelayedLineParams.Audio != None)
		{
			if (DelayedLineParams.DelayTime > 0.f)
			{
				// play later
				SetTimer( DelayedLineParams.DelayTime, false, nameof(PlayQueuedSpeakLine) );
			}
			else
			{
				// play now!
				PlayQueuedSpeakLine();
			}
		}
	}
}

/** Return true if speech from this pawn should be rejected based on the given filter. */
simulated final function bool ShouldFilterOutSpeech(ESpeakLineBroadcastFilter Filter, GearPawn Addressee)
{
	local PlayerController PC;

	switch (Filter)
	{
	case SLBFilter_None:
		return FALSE;

	case SLBFilter_SpeakerTeamOnly:
		// accept speech if there is a local player on the same team, reject otherwise
		foreach LocalPlayerControllers(class'PlayerController', PC)
		{
			if (PC.GetTeamNum() == TeamIndex)
			{
				return FALSE;
			}
		}
		return TRUE;

	case SLBFilter_SpeakerOnly:
		 // remote speakers can't be local players, always filter here
		return TRUE;

	case SLBFilter_SpeakerAndAddresseeOnly:
		return !Addressee.IsLocallyControlled();
	}

	return FALSE;
}


simulated protected function PlayQueuedSpeakLine()
{
`if(`notdefined(FINAL_RELEASE))
	local PlayerController PC;
`endif

	local bool bPlayingMulti;
	local float SpeakTime;

	if (bEnabled)
	{
		// handle current speech, if any
		if (CurrentlySpeakingLine != None)
		{
			CurrentlySpeakingLine.FadeOut(0.2f, 0.f);
		}

		bPlayingMulti = GearGRI(WorldInfo.GRI).IsMultiPlayerGame() || GearGRI(WorldInfo.GRI).IsCoopMultiplayerGame();

		// play the audio
		if ( !bPlayingMulti || !ShouldFilterOutSpeech(DelayedLineParams.MPBroadcastFilter, DelayedLineParams.Addressee) )
		{
			// play the audio
			if (DelayedLineParams.Audio != None)
			{
				CurrentlySpeakingLine = CreateAudioComponent(DelayedLineParams.Audio, false, true);
				if (CurrentlySpeakingLine != None)
				{
					// @fixme, distinguish between messages intended for 1 player vs intended for all players
					CurrentlySpeakingLine.bAllowSpatialization = FALSE;
					//@fixme, probably need a real priority - just using 1 for now so the subtitles show up at all
					CurrentlySpeakingLine.SubtitlePriority = 1;
					CurrentlySpeakingLine.bSuppressSubtitles = DelayedLineParams.bSuppressSubtitle;

					CurrentlySpeakingLine.bAutoDestroy = TRUE;
					AttachComponent( CurrentlySpeakingLine );
					CurrentlySpeakingLine.Play();
				}

				SpeakTime = DelayedLineParams.Audio.GetCueDuration() + 0.2f;
			}
			else
			{
				// time enough to read
				SpeakTime = 2.f;
			}

			// not really used for subtitles; left here in case LDs are using it for debug stuff
	`if(`notdefined(FINAL_RELEASE))
			if (DelayedLineParams.DebugText != "")
			{
				// find the player
				foreach LocalPlayerControllers(class'PlayerController', PC) { break; }
				PC.ClientMessage(Self@":"@DelayedLineParams.DebugText);
			}
	`endif

			ClearTimer('PlayQueuedSpeakLine');
			SetTimer( SpeakTime, FALSE, nameof(RemoteSpeakLineFinished) );
			bSpeaking = TRUE;
		}
	}

	if (bSpeaking)
	{
		GearGRI(WorldInfo.GRI).SpeechManager.NotifyDialogueStart(self, DelayedLineParams.Addressee, DelayedLineParams.Audio, DelayedLineParams.Priority);
	}
}


/**
 * Called when this pawn's spoken line is finished (naturally or interrupted), to give us
 * an opportunity to clean up.
 */
simulated event RemoteSpeakLineFinished()
{
	local RemoteSpeakerDelayedLine EmptyLine;

	GearGRI(WorldInfo.GRI).SpeechManager.NotifyDialogueFinish(self, CurrentlySpeakingLine.SoundCue);

	bSpeaking = FALSE;
	CurrentlySpeakingLine = None;

	// in case this function can be called from other ways than SetTimer
	ClearTimer('RemoteSpeakLineFinished');

	// clear text so the line doesn't get replicated to newly joined clients after it's over
	DelayedLineParams = EmptyLine;
}

simulated event bool IsSameTeam(Pawn P)
{
	return (P.GetTeamNum() == TeamIndex) ? TRUE : FALSE;
}

simulated function OnToggle(SeqAct_Toggle Action)
{
	if (Action.InputLinks[0].bHasImpulse)
	{
		// "turn on" input
		bEnabled = TRUE;
	}
	else if (Action.InputLinks[1].bHasImpulse)
	{
		// "turn off" input
		bEnabled = FALSE;
	}
	else if (Action.InputLinks[2].bHasImpulse)
	{
		// "toggle" input
		bEnabled = !bEnabled;
	}
}

simulated function PlayGUDSAction(EGUDActionID ActionID, optional GearPawn Addressee, optional GearPawn ReferringTo, optional ESpeakLineBroadcastFilter MPBroadcastFilter)
{
	GearGame(WorldInfo.Game).UnscriptedDialogueManager.PlayGUDSAction(ActionID, self, Addressee, ReferringTo, MPBroadcastFilter);
}

simulated function OnPawnMuteGUDS(SeqAct_PawnMuteGUDS Action)
{
	// 0 is mute, 1 is unmute
	bMuteGUDS = Action.InputLinks[0].bHasImpulse;
}



defaultproperties
{
	TickGroup=TG_DuringAsyncWork

	bEnabled=TRUE
	TeamIndex=255
}
