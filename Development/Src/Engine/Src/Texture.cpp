/*=============================================================================
	Texture.cpp: Implementation of UTexture.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

IMPLEMENT_CLASS(UTexture);

void UTexture::ReleaseResource()
{
	check(Resource);

	// Free the resource.
	ReleaseResourceAndFlush(Resource);
	delete Resource;
	Resource = NULL;
}

void UTexture::UpdateResource()
{
	if(Resource)
	{
		// Release the existing texture resource.
		ReleaseResource();
	}

	if( !GIsUCC && !HasAnyFlags(RF_ClassDefaultObject) )
	{
		// Create a new texture resource.
		Resource = CreateResource();
		if( Resource )
		{
			BeginInitResource(Resource);
		}
	}
}

void UTexture::PreEditChange(UProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);
	if( Resource )
	{
		ReleaseResource();
	}
}

void UTexture::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);

	// Determine whether any property that requires recompression of the texture has changed.
	UBOOL RequiresRecompression = 0;
	if( PropertyThatChanged )
	{
		FString PropertyName = *PropertyThatChanged->GetName();

		if( appStricmp( *PropertyName, TEXT("RGBE")							) == 0
		||	appStricmp( *PropertyName, TEXT("CompressionNoAlpha")			) == 0
		||	appStricmp( *PropertyName, TEXT("CompressionNone")				) == 0
		||	appStricmp( *PropertyName, TEXT("CompressionNoMipmaps")			) == 0
		||	appStricmp( *PropertyName, TEXT("CompressionFullDynamicRange")	) == 0
		||	appStricmp( *PropertyName, TEXT("CompressionSettings")			) == 0
		||	appStricmp( *PropertyName, TEXT("bDitherMipMapAlpha")			) == 0
		||	appStricmp( *PropertyName, TEXT("bPreserveBorderR")				) == 0
		||	appStricmp( *PropertyName, TEXT("bPreserveBorderG")				) == 0
		||	appStricmp( *PropertyName, TEXT("bPreserveBorderB")				) == 0
		||	appStricmp( *PropertyName, TEXT("bPreserveBorderA")				) == 0
		)
		{
			RequiresRecompression = 1;
		}
	}
	else
	{
		RequiresRecompression = 1;
	}

	// Only compress when we really need to to avoid lag when level designers/ artists manipulate properties like clamping in the editor.
	if (RequiresRecompression)
	{
		UBOOL CompressionNoneSave = CompressionNone;
		if (!(
			(CompressionSettings == TC_Default)	||
			(CompressionSettings == TC_Normalmap) || 
			(CompressionSettings == TC_NormalmapAlpha)
			))
		{
			DeferCompression = FALSE;
		}

		if (DeferCompression)
		{
			CompressionNone = TRUE;
		}
		Compress();
		if (DeferCompression)
		{
			CompressionNone = CompressionNoneSave;
		}
	}

	// Update cached LOD bias.
	CachedCombinedLODBias = GSystemSettings.TextureLODSettings.CalculateLODBias( this );

	// Recreate the texture's resource.
	UpdateResource();
}

void UTexture::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SourceArt.Serialize( Ar, this );
}

void UTexture::PostLoad()
{
	Super::PostLoad();

	// High dynamic range textures are currently always stored as RGBE (shared exponent) textures.
	// We explicitely set this here as older versions of the engine didn't correctly update the RGBE field.
	// @todo: Ensures that RGBE is correctly set to work around a bug in older versions of the engine.
	RGBE = (CompressionSettings == TC_HighDynamicRange);

	if( !IsTemplate() )
	{
		// Update cached LOD bias.
		CachedCombinedLODBias = GSystemSettings.TextureLODSettings.CalculateLODBias( this );

		// The texture will be cached by the cubemap it is contained within on consoles.
		UTextureCube* CubeMap = Cast<UTextureCube>(GetOuter());
		if (CubeMap == NULL)
		{
			// Recreate the texture's resource.
			UpdateResource();
		}
	}
}

void UTexture::BeginDestroy()
{
	Super::BeginDestroy();
	if( !UpdateStreamingStatus() && Resource )
	{
		// Send the rendering thread a release message for the texture's resource.
		BeginReleaseResource(Resource);
		Resource->ReleaseFence.BeginFence();
		// Keep track that we alrady kicked off the async release.
		bAsyncResourceReleaseHasBeenStarted = TRUE;
	}
}

UBOOL UTexture::IsReadyForFinishDestroy()
{
	UBOOL bReadyForFinishDestroy = FALSE;
	// Check whether super class is ready and whether we have any pending streaming requests in flight.
	if( Super::IsReadyForFinishDestroy() && !UpdateStreamingStatus() )
	{
		// Kick off async resource release if we haven't already.
		if( !bAsyncResourceReleaseHasBeenStarted && Resource )
		{
			// Send the rendering thread a release message for the texture's resource.
			BeginReleaseResource(Resource);
			Resource->ReleaseFence.BeginFence();
			// Keep track that we alrady kicked off the async release.
			bAsyncResourceReleaseHasBeenStarted = TRUE;
		}
		// Only allow FinishDestroy to be called once the texture resource has finished its rendering thread cleanup.
		else if( !Resource || !Resource->ReleaseFence.GetNumPendingFences() )
		{
			bReadyForFinishDestroy = TRUE;
		}
	}
	return bReadyForFinishDestroy;
}

void UTexture::FinishDestroy()
{
	Super::FinishDestroy();

	if(Resource)
	{
		check(!Resource->ReleaseFence.GetNumPendingFences());

		// Free the resource.
		delete Resource;
		Resource = NULL;
	}
}

void UTexture::PreSave()
{
	Super::PreSave();

	// If "no compress" is set, don't do it...
	if (CompressionNone)
	{
		return;
	}

	// Otherwise, if we are not already compressed, do it now.
	if (DeferCompression)
	{
		Compress();
		DeferCompression = false;
	}
}

/**
 * Used by various commandlets to purge Editor only data from the object.
 * 
 * @param TargetPlatform Platform the object will be saved for (ie PC vs console cooking, etc)
 */
