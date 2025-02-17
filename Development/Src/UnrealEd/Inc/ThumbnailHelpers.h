/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
#ifdef _WANTS_ALL_HELPERS
#define _WANTS_PARTICLE_HELPER 1
#define _WANTS_MATERIAL_HELPER 1
#define _WANTS_SKELETAL_HELPER 1
#define _WANTS_SM_HELPER 1
#endif

#ifdef _WANTS_PARTICLE_HELPER
//
//	FParticleSystemThumbnailScene
//

class FParticleSystemThumbnailScene : public FPreviewScene
{
public:

	UParticleSystemComponent* PartComponent;

	// Constructor/destructor.

	FParticleSystemThumbnailScene(UParticleSystem* ParticleSystem)
	{
		UBOOL bNewComponent = false;

		// If no preview component currently existing - create it now and warm it up.
		if (!ParticleSystem->PreviewComponent)
		{
			ParticleSystem->PreviewComponent = ConstructObject<UParticleSystemComponent>(UParticleSystemComponent::StaticClass());
			ParticleSystem->PreviewComponent->Template = ParticleSystem;

			ParticleSystem->PreviewComponent->LocalToWorld.SetIdentity();
	
			bNewComponent = true;
		}

		PartComponent = ParticleSystem->PreviewComponent;
		check(PartComponent);

		// Add Particle component to this scene.
		FPreviewScene::AddComponent(PartComponent,FMatrix::Identity);

		PartComponent->InitializeSystem();
		PartComponent->ActivateSystem();

		// If its new - tick it so its at the warmup time.
//		if (bNewComponent && (PartComponent->WarmupTime == 0.0f))
		if (PartComponent->WarmupTime == 0.0f)
		{
			ParticleSystem->PreviewComponent->ResetBurstLists();

			FLOAT WarmupElapsed = 0.f;
			FLOAT WarmupTimestep = 0.02f;
			while(WarmupElapsed < ParticleSystem->ThumbnailWarmup)
			{
				ParticleSystem->PreviewComponent->Tick(WarmupTimestep);
				WarmupElapsed += WarmupTimestep;
			}
		}
	}

	void GetView(FSceneViewFamily* ViewFamily,INT X,INT Y,UINT SizeX,UINT SizeY)
	{
		UParticleSystem* ParticleSystem = PartComponent->Template;
		FMatrix ViewMatrix = FTranslationMatrix(ParticleSystem->ThumbnailAngle.Vector() * ParticleSystem->ThumbnailDistance);
		ViewMatrix = ViewMatrix * FInverseRotationMatrix(ParticleSystem->ThumbnailAngle);
		ViewMatrix = ViewMatrix * FMatrix(
			FPlane(0,	0,	1,	0),
			FPlane(1,	0,	0,	0),
			FPlane(0,	1,	0,	0),
			FPlane(0,	0,	0,	1));

		FMatrix ProjectionMatrix = FPerspectiveMatrix(
			75.0f * (FLOAT)PI / 360.0f,
			1.0f,
			1.0f,
			NEAR_CLIPPING_PLANE
			);

		ViewFamily->Views.AddItem(
			new FSceneView(
				ViewFamily,
				NULL,
				-1,
				NULL,
				NULL,
				NULL,
				GEngine->ThumbnailParticleSystemPostProcess,
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
				)
			);
	}
};
#endif

#ifdef _WANTS_MATERIAL_HELPER
//
//	FMaterialThumbnailScene
//

