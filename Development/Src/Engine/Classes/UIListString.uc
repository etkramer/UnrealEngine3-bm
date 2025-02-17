/**
 * This specialized version of UIString is used in list cells.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIListString extends UIString
	within UIList
	native(UIPrivate);

cpptext
{
	/* === UIListString interface === */
	void SetValue( struct FUIStringNode* ValueNode );
}

DefaultProperties
{

}
