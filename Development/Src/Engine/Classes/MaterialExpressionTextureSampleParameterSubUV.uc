/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class MaterialExpressionTextureSampleParameterSubUV extends MaterialExpressionTextureSampleParameter2D
	native(Material)
	collapsecategories
	hidecategories(Object);

cpptext
{
	virtual INT Compile(FMaterialCompiler* Compiler);
	virtual FString GetCaption() const;
	virtual UBOOL TextureIsValid( UTexture* InTexture );
	virtual const TCHAR* GetRequirements();
}

defaultproperties
{
	MenuCategories(0)="Texture"
	MenuCategories(1)="Parameters"
	MenuCategories(2)="Particles"
}
