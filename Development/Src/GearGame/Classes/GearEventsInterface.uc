/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * The Gears implementation of event gathering.
 */
class GearEventsInterface extends Object
	implements(OnlineEventsInterface)
	native;

var int NumEvents;
var bool bInit;

native function bool Init(int MaxNumEvents);

native function BeginLog();

native function BeginEvent(string EventName);

native function AddParamInt(string ParamName, int ParamValue);

native function AddParamFloat(string ParamName, float ParamValue);

native function AddParamString(string ParamName, string ParamValue);

native function EndEvent();

native function EndLog();


/**
 * Uploads a log to a remote site
 */
function UploadLog();

/**
 * Writes the log to a local storage device
 */
function SaveLog();

/**
 * Sends the profile data to the server for statistics aggregation
 *
 * @param UniqueId the unique id for the player
 * @param PlayerNick the player's nick name
 * @param ProfileSettings the profile object that is being sent
 *
 * @return true if the async task was started successfully, false otherwise
 */
function bool UploadProfileData(UniqueNetId UniqueId,string PlayerNick,OnlineProfileSettings ProfileSettings);

/**
 * Sends the data contained within the gameplay events object to the online server for statistics
 *
 * @param Events the object that has the set of events in it
 *
 * @return true if the async send started ok, false otherwise
 */
function bool UploadGameplayEventsData(OnlineGameplayEvents Events);

/**
 * Sends the hardware data to the online server for statistics aggregation
 *
 * @param UniqueId the unique id for the player
 * @param PlayerNick the player's nick name
 *
 * @return true if the async task was started successfully, false otherwise
 */
function bool UploadHardwareData(UniqueNetId UniqueId,string PlayerNick);
