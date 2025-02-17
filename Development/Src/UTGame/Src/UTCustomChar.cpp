/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "UTGame.h"
#include "UnSkeletalMeshMerge.h"
#include "EngineMaterialClasses.h"
#include "EngineAnimClasses.h"
#include "UnNet.h"

IMPLEMENT_CLASS(UUTCharInfo);
IMPLEMENT_CLASS(UUTCharFamilyAssetStore);
IMPLEMENT_CLASS(UUTFamilyInfo);

/** Given a faction and character name, find the profile that defines all its parts. */
FCharacterInfo UUTCharInfo::FindCharacter(const FString& InFaction, const FString& InCharID)
{
	//debugf(TEXT("Checking %d Profiles for %s"), Profiles.Num(), *InCharName);

	for(INT i=0; i<Characters.Num(); i++)
	{
		FCharacterInfo& TestChar = Characters(i);
		if( TestChar.Faction == InFaction &&
			TestChar.CharID == InCharID )
		{
			return TestChar;
		}
	}

	// No match - return empty profile.
	FCharacterInfo EmptyChar;
	appMemzero(&EmptyChar, sizeof(FCharacterInfo));
	return EmptyChar;
}

/** Find the info class for a particular family */
UClass* UUTCharInfo::FindFamilyInfo(const FString& InFamilyID)
{
	for(INT i=0; i<Families.Num(); i++)
	{
		UClass* InfoClass = Families(i);
		if(InfoClass)
		{
			UUTFamilyInfo* Info = CastChecked<UUTFamilyInfo>(InfoClass->GetDefaultObject());
			if(Info->FamilyID == InFamilyID)
			{
				return Families(i);
			}
		}
	}

	return NULL;
}

/** Callback executed when each asset package has finished async loading. */
static void AsyncLoadFamilyAssetsCompletionCallback(UObject* Package, void* InAssetStore)
{
	UUTCharFamilyAssetStore* Store = (UUTCharFamilyAssetStore*)InAssetStore;

	debugf(TEXT("Family Asset Package Loaded: %s"), *Package->GetName());
	(Store->NumPendingPackages)--;

	if (!(Cast<UPackage>(Package)->PackageFlags & PKG_ServerSideOnly))
	{
		debugf(NAME_Error, TEXT("Character creation package '%s' doesn't have ServerSideOnly flag set. Set this using the 'SetPackageFlags' commandlet."), *Package->GetName());
	}

	// Add all objects within this package to the asset store, to stop them from being GC'd
	for( TObjectIterator<UObject> It; It; ++It )
	{
		UObject* Obj = *It;
		if (Obj->IsIn(Package))
		{
			Store->FamilyAssets.AddItem(Obj);
		}
	}

	// If we finished loading, print how long it took.
	if(GWorld && Store->NumPendingPackages == 0)
	{
		debugf(TEXT("CONSTRUCTIONING: LoadFamilyAsset (%s) Took: %3.2f secs"), *Store->FamilyID, appSeconds() - Store->StartLoadTime);
	}

#if WITH_UE3_NETWORKING
	// in a network game ping the other side by sending empty control channel bunches
	// so that if we're doing a blocking load we don't get disconnected due to connection timeout
	if (GWorld != NULL && GWorld->GetNetDriver() != NULL)
	{
		UNetDriver* NetDriver = GWorld->GetNetDriver();
		if (NetDriver->ServerConnection != NULL)
		{
			if (NetDriver->ServerConnection->Channels[0] != NULL && !NetDriver->ServerConnection->Channels[0]->Closing)
			{
				FOutBunch Bunch(NetDriver->ServerConnection->Channels[0], FALSE);
				Bunch.bReliable = 0;
				if (!Bunch.IsError())
				{
					NetDriver->ServerConnection->Channels[0]->SendBunch(&Bunch, FALSE);
					NetDriver->ServerConnection->FlushNet();
				}
			}
		}
		else
		{
			for (INT i = 0; i < NetDriver->ClientConnections.Num(); i++)
			{
				UNetConnection* Conn = NetDriver->ClientConnections(i);
				if (Conn != NULL && Conn->Channels[0] != NULL && !Conn->Channels[0]->Closing && Conn->GetUChildConnection() == NULL)
				{
					FOutBunch Bunch(Conn->Channels[0], FALSE);
					Bunch.bReliable = 0;
					if (!Bunch.IsError())
					{
						Conn->Channels[0]->SendBunch(&Bunch, FALSE);
						Conn->FlushNet();
					}
				}
			}
		}
	}
#endif
}

