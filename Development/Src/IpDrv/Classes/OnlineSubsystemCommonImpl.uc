/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Class that implements commonly needed members/features across all platforms
 */
class OnlineSubsystemCommonImpl extends OnlineSubsystem
	native
	config(Engine);

/**
 * Holds the pointer to the platform specific FVoiceInterface implementation
 * used for voice communication
 */
var const native transient pointer VoiceEngine{class FVoiceInterface};

/** Holds the maximum number of local talkers allowed */
var config int MaxLocalTalkers;

/** Holds the maximum number of remote talkers allowed (clamped to 30 which is XHV max) */
var config int MaxRemoteTalkers;

/** Whether speech recognition is enabled */
var config bool bIsUsingSpeechRecognition;

/** The object that handles the game interface implementation across platforms */
var OnlineGameInterfaceImpl GameInterfaceImpl;

var native const Pointer InfocasterDownload;

/**
 * Returns the name of the player for the specified index
 *
 * @param UserIndex the user to return the name of
 *
 * @return the name of the player at the specified index
 */
event string GetPlayerNicknameFromIndex(int UserIndex);

/**
 * Returns the unique id of the player for the specified index
 *
 * @param UserIndex the user to return the id of
 *
 * @return the unique id of the player at the specified index
 */
event UniqueNetId GetPlayerUniqueNetIdFromIndex(int UserIndex);