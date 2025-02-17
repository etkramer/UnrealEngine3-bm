/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class MaterialExpressionTextureSample extends MaterialExpression
	native(Material)
	collapsecategories
	hidecategories(Object);

var() Texture		Texture;
var ExpressionInput	Coordinates;

cpptext
{
	virtual INT Compile(FMaterialCompiler* Compiler);
	virtual void GetOutputs(TArray<FExpressionOutput>& Outputs) const;
	virtual INT GetWidth() const;
	virtual FString GetCaption() const;
	virtual INT GetLabelPadding() { return 8; }

	/**
	 * Updates the material's cached reference to the resource for a given texture.
	 * @param Texture - The UTexture which has a new FTexture.
	 */
	void UpdateTextureResource(class UTexture* Texture);

	/**
	 * Replaces references to the passed in expression with references to a different expression or NULL.
	 * @param	OldExpression		Expression to find reference to.
	 * @param	NewExpression		Expression to replace reference with.
	 */
	virtual void SwapReferenceTo(UMaterialExpression* OldExpression,UMaterialExpression* NewExpression = NULL);
}

defaultproperties
{
	bRealtimePreview=True
	MenuCategories(0)="Texture"
}