/** 
 *	This function initiates async loading of all the assets for a particular custom character family. 
 *	When UUTCharFamilyAssetStore.NumPendingPackages hits zero, all assets are in place and you can begin creating merged characters.
 *	@param bBlocking	If true, game will block until all assets are loaded.
 *	@param bArms		Load package containing arm mesh for this family
 */
UUTCharFamilyAssetStore* UUTCharInfo::LoadFamilyAssets(const FString& InFamilyID, UBOOL bBlocking, UBOOL bArms)
{
	// Wait for garbage collection to finish purging objects before loading the next family; the previously loaded family may still be
	// consuming memory.  This wait ensures that only one character family is loaded at a time.
	UObject::IncrementalPurgeGarbage(FALSE);

	// First find all packages required by parts of this family
	TArray<FString> FamilyPackageNames;

	if(bArms)
	{
		if(InFamilyID == TEXT("") || InFamilyID == TEXT("NONE"))
		{
			//FamilyPackageNames.AddUniqueItem(DefaultArmMeshPackageName);
			//FamilyPackageNames.AddUniqueItem(DefaultArmSkinPackageName);
		}
		else
		{
			UClass* FamilyInfoClass = FindFamilyInfo(InFamilyID);
			if(FamilyInfoClass)
			{
				UUTFamilyInfo* FamilyInfo = CastChecked<UUTFamilyInfo>(FamilyInfoClass->GetDefaultObject());
				FamilyPackageNames.AddUniqueItem(FamilyInfo->ArmMeshPackageName);
				FamilyPackageNames.AddUniqueItem(FamilyInfo->ArmSkinPackageName);
			}
		}
	}

	// Check there are some packages to load.
	if(FamilyPackageNames.Num() == 0)
	{
		debugf(TEXT("LoadFamilyAssets : No packages found for Family: %s"), *InFamilyID);
		return NULL;
	}

	// Create asset store object
	UUTCharFamilyAssetStore* NewStore = ConstructObject<UUTCharFamilyAssetStore>(UUTCharFamilyAssetStore::StaticClass());
	NewStore->FamilyID = InFamilyID;
	NewStore->NumPendingPackages = FamilyPackageNames.Num();
	NewStore->StartLoadTime = appSeconds();

	//
	debugf(TEXT("Begin Async loading packages for Family '%s':"), *InFamilyID);
	for(INT i=0; i<FamilyPackageNames.Num(); i++)
	{
		debugf(TEXT("- %s"), *(FamilyPackageNames(i)));
		FString Filename;
		FString SFPackageName = FamilyPackageNames(i) + STANDALONE_SEEKFREE_SUFFIX;
		if (GPackageFileCache->FindPackageFile(*SFPackageName, NULL, Filename))
		{
			UObject::LoadPackageAsync(SFPackageName, AsyncLoadFamilyAssetsCompletionCallback, NewStore);
		}
		else
		{
			//@todo: might need to check if file exists? I think LoadPackageAsync() asserts if it doesn't
			UObject::LoadPackageAsync( FamilyPackageNames(i), AsyncLoadFamilyAssetsCompletionCallback, NewStore);
		}
	}

	// If desired, block now until package is loaded.
	if(bBlocking)
	{
		UObject::FlushAsyncLoading();
	}

	return NewStore;
}

FSceneView* CalcPortraitSceneView(FSceneViewFamily* ViewFamily, FLOAT FOV, INT TextureSize)
{
	FMatrix ViewMatrix = FMatrix::Identity;	

	INT X = 0;
	INT Y = 0;
	UINT SizeX = TextureSize;
	UINT SizeY = TextureSize;

	// Take screen percentage option into account if percentage != 100.
	GSystemSettings.ScaleScreenCoords(X,Y,SizeX,SizeY);

	ViewMatrix = ViewMatrix * FMatrix(
			FPlane(0,	0,	1,	0),
			FPlane(1,	0,	0,	0),
			FPlane(0,	1,	0,	0),
			FPlane(0,	0,	0,	1));


	FMatrix ProjectionMatrix = FPerspectiveMatrix(
		FOV * (FLOAT)PI / 360.0f,
		SizeX,
		SizeY,
		NEAR_CLIPPING_PLANE
		);

	FSceneView* View = new FSceneView(
		ViewFamily,
		NULL,
		-1,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		X,
		Y,
		SizeX,
		SizeY,
		ViewMatrix,
		ProjectionMatrix,
		FLinearColor::Black,
		FLinearColor(0,0,0,0),
		FLinearColor::White,
		TSet<UPrimitiveComponent*>()
		);
	ViewFamily->Views.AddItem(View);
	return View;
}

