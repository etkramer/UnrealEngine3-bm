/**
 * Base class for events which are activated when some widget that contains data wishes to publish that data.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * @note: native because C++ code activates this event
 */
class UIEvent_SubmitData extends UIEvent
	native(inherit)
	abstract;

DefaultProperties
{
	ObjCategory="Data Submitted"
}
