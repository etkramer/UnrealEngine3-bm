/**
 * This action changes (or removes) the UIState specified by StateName for the widgets associated
 * with the Target variable link.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_DeactivateState extends UIAction_ChangeState
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
	ObjName="Deactivate UI State"
}