/** Utility for creating a random player character. */
UTexture* UUTCharInfo::MakeCharPortraitTexture(USkeletalMesh* CharMesh, FCharPortraitSetup Setup, UStaticMesh* BackgroundMesh)
{
	// Scene we will use for rendering this mesh to a texture.
	// The destructor of FPreviewScene removes all the components from it.
	FPreviewScene CharScene(Setup.DirLightRot, Setup.SkyBrightness, Setup.DirLightBrightness, FALSE);

	CharScene.RemoveComponent(CharScene.SkyLight);
	CharScene.SkyLight->LightColor = Setup.SkyColor;
	CharScene.SkyLight->LowerBrightness = Setup.SkyLowerBrightness;
	CharScene.SkyLight->LowerColor = Setup.SkyLowerColor;
	// Make sure the light is not affected by GSystemSettings.bAllowDynamicLights
	CharScene.SkyLight->bCanAffectDynamicPrimitivesOutsideDynamicChannel = TRUE;
	CharScene.AddComponent(CharScene.SkyLight, FMatrix::Identity);

	CharScene.RemoveComponent(CharScene.DirectionalLight);
	CharScene.DirectionalLight->bCanAffectDynamicPrimitivesOutsideDynamicChannel = TRUE;
	CharScene.DirectionalLight->LightColor = Setup.DirLightColor;
	CharScene.AddComponent(CharScene.DirectionalLight, FRotationMatrix(Setup.DirLightRot));

	UDirectionalLightComponent* DirLight2 = ConstructObject<UDirectionalLightComponent>(UDirectionalLightComponent::StaticClass());
	DirLight2->Brightness = Setup.DirLight2Brightness;
	DirLight2->LightColor = Setup.DirLight2Color;
	DirLight2->bCanAffectDynamicPrimitivesOutsideDynamicChannel = TRUE;
	CharScene.AddComponent(DirLight2, FRotationMatrix(Setup.DirLight2Rot));

	UDirectionalLightComponent* DirLight3 = ConstructObject<UDirectionalLightComponent>(UDirectionalLightComponent::StaticClass());
	DirLight3->Brightness = Setup.DirLight3Brightness;
	DirLight3->LightColor = Setup.DirLight3Color;
	DirLight3->bCanAffectDynamicPrimitivesOutsideDynamicChannel = TRUE;
	CharScene.AddComponent(DirLight3, FRotationMatrix(Setup.DirLight3Rot));

	if(BackgroundMesh)
	{
		UStaticMeshComponent* BackgroundComp = ConstructObject<UStaticMeshComponent>(UStaticMeshComponent::StaticClass());
		BackgroundComp->SetStaticMesh(BackgroundMesh);
		FScaleMatrix BGTM(5.f);
		BGTM.SetOrigin(Setup.PortraitBackgroundTranslation);
		CharScene.AddComponent(BackgroundComp, BGTM);
		//BackgroundComp->SetMaterial(0, Setup.BackgroundMaterial);
	}

	// Add the mesh
	USkeletalMeshComponent* PreviewSkelComp = ConstructObject<USkeletalMeshComponent>(USkeletalMeshComponent::StaticClass());
	PreviewSkelComp->SetSkeletalMesh(CharMesh);
	CharScene.AddComponent(PreviewSkelComp,FRotationMatrix(Setup.MeshRot));

	FVector UseOffset = Setup.MeshOffset;
	if(Setup.CenterOnBone != NAME_None)
	{
		INT BoneIndex = PreviewSkelComp->MatchRefBone(Setup.CenterOnBone);
		if(BoneIndex != INDEX_NONE)
		{
			FMatrix BoneTM = PreviewSkelComp->GetBoneMatrix(BoneIndex);
			UseOffset -= BoneTM.GetOrigin();
		}
	}

	FMatrix NewTM = FRotationMatrix(Setup.MeshRot);
	NewTM.SetOrigin(UseOffset);
	CharScene.RemoveComponent(PreviewSkelComp);
	CharScene.AddComponent(PreviewSkelComp, NewTM);

	// Create the output texture we'll render to
	UTextureRenderTarget2D* PortraitTex = ConstructObject<UTextureRenderTarget2D>(UTextureRenderTarget2D::StaticClass());
	check(PortraitTex);
	PortraitTex->bUpdateImmediate = TRUE;
	PortraitTex->bNeedsTwoCopies = FALSE;
	PortraitTex->bRenderOnce = TRUE;
	PortraitTex->Init(Setup.TextureSize, Setup.TextureSize, PF_A8R8G8B8);

	// TEMP: Block on scene being rendered.
	FlushRenderingCommands();

	// Create view family
	FSceneViewFamilyContext CharSceneViewFamily(
		PortraitTex->GetRenderTargetResource(),
		CharScene.GetScene(),
		(SHOW_DefaultGame & ~SHOW_PostProcess),
		appSeconds(),
		0.0f,
		appSeconds(), 
		NULL);

	// Calculate scene view and add to ViewFamily
	CalcPortraitSceneView(&CharSceneViewFamily, Setup.CamFOV, Setup.TextureSize);

	// Make a Canvas
	FCanvas Canvas(PortraitTex->GetRenderTargetResource(), NULL);

	FViewport* UseViewport = GEngine->GetAViewport();
	if(!UseViewport)
	{
		appErrorf(TEXT("MakeCharPortraitTexture - Couldn't find FViewport!"));
	}

	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
		BeginDrawingCommand,
		FViewport*,Viewport,UseViewport,
	{
		Viewport->BeginRenderFrame();
	}); 

	// Send a command to render this scene.
	BeginRenderingViewFamily(&Canvas, &CharSceneViewFamily);

	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		EndDrawingCommand,
		FViewport*,Viewport,UseViewport,
		FRenderTarget*,RenderTarget,PortraitTex->GetRenderTargetResource(),
	{
		Viewport->EndRenderFrame( FALSE, FALSE );
		RHICopyToResolveTarget(RenderTarget->GetRenderTargetSurface(), TRUE, FResolveParams());
	});

	// TEMP: Block on scene being rendered.
	FlushRenderingCommands();

	UTexture* OutPortraitTex = PortraitTex;

