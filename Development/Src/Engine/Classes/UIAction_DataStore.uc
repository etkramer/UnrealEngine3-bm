/**
 * Base class for all actions that manipulate data store bindings.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_DataStore extends UIAction
	native(inherit)
	abstract
	DependsOn(UIRoot);

/**
 * For widgets that support multiple data store bindings, indicates which data store this action is associated with
 * A value of -1 indicates that this action should operate on all data store bindings.
 */
var()	int			BindingIndex;

DefaultProperties
{
	ObjName="Data Store"
	ObjCategory="Data Store"

	bAutoTargetOwner=true

	BindingIndex=INDEX_NONE
}
