/*=============================================================================
	HModel.h: HModel definition.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _INC_HMODEL
#define _INC_HMODEL

/**
 * A hit proxy representing a UModel.
 */
class HModel : public HHitProxy
{
	DECLARE_HIT_PROXY(HModel,HHitProxy);
public:

	/** Initialization constructor. */
	HModel(UModel* InModel):
		Model(InModel)
	{}

	/**
	 * Finds the surface at the given screen coordinates of a view family.
	 * @return True if a surface was found.
	 */
	UBOOL ResolveSurface(const FSceneView* View,INT X,INT Y,UINT& OutSurfaceIndex) const;

	// HHitProxy interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << Model;
	}
	virtual EMouseCursor GetMouseCursor()
	{
		return MC_Cross;
	}

	// Accessors.
	UModel* GetModel() const { return Model; }

private:
	
	UModel* Model;
};

#endif
