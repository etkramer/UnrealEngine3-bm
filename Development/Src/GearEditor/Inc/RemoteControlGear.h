//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================

#ifndef _REMOTE_CONTROL_WARFARE_H_
#define _REMOTE_CONTROL_WARFARE_H_


/**
 * This is our function which each of our pages will call in their constructor to make
 * certain the set of XRCs have been loaded.  
 *
 * We currently have some XRCs that share "pages" so this is needed.
 **/
void LoadGearRemoteControlXRCs();


#endif // _REMOTE_CONTROL_WARFARE_H_
