/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
//=============================================================================
// CheatManager
// Object within playercontroller that manages "cheat" commands
//=============================================================================

class ExampleCheatManager extends CheatManager within ExamplePlayerController
	dependson(OnlineProfileSettings);

exec function HackRegisterLocalTalker()
{
	if( OnlineSub != None )
	{
		OnlineSub.VoiceInterface.RegisterLocalTalker( 0 );
		OnlineSub.VoiceInterface.StartSpeechRecognition( 0 );

		OnlineSub.VoiceInterface.AddRecognitionCompleteDelegate( 0, OnRecognitionComplete );
	}
}

function OnRecognitionComplete()
{
	local array<SpeechRecognizedWord> Words;
	local int i;

	if( OnlineSub != None )
	{
		OnlineSub.VoiceInterface.GetRecognitionResults( 0, Words );
		for (i = 0; i < Words.length; i++)
		{
			`Log("Speech recognition got word:" @ Words[i].WordText);
		}
	}
}

