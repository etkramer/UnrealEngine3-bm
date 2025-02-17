/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_AISetMood extends SequenceAction;

var() EPerceptionMood	PerceptionMood;
var() ECombatMood		CombatMood;
var() EAIMoveMood		MoveMood;

var() float				UnawareSightRadius;

defaultproperties
{
    ObjName="AI: Set Moods"
    ObjCategory="AI"

	UnawareSightRadius=-1
}