void UTexture::StripData(UE3::EPlatformType TargetPlatform)
{
	Super::StripData(TargetPlatform);
	// Remove source art.
	SourceArt.RemoveBulkData();
}

/** Helper functions for text output of texture properties... */
#ifndef CASE_TEXT
#define CASE_TEXT(txt) case txt: return TEXT(#txt)
#endif

#ifndef TEXT_TO_ENUM
#define TEXT_TO_ENUM(eVal, txt)		if (appStricmp(TEXT(#eVal), txt) == 0)	return eVal;
#endif

const TCHAR* UTexture::GetCompressionSettingsString(TextureCompressionSettings InCompressionSettings)
{
	switch (InCompressionSettings)
	{
	CASE_TEXT(TC_Normalmap);
	CASE_TEXT(TC_Displacementmap);
	CASE_TEXT(TC_NormalmapAlpha);
	CASE_TEXT(TC_Grayscale);
	CASE_TEXT(TC_HighDynamicRange);
	}

	return TEXT("TC_Default");
}

TextureCompressionSettings UTexture::GetCompressionSettingsFromString(const TCHAR* InCompressionSettingsStr)

{
	TEXT_TO_ENUM(TC_Normalmap, InCompressionSettingsStr)
	TEXT_TO_ENUM(TC_Displacementmap, InCompressionSettingsStr)
	TEXT_TO_ENUM(TC_NormalmapAlpha, InCompressionSettingsStr)
	TEXT_TO_ENUM(TC_Grayscale, InCompressionSettingsStr)
	TEXT_TO_ENUM(TC_HighDynamicRange, InCompressionSettingsStr)

	return TC_Default;
}

const TCHAR* UTexture::GetPixelFormatString(EPixelFormat InPixelFormat)
{
	switch (InPixelFormat)
	{
	CASE_TEXT(PF_A32B32G32R32F);
	CASE_TEXT(PF_A8R8G8B8);
	CASE_TEXT(PF_G8);
	CASE_TEXT(PF_G16);
	CASE_TEXT(PF_DXT1);
	CASE_TEXT(PF_DXT3);
	CASE_TEXT(PF_DXT5);
	CASE_TEXT(PF_UYVY);
	CASE_TEXT(PF_FloatRGB);
	CASE_TEXT(PF_FloatRGBA);
	CASE_TEXT(PF_DepthStencil);
	CASE_TEXT(PF_ShadowDepth);
	CASE_TEXT(PF_FilteredShadowDepth);
	CASE_TEXT(PF_R32F);
	CASE_TEXT(PF_G16R16);
	CASE_TEXT(PF_G16R16F);
	CASE_TEXT(PF_G16R16F_FILTER);
	CASE_TEXT(PF_G32R32F);
	CASE_TEXT(PF_A2B10G10R10);
	CASE_TEXT(PF_A16B16G16R16);
	CASE_TEXT(PF_D24);
	CASE_TEXT(PF_R16F);
	CASE_TEXT(PF_R16F_FILTER);
	}
	return TEXT("PF_Unknown");
}

