/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class CameraModifier extends Object
	native;

/* Interface class for all camera modifiers applied to
	a player camera actor. Sub classes responsible for
	implementing declared functions.
*/

/* Do not apply this modifier */
var	protected	bool	bDisabled;
/* This modifier is still being applied and will disable itself */
var				bool	bPendingDisable;

/** Camera this object is attached to */
var Camera	CameraOwner;


//debug
var(Debug) bool bDebug;

/** Allow anything to happen right after creation */
function Init();


/**
 * Directly modifies variables in the camera actor
 *
 * @param	Camera		reference to camera actor we are modifiying
 * @param	DeltaTime	Change in time since last update
 * @param	OutPOV		current Point of View, to be updated.
 * @return	bool		TRUE if should STOP looping the chain, FALSE otherwise
 */
native function bool ModifyCamera
(
		Camera	Camera,
		float	DeltaTime,
	out TPOV	OutPOV
);

/** Accessor function to check if modifier is inactive */
native function bool IsDisabled() const;

/**
 * Camera modifier evaluates itself vs the given camera's modifier list
 *	and decides whether to add itself or not
 *
 * @parma	Camera - reference to camera actor we want add this modifier to
 * @output	bool   - TRUE if modifier added to camera's modifier list, FALSE otherwise
 */
function bool AddCameraModifier( Camera Camera )
{
	//debug
	`Log( self@"AddModifier"@Camera.ModifierList.Length, bDebug );

	CameraOwner = Camera;

	// Default implementation adds self and always returns true
	Camera.ModifierList[Camera.ModifierList.Length] = self;
	return true;
}

/**
 * Camera modifier removes itself from given camera's modifier list
 *
 * @param	Camera	- reference to camara actor we want to remove this modifier from
 * @output	bool	- TRUE if modifier removed successfully, FALSE otherwise
 */
function bool RemoveCameraModifier( Camera Camera )
{
	local int ModifierIdx;

	//debug
	`Log( self@"RemoveModifier", bDebug );

	// Loop through each modifier in camera
	for( ModifierIdx = 0; ModifierIdx < Camera.ModifierList.Length; ModifierIdx++ )
	{
		// If we found ourselves, remove ourselves from the list and return
		if( Camera.ModifierList[ModifierIdx] == self )
		{
			Camera.ModifierList.Remove(ModifierIdx, 1);
			return true;
		}
	}

	// Didn't find ourselves in the list
	return false;
}

/** Accessor functions for changing disable flag */
event DisableModifier()
{
	//debug
	`Log( self@"DisableModifier", bDebug );

	bDisabled = true;
	bPendingDisable = false;
}
function EnableModifier()
{
	//debug
	`Log( self@"EnableModifier", bDebug );

	bDisabled = false;
	bPendingDisable = false;
}
function ToggleModifier()
{
	//debug
	`Log( self@"ToggleModifier", bDebug );

	if( bDisabled )
	{
		EnableModifier();
	}
	else
	{
		DisableModifier();
	}
}
/**
 * Allow this modifier a chance to change view rotation and deltarot
 * Default just returns ViewRotation unchanged
 * @return	bool - TRUE if should stop looping modifiers to adjust rotation, FALSE otherwise
 */
simulated function bool ProcessViewRotation( Actor ViewTarget, float DeltaTime, out Rotator out_ViewRotation, out Rotator out_DeltaRot );

defaultproperties
{
}
