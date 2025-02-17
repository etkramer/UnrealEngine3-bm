/*=============================================================================
	SpriteComponent.cpp: USpriteComponent implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "LevelUtils.h"

IMPLEMENT_CLASS(USpriteComponent);

/** Represents a sprite to the scene manager. */
class FSpriteSceneProxy : public FPrimitiveSceneProxy
{
public:

	/** Initialization constructor. */
	FSpriteSceneProxy(const USpriteComponent* InComponent):
		FPrimitiveSceneProxy(InComponent),
		ScreenSize(InComponent->ScreenSize),
		Color(255,255,255),
		LevelColor(255,255,255),
		PropertyColor(255,255,255),
		bIsScreenSizeScaled(InComponent->bIsScreenSizeScaled)
	{
		// Calculate the scale factor for the sprite.
		FLOAT Scale = InComponent->Scale;
		if(InComponent->GetOwner())
		{
			Scale *= InComponent->GetOwner()->DrawScale;
		}

		if(InComponent->Sprite)
		{
			Texture = InComponent->Sprite->Resource;
			SizeX = Scale * InComponent->Sprite->GetSurfaceWidth();
			SizeY = Scale * InComponent->Sprite->GetSurfaceHeight();
		}
		else
		{
			Texture = NULL;
			SizeX = SizeY = 0;
		}

        AActor* Owner = InComponent->GetOwner();
		if (Owner)
		{
			// If the owner of this sprite component is an ALight, tint the sprite
			// to match the lights color.

			ALight* Light = Cast<ALight>( Owner );
			if( Light )
			{
				if( Light->LightComponent )
				{
					Color = Light->LightComponent->LightColor.ReinterpretAsLinear();
					Color.A = 255;
				}
			}

			// Override the colors for special situations

			const UBOOL bIsSelected = Owner->IsSelected();
			if ( bIsSelected )
			{
				Color = FColor(128,230,128);
			}

			// Sprites of locked actors draw in red.
			if (Owner->bLockLocation)
			{
				Color = FColor(255,0,0);
			}

			// Level colorization
			ULevel* Level = Owner->GetLevel();
			ULevelStreaming* LevelStreaming = FLevelUtils::FindStreamingLevel( Level );
			if ( LevelStreaming )
			{
				// Selection takes priority over level coloration.
				LevelColor = bIsSelected ? Color : LevelStreaming->DrawColor;
			}
		}

		GEngine->GetPropertyColorationColor( (UObject*)InComponent, PropertyColor );
	}

	// FPrimitiveSceneProxy interface.
	
	/** 
	 * Draw the scene proxy as a dynamic element
	 *
	 * @param	PDI - draw interface to render to
	 * @param	View - current view
	 * @param	DPGIndex - current depth priority 
	 * @param	Flags - optional set of flags from EDrawDynamicElementFlags
	 */
	virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex,DWORD Flags)
	{
		// Determine the DPG the primitive should be drawn in for this view.
		if ((GetViewRelevance(View).GetDPG(DPGIndex) == TRUE) && Texture)
		{
			// Calculate the view-dependent scaling factor.
			FLOAT ViewedSizeX = SizeX;
			FLOAT ViewedSizeY = SizeY;
			if (bIsScreenSizeScaled && (View->ProjectionMatrix.M[3][3] != 1.0f))
			{
				const FLOAT ZoomFactor	= Min<FLOAT>(View->ProjectionMatrix.M[0][0], View->ProjectionMatrix.M[1][1]);
				const FLOAT Radius		= View->WorldToScreen(Origin).W * (ScreenSize / ZoomFactor);
				if (Radius < 1.0f)
				{
					ViewedSizeX *= Radius;
					ViewedSizeY *= Radius;
				}
			}

			const FColor& SpriteColor = (View->Family->ShowFlags & SHOW_LevelColoration) ? LevelColor :
											( (View->Family->ShowFlags & SHOW_PropertyColoration) ? PropertyColor : Color );

			PDI->DrawSprite(
				Origin,
				ViewedSizeX,
				ViewedSizeY,
				Texture,
				SpriteColor,
				DPGIndex
				);
		}
	}
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View)
	{
		const UBOOL bVisible = (View->Family->ShowFlags & SHOW_Sprites) ? TRUE : FALSE;
		FPrimitiveViewRelevance Result;
		Result.bDynamicRelevance = IsShown(View);
		Result.SetDPG(GetDepthPriorityGroup(View),bVisible);
		if (IsShadowCast(View))
		{
			Result.bShadowRelevance = TRUE;
		}
		return Result;
	}
	virtual void OnTransformChanged()
	{
		Origin = LocalToWorld.GetOrigin();
	}
	virtual EMemoryStats GetMemoryStatType( void ) const { return( STAT_GameToRendererMallocOther ); }
	virtual DWORD GetMemoryFootprint( void ) const { return( sizeof( *this ) + GetAllocatedSize() ); }
	DWORD GetAllocatedSize( void ) const { return( FPrimitiveSceneProxy::GetAllocatedSize() ); }

private:
	FVector Origin;
	FLOAT SizeX;
	FLOAT SizeY;
	const FLOAT ScreenSize;
	const FTexture* Texture;
	FColor Color;
	FColor LevelColor;
	FColor PropertyColor;
	const BITFIELD bIsScreenSizeScaled : 1;
};

FPrimitiveSceneProxy* USpriteComponent::CreateSceneProxy()
{
	return new FSpriteSceneProxy(this);
}

void USpriteComponent::UpdateBounds()
{
	const FLOAT NewScale = (Owner ? Owner->DrawScale : 1.0f) * (Sprite ? (FLOAT)Max(Sprite->SizeX,Sprite->SizeY) : 1.0f);
	Bounds = FBoxSphereBounds(GetOrigin(),FVector(NewScale,NewScale,NewScale),appSqrt(3.0f * Square(NewScale)));
}