#if !CONSOLE
	//On PC, copy the render target to a texture.  This avoids needing to re-render the portrait everytime the d3d device is reset, eg when going into fullscreen.
	UTexture2D* StaticPortraitTex = PortraitTex->ConstructTexture2D(PortraitTex->GetOuter(), TEXT(""), PortraitTex->GetFlags(), FALSE, FALSE);
	check(StaticPortraitTex);
	OutPortraitTex = StaticPortraitTex;
#endif

	// Return the texture we created.
	return OutPortraitTex;
}

/** Find the index of this char in the unlockable char array. */
static INT FindUnlockableCharIndex(const TArray<FString>& UnlockableChars, const FString& InChar)
{
	for(INT i=0; i<UnlockableChars.Num(); i++)
	{
		if(UnlockableChars(i) == InChar)
		{
			return i;
		}
	}

	return INDEX_NONE;
}

/** Sets/unsets bit in CurrentStatus for the supplied character name. */
INT	UUTCharInfo::SetCharLockedStatus(const FString& CharName, UBOOL bLocked, INT CurrentStatus)
{
	INT CharIndex = FindUnlockableCharIndex(UnlockableChars, CharName);
	if(CharIndex == INDEX_NONE || CharIndex >= 31)
	{
		return CurrentStatus;
	}
	else
	{
		// Make bit mask for this char.
#if !__INTEL_BYTE_ORDER__
		INT BitMask = (1 << (31 - CharIndex));
#else
		INT BitMask = (1 << CharIndex);
#endif

		// Unset bit if still locked
		if(bLocked)
		{
			CurrentStatus &= ~BitMask;
		}
		// Set bit if unlocked
		else
		{
			CurrentStatus |= BitMask;
		}

		return CurrentStatus;
	}
}

/** See if a particular character is unlocked.*/
UBOOL UUTCharInfo::CharIsUnlocked(const FString& CharName, INT CurrentStatus)
{
	INT CharIndex = FindUnlockableCharIndex(UnlockableChars, CharName);
	if(CharIndex != INDEX_NONE || CharIndex >= 31)
	{
		// Make bit mask for this char.
#if !__INTEL_BYTE_ORDER__
		INT BitMask = (1 << (31 - CharIndex));
#else
		INT BitMask = (1 << CharIndex);
#endif

		// See if that bit is set in CurrentStatus
		if(CurrentStatus & BitMask)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
}