class FMaterialThumbnailScene : public FPreviewScene
{
private:
	FLinearColor BackgroundClearColor;
	UStaticMeshComponent* StaticMeshComponent;
	UStaticMeshComponent* BackgroundComponent;

public:	
	// Constructor/destructor.
	FMaterialThumbnailScene(UMaterialInterface* InMaterial,
		EThumbnailPrimType InPrimType,INT InPitch,INT InYaw,FLOAT InZoomPct,
		EThumbnailBackgroundType InBackgroundType,
		FColor BackgroundColor,
		FColor BackgroundColorForTranslucent)
		: BackgroundClearColor(BackgroundColor)
	{
		InZoomPct = Clamp<FLOAT>( InZoomPct, 1.0, 2.0 );

		// Background

		BackgroundComponent = NULL;
		if( InBackgroundType == TBT_DefaultBackground ||
			InBackgroundType == TBT_SolidBackground )
		{
			BackgroundComponent = GUnrealEd->GetThumbnailManager()->GetBackgroundComponent();
			BackgroundComponent->StaticMesh = GUnrealEd->GetThumbnailManager()->TexPropCube;
			BackgroundComponent->Materials.Empty();
			if( InBackgroundType == TBT_SolidBackground &&
				GUnrealEd->GetThumbnailManager()->ThumbnailBackgroundSolidMatInst ) 
			{
				// render solid color for background
				const FLinearColor BackgroundColor(FLinearColor::Black);
				GUnrealEd->GetThumbnailManager()->ThumbnailBackgroundSolidMatInst->SetVectorParameterValue(FName(TEXT("Color")),BackgroundColor);
				BackgroundComponent->Materials.AddItem( GUnrealEd->GetThumbnailManager()->ThumbnailBackgroundSolidMatInst );
			}
			else if( GUnrealEd->GetThumbnailManager()->ThumbnailBackground )
			{
				// render with thumbnail background material
				BackgroundComponent->Materials.AddItem( GUnrealEd->GetThumbnailManager()->ThumbnailBackground );
			}			
			BackgroundComponent->CastShadow = 0;
			FPreviewScene::AddComponent(BackgroundComponent,FScaleMatrix(FVector(7,7,7)));
		}

		// Preview primitive

		StaticMeshComponent = GUnrealEd->GetThumbnailManager()->GetStaticMeshPreviewComponent();
		check(StaticMeshComponent);
		switch( InPrimType )
		{
			case TPT_Cube:
				StaticMeshComponent->StaticMesh = GUnrealEd->GetThumbnailManager()->TexPropCube;
				break;
			case TPT_Sphere:
				StaticMeshComponent->StaticMesh = GUnrealEd->GetThumbnailManager()->TexPropSphere;
				break;
			case TPT_Cylinder:
				StaticMeshComponent->StaticMesh = GUnrealEd->GetThumbnailManager()->TexPropCylinder;
				break;
			case TPT_Plane:
				StaticMeshComponent->StaticMesh = GUnrealEd->GetThumbnailManager()->TexPropPlane;
				InPitch = 0;
				InYaw = 0;
				break;
			default:
				check(0);
		}
		StaticMeshComponent->Materials.Empty();
		StaticMeshComponent->Materials.AddItem(InMaterial);
		StaticMeshComponent->CastShadow = 0;
		FMatrix Matrix = FMatrix::Identity;
		Matrix = Matrix * FRotationMatrix( FRotator(-InPitch,-InYaw,0) );

		// Hack because Plane staticmesh points wrong way.
		if(InPrimType == TPT_Plane)
			Matrix = Matrix * FRotationMatrix( FRotator(0, 32767, 0) );

		InZoomPct -= 1.0;
		Matrix = Matrix * FTranslationMatrix( FVector(-100*InZoomPct,0,0 ) );
		FPreviewScene::AddComponent(StaticMeshComponent,Matrix);

		UMaterial* ThumbMaterial = InMaterial->GetMaterial();
		if (ThumbMaterial && ThumbMaterial->bUsedWithFogVolumes)
		{
			Scene->RemoveFogVolume(StaticMeshComponent);
			Scene->AddFogVolume(StaticMeshComponent);
		}
		if( ThumbMaterial && ThumbMaterial->BlendMode == BLEND_Modulate )
		{
			BackgroundClearColor = FLinearColor::White;
		}
		else if( ThumbMaterial && ThumbMaterial->BlendMode == BLEND_Translucent )
		{
			BackgroundClearColor = FLinearColor( BackgroundColorForTranslucent );
		}
	}

	void GetView(FSceneViewFamily* ViewFamily,INT X,INT Y,UINT SizeX,UINT SizeY)
	{
		FMatrix ViewMatrix = FTranslationMatrix(-FVector(-350,0,0));
		ViewMatrix = ViewMatrix * FMatrix(
			FPlane(0,	0,	1,	0),
			FPlane(1,	0,	0,	0),
			FPlane(0,	1,	0,	0),
			FPlane(0,	0,	0,	1));

		FMatrix ProjectionMatrix = FPerspectiveMatrix(
			75.0f * (FLOAT)PI / 360.0f,
			1.0f,
			1.0f,
			NEAR_CLIPPING_PLANE
			);

		ViewFamily->Views.AddItem(
			new FSceneView(
				ViewFamily,
				NULL,
				-1,
				NULL,
				NULL,
				NULL,
				GEngine->ThumbnailMaterialPostProcess,
				NULL,
				NULL,
				NULL,
				X,
				Y,
				SizeX,
				SizeY,
				ViewMatrix,
				ProjectionMatrix,
				BackgroundClearColor,
				FLinearColor(0,0,0,0),
				FLinearColor::White,
				TSet<UPrimitiveComponent*>()
				)
			);
	}
};
#endif

#ifdef _WANTS_SKELETAL_HELPER
class FSkeletalMeshThumbnailScene : public FPreviewScene
{
public:

