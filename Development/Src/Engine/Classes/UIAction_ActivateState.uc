/**
 * This action changes (or appends) the UIState specified by NewState for the widgets associated
 * with the Target variable link.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_ActivateState extends UIAction_ChangeState
	native(inherit);

cpptext
{
	/**
	 * Activates the state associated with this action for all targets.  If any of the targets cannot change states,
	 * disables all output links.
	 */
	virtual void Activated();
}

DefaultProperties
{
	ObjName="Activate UI State"
}
