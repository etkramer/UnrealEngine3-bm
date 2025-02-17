/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This interface deals with capturing gameplay events for logging with an online service
 */
interface OnlineEventsInterface;

/**
 * Initializes the events capture interface
 *
 * @param MaxNumEvents the maximum number of events that will be sent
 */
function bool Init(int MaxNumEvents);

/**
 * Begins a logging event (creates a new log entry)
 */
function BeginLog();

/**
 * Closes a logging event
 */
function EndLog();

/**
 * Creates an event within a log
 *
 * @param EventName the name to assign the event
 */
function BeginEvent(string EventName);

/**
 * Adds a parameter to the event log
 *
 * @param ParamName the name to assign the parameter
 * @param ParamValue the value to assign the parameter
 */
function AddParamInt(string ParamName, int ParamValue);

/**
 * Adds a parameter to the event log
 *
 * @param ParamName the name to assign the parameter
 * @param ParamValue the value to assign the parameter
 */
function AddParamFloat(string ParamName, float ParamValue);

/**
 * Adds a parameter to the event log
 *
 * @param ParamName the name to assign the parameter
 * @param ParamValue the value to assign the parameter
 */
function AddParamString(string ParamName, string ParamValue);

/**
 * Closes an event within a log
 */
function EndEvent();

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
 * Sends the hardware data to the server for statistics aggregation
 *
 * @param UniqueId the unique id for the player
 * @param PlayerNick the player's nick name
 *
 * @return true if the async task was started successfully, false otherwise
 */
function bool UploadHardwareData(UniqueNetId UniqueId,string PlayerNick);
