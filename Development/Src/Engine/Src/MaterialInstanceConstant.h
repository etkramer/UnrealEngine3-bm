/*=============================================================================
	MaterialInstanceConstant.h: MaterialInstanceConstant definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __MATERIALINSTANCECONSTANT_H__
#define __MATERIALINSTANCECONSTANT_H__

#include "MaterialInstance.h"

class FMaterialInstanceConstantResource : public FMaterialInstanceResource
{
public:

	typedef UMaterialInstanceConstant InstanceType;

	friend class MICVectorParameterMapping;
	friend class MICScalarParameterMapping;
	friend class MICTextureParameterMapping;
	friend class MICFontParameterMapping;

	/** Initialization constructor. */
	FMaterialInstanceConstantResource(UMaterialInstance* InOwner,UBOOL bInSelected)
	:	FMaterialInstanceResource( InOwner, bInSelected )
	{
	}

	// FMaterialRenderProxy interface.
	virtual UBOOL GetVectorValue(const FName& ParameterName, FLinearColor* OutValue, const FMaterialRenderContext& Context) const;
	virtual UBOOL GetScalarValue(const FName& ParameterName,FLOAT* OutValue, const FMaterialRenderContext& Context) const;
	virtual UBOOL GetTextureValue(const FName& ParameterName,const FTexture** OutValue, const FMaterialRenderContext& Context) const;

private:

	TMap<FName,FLinearColor> VectorParameterMap;
	TMap<FName,FLOAT> ScalarParameterMap;
	TMap<FName,const UTexture*> TextureParameterMap;
};

#endif