EPixelFormat UTexture::GetPixelFormatFromString(const TCHAR* InPixelFormatStr)
{
	TEXT_TO_ENUM(PF_A32B32G32R32F, InPixelFormatStr);
	TEXT_TO_ENUM(PF_A8R8G8B8, InPixelFormatStr);
	TEXT_TO_ENUM(PF_G8, InPixelFormatStr);
	TEXT_TO_ENUM(PF_G16, InPixelFormatStr);
	TEXT_TO_ENUM(PF_DXT1, InPixelFormatStr);
	TEXT_TO_ENUM(PF_DXT3, InPixelFormatStr);
	TEXT_TO_ENUM(PF_DXT5, InPixelFormatStr);
	TEXT_TO_ENUM(PF_UYVY, InPixelFormatStr);
	TEXT_TO_ENUM(PF_FloatRGB, InPixelFormatStr);
	TEXT_TO_ENUM(PF_FloatRGBA, InPixelFormatStr);
	TEXT_TO_ENUM(PF_DepthStencil, InPixelFormatStr);
	TEXT_TO_ENUM(PF_ShadowDepth, InPixelFormatStr);
	TEXT_TO_ENUM(PF_FilteredShadowDepth, InPixelFormatStr);
	TEXT_TO_ENUM(PF_R32F, InPixelFormatStr);
	TEXT_TO_ENUM(PF_G16R16, InPixelFormatStr);
	TEXT_TO_ENUM(PF_G16R16F, InPixelFormatStr);
	TEXT_TO_ENUM(PF_G16R16F_FILTER, InPixelFormatStr);
	TEXT_TO_ENUM(PF_G32R32F, InPixelFormatStr);
	TEXT_TO_ENUM(PF_A2B10G10R10, InPixelFormatStr);
	TEXT_TO_ENUM(PF_A16B16G16R16, InPixelFormatStr);
	TEXT_TO_ENUM(PF_D24, InPixelFormatStr);
	TEXT_TO_ENUM(PF_R16F, InPixelFormatStr);
	TEXT_TO_ENUM(PF_R16F_FILTER, InPixelFormatStr);

	return PF_Unknown;
}

const TCHAR* UTexture::GetTextureFilterString(TextureFilter InFilter)
{
	switch (InFilter)
	{
    CASE_TEXT(TF_Nearest);
    CASE_TEXT(TF_Linear);
	}
	return TEXT("TF_Nearest");
}

TextureFilter UTexture::GetTextureFilterFromString(const TCHAR* InFilterStr)
{
	TEXT_TO_ENUM(TF_Linear, InFilterStr);
	
	return TF_Nearest;
}

const TCHAR* UTexture::GetTextureAddressString(TextureAddress InAddress)
{
	switch (InAddress)
	{
	CASE_TEXT(TA_Clamp);
	CASE_TEXT(TA_Mirror);
	}

	return TEXT("TA_Wrap");
}
TextureAddress UTexture::GetTextureAddressFromString(const TCHAR* InAddressStr)
{
	TEXT_TO_ENUM(TA_Clamp, InAddressStr);
	TEXT_TO_ENUM(TA_Mirror, InAddressStr);

	return TA_Wrap;
}

