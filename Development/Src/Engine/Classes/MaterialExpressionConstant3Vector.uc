/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class MaterialExpressionConstant3Vector extends MaterialExpression
	native(Material)
	collapsecategories
	hidecategories(Object);

var const LinearColor Colour;
var() float	R,
			G,
			B;

cpptext
{
	virtual INT Compile(FMaterialCompiler* Compiler);
	virtual FString GetCaption() const;
	virtual void Serialize(FArchive& Ar);
}

defaultproperties
{
	MenuCategories(0)="Constants"
	MenuCategories(1)="Vectors"
}
