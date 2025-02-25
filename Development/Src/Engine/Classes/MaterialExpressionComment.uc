/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class MaterialExpressionComment extends MaterialExpression
	native(Material);

var int		PosX;
var int		PosY;
var int		SizeX;
var int		SizeY;
var() string	Text;

cpptext
{
	/**
	 * Text description of this expression.
	 */
	virtual FString GetCaption() const;
}

defaultproperties
{
	MenuCategories(0)="Utility"
}