const TCHAR* UTexture::GetTextureGroupString(TextureGroup InGroup)
{
	switch (InGroup)
	{
	CASE_TEXT(TEXTUREGROUP_WorldNormalMap);
	CASE_TEXT(TEXTUREGROUP_WorldSpecular);
	CASE_TEXT(TEXTUREGROUP_Character);
	CASE_TEXT(TEXTUREGROUP_CharacterNormalMap);
	CASE_TEXT(TEXTUREGROUP_CharacterSpecular);
	CASE_TEXT(TEXTUREGROUP_Weapon);
	CASE_TEXT(TEXTUREGROUP_WeaponNormalMap);
	CASE_TEXT(TEXTUREGROUP_WeaponSpecular);
	CASE_TEXT(TEXTUREGROUP_Vehicle);
	CASE_TEXT(TEXTUREGROUP_VehicleNormalMap);
	CASE_TEXT(TEXTUREGROUP_VehicleSpecular);
	CASE_TEXT(TEXTUREGROUP_Cinematic);
	CASE_TEXT(TEXTUREGROUP_Effects);
	CASE_TEXT(TEXTUREGROUP_Skybox);
	CASE_TEXT(TEXTUREGROUP_UI);
	CASE_TEXT(TEXTUREGROUP_LightAndShadowMap);
	CASE_TEXT(TEXTUREGROUP_RenderTarget);
	}

	return TEXT("TEXTUREGROUP_World");
}

TextureGroup UTexture::GetTextureGroupFromString(const TCHAR* InGroupStr)
{
	TEXT_TO_ENUM(TEXTUREGROUP_WorldNormalMap, InGroupStr);
	TEXT_TO_ENUM(TEXTUREGROUP_WorldSpecular, InGroupStr);
	TEXT_TO_ENUM(TEXTUREGROUP_Character, InGroupStr);
	TEXT_TO_ENUM(TEXTUREGROUP_CharacterNormalMap, InGroupStr);
	TEXT_TO_ENUM(TEXTUREGROUP_CharacterSpecular, InGroupStr);
	TEXT_TO_ENUM(TEXTUREGROUP_Weapon, InGroupStr);
	TEXT_TO_ENUM(TEXTUREGROUP_WeaponNormalMap, InGroupStr);
	TEXT_TO_ENUM(TEXTUREGROUP_WeaponSpecular, InGroupStr);
	TEXT_TO_ENUM(TEXTUREGROUP_Vehicle, InGroupStr);
	TEXT_TO_ENUM(TEXTUREGROUP_VehicleNormalMap, InGroupStr);
	TEXT_TO_ENUM(TEXTUREGROUP_VehicleSpecular, InGroupStr);
	TEXT_TO_ENUM(TEXTUREGROUP_Cinematic, InGroupStr);
	TEXT_TO_ENUM(TEXTUREGROUP_Effects, InGroupStr);
	TEXT_TO_ENUM(TEXTUREGROUP_Skybox, InGroupStr);
	TEXT_TO_ENUM(TEXTUREGROUP_UI, InGroupStr);
	TEXT_TO_ENUM(TEXTUREGROUP_LightAndShadowMap, InGroupStr);
	TEXT_TO_ENUM(TEXTUREGROUP_RenderTarget, InGroupStr);

	return TEXTUREGROUP_World;
}

/**
 * Initializes LOD settings by reading them from the passed in filename/ section.
 *
 * @param	IniFilename		Filename of ini to read from.
 * @param	IniSection		Section in ini to look for settings
 */
