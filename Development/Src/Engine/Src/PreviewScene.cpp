/*=============================================================================
	PreviewScene.cpp: Preview scene implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineAudioDeviceClasses.h"

FPreviewScene::FPreviewScene(const FRotator& LightRotation,
							 FLOAT SkyBrightness,
							 FLOAT LightBrightness,
							 UBOOL bAlwaysAllowAudioPlayback)
{
	Scene = AllocateScene( NULL, bAlwaysAllowAudioPlayback, FALSE );

	SkyLight = ConstructObject<USkyLightComponent>(USkyLightComponent::StaticClass());
	SkyLight->Brightness = SkyBrightness;
	SkyLight->LightColor = FColor(255,255,255);
	SkyLight->ConditionalAttach(Scene,NULL,FMatrix::Identity);
	Components.AddItem(SkyLight);

	DirectionalLight = ConstructObject<UDirectionalLightComponent>(UDirectionalLightComponent::StaticClass());
	DirectionalLight->Brightness = LightBrightness;
	DirectionalLight->LightColor = FColor(255,255,255);
	DirectionalLight->ConditionalAttach(Scene,NULL,FRotationMatrix(LightRotation));
	Components.AddItem(DirectionalLight);
}

FPreviewScene::~FPreviewScene()
{
	// Stop any audio components playing in this scene
	if( GEngine->Client && GEngine->Client->GetAudioDevice() )
	{
		GEngine->Client->GetAudioDevice()->Flush( GetScene() );
	}

	// Remove all the attached components
	for( INT ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++ )
	{
		Components( ComponentIndex )->ConditionalDetach();
	}

	Scene->Release();
}

void FPreviewScene::AddComponent(UActorComponent* Component,const FMatrix& LocalToWorld)
{
	Components.AddUniqueItem(Component);
	Component->ConditionalAttach(Scene,NULL,LocalToWorld);
}

void FPreviewScene::RemoveComponent(UActorComponent* Component)
{
	Component->ConditionalDetach();
	Components.RemoveItem(Component);
}

FArchive& operator<<(FArchive& Ar,FPreviewScene& P)
{
	return Ar << P.Components << P.DirectionalLight << P.SkyLight;
}

/** Accessor for finding the current direction of the preview scene's DirectionalLight. */
FRotator FPreviewScene::GetLightDirection()
{
	FMatrix ParentToWorld = FMatrix(
		FPlane(+0,+0,+1,+0),
		FPlane(+0,+1,+0,+0),
		FPlane(+1,+0,+0,+0),
		FPlane(+0,+0,+0,+1)
		) * DirectionalLight->LightToWorld;
		

	return ParentToWorld.Rotator();
}

/** Function for modifying the current direction of the preview scene's DirectionalLight. */
void FPreviewScene::SetLightDirection(const FRotator& InLightDir)
{
	FRotationTranslationMatrix NewLightTM( InLightDir, DirectionalLight->LightToWorld.GetOrigin() );
	DirectionalLight->ConditionalUpdateTransform(NewLightTM);
}

void FPreviewScene::SetLightBrightness(FLOAT LightBrightness)
{
	DirectionalLight->PreEditChange(NULL);
	DirectionalLight->Brightness = LightBrightness;
	DirectionalLight->PostEditChange(NULL);
}

void FPreviewScene::SetSkyBrightness(FLOAT SkyBrightness)
{
	SkyLight->PreEditChange(NULL);
	SkyLight->Brightness = SkyBrightness;
	SkyLight->PostEditChange(NULL);
}
