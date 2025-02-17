/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTSkeletalMeshComponent extends SkeletalMeshComponent
	native;

cpptext
{
	/** Creates a FUTSkeletalMeshSceneProxy (defined in UTWeapon.cpp) */
	virtual FPrimitiveSceneProxy* CreateSceneProxy();
	virtual void Tick(FLOAT DeltaTime);
}

/** This changes the FOV used for rendering the skeletal mesh component. A value of 0 means to use the default. */
var() const float FOV;

/** whether textures are currently forced loaded */
var		bool		bForceLoadTextures;

/** when to clear forced streaming */
var		float		ClearStreamingTime;

/**
  * Force streamed textures to be loaded.  Used to get MIPS streamed in before weapon comes up
  * @PARAM bForcePreload if true causes streamed textures to be force loaded, if false, clears force loading
  */
simulated event PreloadTextures(bool bForcePreload, float ClearTime)
{
	local int i, idx;
	local array<texture> Textures;

	bForceLoadTextures = bForcePreload;
	ClearStreamingTime = ClearTime;

	for (Idx = 0; Idx < Materials.Length; Idx++)
	{
		if (Materials[Idx] != None)
		{
			Textures = Materials[Idx].GetMaterial().GetTextures();

			for (i = 0; i < Textures.Length; i++)
			{
				if ( Texture2D(Textures[i]) != None )
				{
					Texture2D(Textures[i]).bForceMiplevelsToBeResident = bForcePreload;
				}
			}
		}
	}
}

/** changes the value of FOV */
native final function SetFOV(float NewFOV);

DefaultProperties
{
	FOV=0.0
}