void FTextureLODSettings::Initialize( const TCHAR* IniFilename, const TCHAR* IniSection )
{
	// Read individual entries. This must be updated whenever new entries are added to the enumeration.
	ReadEntry( TEXTUREGROUP_World				, TEXT("TEXTUREGROUP_World")				, IniFilename, IniSection );
	ReadEntry( TEXTUREGROUP_WorldNormalMap		, TEXT("TEXTUREGROUP_WorldNormalMap")		, IniFilename, IniSection );
	ReadEntry( TEXTUREGROUP_WorldSpecular		, TEXT("TEXTUREGROUP_WorldSpecular")		, IniFilename, IniSection );
	ReadEntry( TEXTUREGROUP_Character			, TEXT("TEXTUREGROUP_Character")			, IniFilename, IniSection );
	ReadEntry( TEXTUREGROUP_CharacterNormalMap	, TEXT("TEXTUREGROUP_CharacterNormalMap")	, IniFilename, IniSection );
	ReadEntry( TEXTUREGROUP_CharacterSpecular	, TEXT("TEXTUREGROUP_CharacterSpecular")	, IniFilename, IniSection );
	ReadEntry( TEXTUREGROUP_Weapon				, TEXT("TEXTUREGROUP_Weapon")				, IniFilename, IniSection );
	ReadEntry( TEXTUREGROUP_WeaponNormalMap		, TEXT("TEXTUREGROUP_WeaponNormalMap")		, IniFilename, IniSection );
	ReadEntry( TEXTUREGROUP_WeaponSpecular		, TEXT("TEXTUREGROUP_WeaponSpecular")		, IniFilename, IniSection );
	ReadEntry( TEXTUREGROUP_Vehicle				, TEXT("TEXTUREGROUP_Vehicle")				, IniFilename, IniSection );
	ReadEntry( TEXTUREGROUP_VehicleNormalMap	, TEXT("TEXTUREGROUP_VehicleNormalMap")		, IniFilename, IniSection );
	ReadEntry( TEXTUREGROUP_VehicleSpecular		, TEXT("TEXTUREGROUP_VehicleSpecular")		, IniFilename, IniSection );
	ReadEntry( TEXTUREGROUP_Cinematic			, TEXT("TEXTUREGROUP_Cinematic")			, IniFilename, IniSection );
	ReadEntry( TEXTUREGROUP_Effects				, TEXT("TEXTUREGROUP_Effects")				, IniFilename, IniSection );
	ReadEntry( TEXTUREGROUP_EffectsNotFiltered	, TEXT("TEXTUREGROUP_EffectsNotFiltered")	, IniFilename, IniSection );
	ReadEntry( TEXTUREGROUP_Skybox				, TEXT("TEXTUREGROUP_Skybox")				, IniFilename, IniSection );
	ReadEntry( TEXTUREGROUP_UI					, TEXT("TEXTUREGROUP_UI")					, IniFilename, IniSection );
	ReadEntry( TEXTUREGROUP_LightAndShadowMap	, TEXT("TEXTUREGROUP_LightAndShadowMap")	, IniFilename, IniSection );
	ReadEntry( TEXTUREGROUP_RenderTarget		, TEXT("TEXTUREGROUP_RenderTarget")			, IniFilename, IniSection );
}

/**
 * Returns the texture group names, sorted like enum.
 *
 * @return array of texture group names
 */
TArray<FString> FTextureLODSettings::GetTextureGroupNames()
{
	// Order is important!
	TArray<FString> TextureGroupNames;
	new(TextureGroupNames) FString(TEXT("World"));
	new(TextureGroupNames) FString(TEXT("WorldNormalMap"));
	new(TextureGroupNames) FString(TEXT("WorldSpecular"));
	new(TextureGroupNames) FString(TEXT("Character"));
	new(TextureGroupNames) FString(TEXT("CharacterNormalMap"));
	new(TextureGroupNames) FString(TEXT("CharacterSpecular"));
	new(TextureGroupNames) FString(TEXT("Weapon"));
	new(TextureGroupNames) FString(TEXT("WeaponNormalMap"));
	new(TextureGroupNames) FString(TEXT("WeaponSpecular"));
	new(TextureGroupNames) FString(TEXT("Vehicle"));
	new(TextureGroupNames) FString(TEXT("VehicleNormalMap"));
	new(TextureGroupNames) FString(TEXT("VehicleSpecular"));
	new(TextureGroupNames) FString(TEXT("Cinematic"));
	new(TextureGroupNames) FString(TEXT("Effects"));
	new(TextureGroupNames) FString(TEXT("EffectsNotFiltered"));
	new(TextureGroupNames) FString(TEXT("Skybox"));
	new(TextureGroupNames) FString(TEXT("UI"));
	new(TextureGroupNames) FString(TEXT("LightAndShadowMap"));
	new(TextureGroupNames) FString(TEXT("RenderTarget"));
	return TextureGroupNames;
}

/**
 * Reads a single entry and parses it into the group array.
 *
 * @param	GroupId			Id/ enum of group to parse
 * @param	GroupName		Name of group to look for in ini
 * @param	IniFilename		Filename of ini to read from.
 * @param	IniSection		Section in ini to look for settings
 */
