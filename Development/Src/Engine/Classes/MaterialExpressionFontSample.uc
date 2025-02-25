/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class MaterialExpressionFontSample extends MaterialExpression
	native(Material)
	collapsecategories
	hidecategories(Object);

/** font resource that will be sampled */
var() Font Font;
/** allow access to the various font pages */
var() int FontTexturePage;

cpptext
{
	/** 
	* Generate the compiled material string for this expression
	* @param Compiler - shader material compiler
	* @return index to the new generated expression
	*/
	virtual INT Compile(FMaterialCompiler* Compiler);

	/**
	* List of outputs from this expression
	* @param Outputs - out list of expression
	*/
	virtual void GetOutputs(TArray<FExpressionOutput>& Outputs) const;

	/**
	* Width of the thumbnail for this expression int he material editor
	* @return size in pixels
	*/
	virtual INT GetWidth() const;

	/**
	* Caption description for this expression
	* @return string caption
	*/
	virtual FString GetCaption() const;

	/**
	* Padding for the text lable
	* @return size in pixels
	*/
	virtual INT GetLabelPadding() { return 8; }
}

defaultproperties
{
	bRealtimePreview=True
	MenuCategories(0)="Font"
	MenuCategories(1)="Texture"
}
