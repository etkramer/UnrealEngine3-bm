/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class MaterialExpressionSine extends MaterialExpression
	native(Material)
	collapsecategories
	hidecategories(Object);

var ExpressionInput	Input;

var() float	Period;

cpptext
{
	virtual INT Compile(FMaterialCompiler* Compiler);
	virtual FString GetCaption() const;

	/**
	 * Replaces references to the passed in expression with references to a different expression or NULL.
	 * @param	OldExpression		Expression to find reference to.
	 * @param	NewExpression		Expression to replace reference with.
	 */
	virtual void SwapReferenceTo(UMaterialExpression* OldExpression,UMaterialExpression* NewExpression = NULL);
}

defaultproperties
{
	Period=1.f
	MenuCategories(0)="Math"
}