void FTextureLODSettings::ReadEntry( INT GroupId, const TCHAR* GroupName, const TCHAR* IniFilename, const TCHAR* IniSection )
{
	// Look for string in filename/ section.
	FString Entry;
	if( GConfig->GetString( IniSection, GroupName, Entry, IniFilename ) )
	{
		// Trim whitespace at the beginning.
		Entry = Entry.Trim();
		// Remove brackets.
		Entry = Entry.Replace( TEXT("("), TEXT("") );
		Entry = Entry.Replace( TEXT(")"), TEXT("") );
		
		// Parse minimum LOD mip count.
		INT	MinLODSize = 0;
		if( Parse( *Entry, TEXT("MinLODSize="), MinLODSize ) )
		{
			TextureLODGroups[GroupId].MinLODMipCount = appCeilLogTwo( MinLODSize );
		}

		// Parse maximum LOD mip count.
		INT MaxLODSize = 0;
		if( Parse( *Entry, TEXT("MaxLODSize="), MaxLODSize ) )
		{
			TextureLODGroups[GroupId].MaxLODMipCount = appCeilLogTwo( MaxLODSize );
		}

		// Parse LOD bias.
		INT LODBias = 0;
		if( Parse( *Entry, TEXT("LODBias="), LODBias ) )
		{
			TextureLODGroups[GroupId].LODBias = LODBias;
		}

		// Parse min/map/mip filter names.
		FName MinMagFilter = NAME_Aniso;
		Parse( *Entry, TEXT("MinMagFilter="), MinMagFilter );
		FName MipFilter = NAME_Linear;
		Parse( *Entry, TEXT("MipFilter="), MipFilter );

		// Convert into single filter enum. The code is layed out such that invalid input will 
		// map to the default state of highest quality filtering.

		// Linear filtering
		if( MinMagFilter == NAME_Linear )
		{
			if( MipFilter == NAME_Point )
			{
				TextureLODGroups[GroupId].Filter = SF_Bilinear;
			}
			else
			{
				TextureLODGroups[GroupId].Filter = SF_Trilinear;
			}
		}
		// Point. Don't even care about mip filter.
		else if( MinMagFilter == NAME_Point )
		{
			TextureLODGroups[GroupId].Filter = SF_Point;
		}
		// Aniso or unknown.
		else
		{
			if( MipFilter == NAME_Point )
			{
				TextureLODGroups[GroupId].Filter = SF_AnisotropicPoint;
			}
			else
			{
				TextureLODGroups[GroupId].Filter = SF_AnisotropicLinear;
			}
		}
	}
}

/**
 * Calculates and returns the LOD bias based on texture LOD group, LOD bias and maximum size.
 *
 * @param	Texture		Texture object to calculate LOD bias for.
 * @return	LOD bias
 */
INT FTextureLODSettings::CalculateLODBias( UTexture* Texture ) const
{	
	// Find LOD group.
	check( Texture );
	const FTextureLODGroup& LODGroup = TextureLODGroups[Texture->LODGroup];

	// Calculate maximum number of miplevels.
	const INT TextureMipCount	= appCeilLogTwo( appTrunc( Max( Texture->GetSurfaceWidth(), Texture->GetSurfaceHeight() ) ) );
	// Calculate LOD bias.
	INT UsedLODBias		= LODGroup.LODBias + Texture->LODBias;
	UsedLODBias			= TextureMipCount - Clamp( TextureMipCount - UsedLODBias, LODGroup.MinLODMipCount, LODGroup.MaxLODMipCount );

	// Return clamped LOD bias; we never increase detail.
	return Max( UsedLODBias, 0 );
}

/**
 * Will return the LODBias for a passed in LODGroup
 *
 * @param	InLODGroup		The LOD Group ID 
 * @return	LODBias
 */
INT FTextureLODSettings::GetTextureLODGroupLODBias( INT InLODGroup ) const
{
	INT Retval = 0;

	const FTextureLODGroup& LODGroup = TextureLODGroups[InLODGroup]; 

	Retval = LODGroup.LODBias;

	return Retval;
}

/**
 * Returns the filter state that should be used for the passed in texture, taking
 * into account other system settings.
 *
 * @param	Texture		Texture to retrieve filter state for
 * @return	Filter sampler state for passed in texture
 */
ESamplerFilter FTextureLODSettings::GetSamplerFilter( const UTexture* Texture ) const
{
	// Default to point filtering.
	ESamplerFilter Filter = SF_Point;

	// Only diverge from default for valid textures that don't use point filtering.
	if( !Texture || Texture->Filter != TF_Nearest )
	{
		// Use LOD group value to find proper filter setting.
		Filter = TextureLODGroups[Texture->LODGroup].Filter;
	}

	return Filter;
}



