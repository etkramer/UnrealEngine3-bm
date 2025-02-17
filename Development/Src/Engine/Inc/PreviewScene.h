/*=============================================================================
	PreviewScene.h: Preview scene definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __PREVIEWSCENE_H__
#define __PREVIEWSCENE_H__

/**
 * Encapsulates a simple scene setup for preview or thumbnail rendering.
 */
class FPreviewScene
{
public:
	FPreviewScene(const FRotator& LightRotation = FRotator(-8192,8192,0),FLOAT SkyBrightness = 0.25f,FLOAT LightBrightness = 1.0f,UBOOL bAlwaysAllowAudioPlayback = FALSE);
	virtual ~FPreviewScene();

	/**
	 * Adds a component to the preview scene.  This attaches the component to the scene, and takes ownership of it.
	 */
	void AddComponent(class UActorComponent* Component,const FMatrix& LocalToWorld);

	/**
	 * Removes a component from the preview scene.  This detaches the component from the scene, and returns ownership of it.
	 */
	void RemoveComponent(class UActorComponent* Component);

	// Serializer.
	friend FArchive& operator<<(FArchive& Ar,FPreviewScene& P);

	// Accessors.
	FSceneInterface* GetScene() const { return Scene; }

	FRotator GetLightDirection();
	void SetLightDirection(const FRotator& InLightDir);
	void SetLightBrightness(FLOAT LightBrightness);

	void SetSkyBrightness(FLOAT SkyBrightness);

	class UDirectionalLightComponent* DirectionalLight;
	class USkyLightComponent* SkyLight;
private:

	TArray<class UActorComponent*> Components;


protected:
	FSceneInterface* Scene;
};

#endif // __PREVIEWSCENE_H__