	USkeletalMeshComponent* SkeletalMeshComponent;

	// Constructor/destructor.

	FSkeletalMeshThumbnailScene(USkeletalMesh* InSkeletalMesh)
	{
		SkeletalMeshComponent = GUnrealEd->GetThumbnailManager()->GetSkeletalMeshPreviewComponent();
		SkeletalMeshComponent->SkeletalMesh = InSkeletalMesh;
		SkeletalMeshComponent->Materials.Empty();
		FPreviewScene::AddComponent(SkeletalMeshComponent,FMatrix::Identity);
	}

	void GetView(FSceneViewFamily* ViewFamily,INT X,INT Y,UINT SizeX,UINT SizeY)
	{
		// We look at the bounding box to see which side to render it from.
		FMatrix ViewMatrix;

		if(SkeletalMeshComponent->Bounds.BoxExtent.X > SkeletalMeshComponent->Bounds.BoxExtent.Y)
		{
			ViewMatrix = FTranslationMatrix(-(SkeletalMeshComponent->Bounds.Origin - FVector(0,SkeletalMeshComponent->Bounds.SphereRadius / (75.0f * (FLOAT)PI / 360.0f),0)));
			ViewMatrix = ViewMatrix * FInverseRotationMatrix(FRotator(0,16384,0));
		}
		else
		{
			ViewMatrix = FTranslationMatrix(-(SkeletalMeshComponent->Bounds.Origin - FVector(-SkeletalMeshComponent->Bounds.SphereRadius / (75.0f * (FLOAT)PI / 360.0f),0,0)));
			ViewMatrix = ViewMatrix * FInverseRotationMatrix(FRotator(0,32768,0));
		}
		ViewMatrix = ViewMatrix * FMatrix(
			FPlane(0,	0,	1,	0),
			FPlane(1,	0,	0,	0),
			FPlane(0,	1,	0,	0),
			FPlane(0,	0,	0,	1));

		FMatrix ProjectionMatrix = FPerspectiveMatrix(
			75.0f * (FLOAT)PI / 360.0f,
			1.0f,
			1.0f,
			NEAR_CLIPPING_PLANE
			);

		ViewFamily->Views.AddItem(
				new FSceneView(
				ViewFamily,
				NULL,
				-1,
				NULL,
				NULL,
				NULL,
				GEngine->ThumbnailSkeletalMeshPostProcess,
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
				)
			);
	}
};
#endif

#ifdef _WANTS_SM_HELPER
class FStaticMeshThumbnailScene : public FPreviewScene
{
public:

	UStaticMeshComponent* StaticMeshComponent;

	// Constructor/destructor.

	FStaticMeshThumbnailScene(UStaticMesh* StaticMesh)
	{
		StaticMeshComponent = GUnrealEd->GetThumbnailManager()->GetStaticMeshPreviewComponent();
		StaticMeshComponent->StaticMesh = StaticMesh;
		StaticMeshComponent->Materials.Empty();

		FMatrix LockMatrix = FRotationMatrix( FRotator(0,StaticMesh->ThumbnailAngle.Yaw,0) ) * FRotationMatrix( FRotator(0,0,StaticMesh->ThumbnailAngle.Pitch) );
		FPreviewScene::AddComponent(StaticMeshComponent,FTranslationMatrix(-StaticMesh->Bounds.Origin) * LockMatrix);
	}

	void GetView(FSceneViewFamily* ViewFamily,INT X,INT Y,UINT SizeX,UINT SizeY)
	{
		UStaticMesh* StaticMesh = StaticMeshComponent->StaticMesh;
		FMatrix ViewMatrix = FTranslationMatrix(FVector(0,StaticMesh->Bounds.SphereRadius / (75.0f * (FLOAT)PI / 360.0f) + StaticMesh->ThumbnailDistance,0));
		ViewMatrix = ViewMatrix * FInverseRotationMatrix(FRotator(0,16384,0));
		ViewMatrix = ViewMatrix * FMatrix(
			FPlane(0,	0,	1,	0),
			FPlane(1,	0,	0,	0),
			FPlane(0,	1,	0,	0),
			FPlane(0,	0,	0,	1));

		FMatrix ProjectionMatrix = FPerspectiveMatrix(
			75.0f * (FLOAT)PI / 360.0f,
			1.0f,
			1.0f,
			NEAR_CLIPPING_PLANE
			);

		ViewFamily->Views.AddItem(
			new FSceneView(
				ViewFamily,
				NULL,
				-1,
				NULL,
				NULL,
				NULL,
				GEngine->ThumbnailSkeletalMeshPostProcess,
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
				)
			);
	}
};
#endif
