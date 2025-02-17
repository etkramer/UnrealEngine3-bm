/*=============================================================================
	SpotLightComponent.cpp: LightComponent implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "LightRendering.h"
#include "PointLightSceneInfo.h"

IMPLEMENT_CLASS(USpotLightComponent);

/**
 * The spot light policy for TMeshLightingDrawingPolicy.
 */
class FSpotLightPolicy
{
public:
	typedef class FSpotLightSceneInfo SceneInfoType;
	class VertexParametersType
	{
	public:
		void Bind(const FShaderParameterMap& ParameterMap)
		{
			LightPositionAndInvRadiusParameter.Bind(ParameterMap,TEXT("LightPositionAndInvRadius"));
		}
		void SetLight(FShader* VertexShader,const FSpotLightSceneInfo* Light,const FSceneView* View) const;
		void Serialize(FArchive& Ar)
		{
			Ar << LightPositionAndInvRadiusParameter;
		}
	private:
		FShaderParameter LightPositionAndInvRadiusParameter;
	};
	class PixelParametersType
	{
	public:
		void Bind(const FShaderParameterMap& ParameterMap)
		{
			SpotAnglesParameter.Bind(ParameterMap,TEXT("SpotAngles"),TRUE);
			SpotDirectionParameter.Bind(ParameterMap,TEXT("SpotDirection"),TRUE);
			LightColorAndFalloffExponentParameter.Bind(ParameterMap,TEXT("LightColorAndFalloffExponent"),TRUE);
		}
		void SetLight(FShader* PixelShader,const FSpotLightSceneInfo* Light,const FSceneView* View) const;
		void Serialize(FArchive& Ar)
		{
			Ar << SpotAnglesParameter;
			Ar << SpotDirectionParameter;
			Ar << LightColorAndFalloffExponentParameter;
		}
	private:
		FShaderParameter SpotAnglesParameter;
		FShaderParameter SpotDirectionParameter;
		FShaderParameter LightColorAndFalloffExponentParameter;
	};

	/**
	* Modulated shadow shader params associated with this light policy
	*/
	class ModShadowPixelParamsType
	{
	public:
		void Bind( const FShaderParameterMap& ParameterMap )
		{
			LightPositionParam.Bind(ParameterMap,TEXT("LightPosition"));
			FalloffParameters.Bind(ParameterMap,TEXT("FalloffParameters"));
			SpotDirectionParam.Bind(ParameterMap,TEXT("SpotDirection"),TRUE);
			SpotAnglesParam.Bind(ParameterMap,TEXT("SpotAngles"),TRUE);
		}
		void SetModShadowLight( FShader* PixelShader, const FSpotLightSceneInfo* Light,const FSceneView* View ) const;
		void Serialize( FArchive& Ar )
		{
			Ar << LightPositionParam;
			Ar << FalloffParameters;
			Ar << SpotDirectionParam;
			Ar << SpotAnglesParam;
		}
		static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
		{
			OutEnvironment.Definitions.Set(TEXT("MODSHADOW_LIGHTTYPE_SPOT"),TEXT("1"));
		}
	private:
		/** world position of light casting a shadow. Note: w = 1.0 / Radius */
		FShaderParameter LightPositionParam;
		/** attenuation exponent for light casting a shadow */
		FShaderParameter FalloffParameters;
		/** spot light direction vector in world space */
		FShaderParameter SpotDirectionParam;
		/** spot light cone cut-off angles */
		FShaderParameter SpotAnglesParam;
	};

	static UBOOL ShouldCacheStaticLightingShaders()
	{
		return TRUE;
	}
};

IMPLEMENT_LIGHT_SHADER_TYPE(FSpotLightPolicy,TEXT("SpotLightVertexShader"),TEXT("SpotLightPixelShader"),VER_CONTENT_RESAVE_AUGUST_2007_QA_BUILD,0);

/**
 * The scene info for a spot light.
 */
class FSpotLightSceneInfo : public TPointLightSceneInfo<FSpotLightPolicy>
{
public:

	/** Cosine of the spot light's inner cone angle. */
	FLOAT CosInnerCone;

	/** Cosine of the spot light's outer cone angle. */
	FLOAT CosOuterCone;

	/** 1 / (CosInnerCone - CosOuterCone) */
	FLOAT InvCosConeDifference;

	/** Sine of the spot light's outer cone angle. */
	FLOAT SinOuterCone;

	/** 1 / Tangent of the spot light's outer cone angle. */
	FLOAT InvTanOuterCone;

	/** Initialization constructor. */
	FSpotLightSceneInfo(const USpotLightComponent* Component):
		TPointLightSceneInfo<FSpotLightPolicy>(Component)
	{
		FLOAT ClampedInnerConeAngle = Clamp(Component->InnerConeAngle,0.0f,89.0f) * (FLOAT)PI / 180.0f;
		FLOAT ClampedOuterConeAngle = Clamp(Component->OuterConeAngle * (FLOAT)PI / 180.0f,ClampedInnerConeAngle + 0.001f,89.0f * (FLOAT)PI / 180.0f + 0.001f);
		CosOuterCone = appCos(ClampedOuterConeAngle);
		SinOuterCone = appSin(ClampedOuterConeAngle);
		CosInnerCone = appCos(ClampedInnerConeAngle);
		InvCosConeDifference = 1.0f / (CosInnerCone - CosOuterCone);
		InvTanOuterCone = 1.0f / appTan(ClampedOuterConeAngle);
	}

	// FLightSceneInfo interface.
	virtual UBOOL AffectsBounds(const FBoxSphereBounds& Bounds) const
	{
		if(!TPointLightSceneInfo<FSpotLightPolicy>::AffectsBounds(Bounds))
		{
			return FALSE;
		}

		FVector	U = GetOrigin() - (Bounds.SphereRadius / SinOuterCone) * GetDirection(),
				D = Bounds.Origin - U;
		FLOAT	dsqr = D | D,
				E = GetDirection() | D;
		if(E > 0.0f && E * E >= dsqr * Square(CosOuterCone))
		{
			D = Bounds.Origin - GetOrigin();
			dsqr = D | D;
			E = -(GetDirection() | D);
			if(E > 0.0f && E * E >= dsqr * Square(SinOuterCone))
				return dsqr <= Square(Bounds.SphereRadius);
			else
				return TRUE;
		}

		return FALSE;
	}
	
	virtual UBOOL GetWholeSceneProjectedShadowInitializer(FProjectedShadowInitializer& OutInitializer) const
	{
		// Create a shadow projection that includes the entire spot light cone.
		return OutInitializer.CalcTransforms(
			-LightToWorld.GetOrigin(),
			WorldToLight.RemoveTranslation() * 
				FScaleMatrix(FVector(-InvTanOuterCone,InvTanOuterCone,1.0f)),
			FVector(0,0,1),
			FBoxSphereBounds(
				LightToWorld.RemoveTranslation().TransformFVector(FVector(0,0,Radius / 2.0f)),
				FVector(Radius/2.0f,Radius/2.0f,Radius/2.0f),
				Radius / 2.0f
				),
			FVector4(0,0,1,0),
			0.1f,
			Radius,
			FALSE
			);
	}
};

void FSpotLightPolicy::VertexParametersType::SetLight(FShader* VertexShader,const FSpotLightSceneInfo* Light,const FSceneView* View) const
{
	SetVertexShaderValue(
		VertexShader->GetVertexShader(),
		LightPositionAndInvRadiusParameter,
		FVector4(
			Light->GetOrigin() + View->PreViewTranslation,
			Light->InvRadius
			)
		);
}

void FSpotLightPolicy::PixelParametersType::SetLight(FShader* PixelShader,const FSpotLightSceneInfo* Light,const FSceneView* View) const
{
	SetPixelShaderValue(PixelShader->GetPixelShader(),SpotAnglesParameter,FVector4(Light->CosOuterCone,Light->InvCosConeDifference,0,0));
	SetPixelShaderValue(PixelShader->GetPixelShader(),SpotDirectionParameter,Light->GetDirection());
	SetPixelShaderValue(PixelShader->GetPixelShader(),LightColorAndFalloffExponentParameter,
		FVector4(Light->Color.R,Light->Color.G,Light->Color.B,Light->FalloffExponent));
}

void FSpotLightPolicy::ModShadowPixelParamsType::SetModShadowLight(FShader* PixelShader,const FSpotLightSceneInfo* Light,const FSceneView* View) const
{
	// set world light position and falloff rate
	SetPixelShaderValue(
		PixelShader->GetPixelShader(),
		LightPositionParam,
		FVector4(
			Light->GetOrigin() + View->PreViewTranslation,
			1.0f / Light->Radius
			)
		);
	SetPixelShaderValue(
		PixelShader->GetPixelShader(),
		FalloffParameters, 
		FVector(
			Light->ShadowFalloffExponent,
			Light->ShadowFalloffScale,
			Light->ShadowFalloffBias
			)
		);
	// set spot light direction
	SetPixelShaderValue(PixelShader->GetPixelShader(),SpotDirectionParam,Light->GetDirection());
	// set spot light inner/outer cone angles
	SetPixelShaderValue(PixelShader->GetPixelShader(),SpotAnglesParam,FVector4(Light->CosOuterCone,Light->InvCosConeDifference,0,0));
}

FLightSceneInfo* USpotLightComponent::CreateSceneInfo() const
{
	return new FSpotLightSceneInfo(this);
}

UBOOL USpotLightComponent::AffectsBounds(const FBoxSphereBounds& Bounds) const
{
	if(!Super::AffectsBounds(Bounds))
	{
		return FALSE;
	}

	FLOAT	ClampedInnerConeAngle = Clamp(InnerConeAngle,0.0f,89.0f) * (FLOAT)PI / 180.0f,
			ClampedOuterConeAngle = Clamp(OuterConeAngle * (FLOAT)PI / 180.0f,ClampedInnerConeAngle + 0.001f,89.0f * (FLOAT)PI / 180.0f + 0.001f);

	FLOAT	Sin = appSin(ClampedOuterConeAngle),
			Cos = appCos(ClampedOuterConeAngle);

	FVector	U = GetOrigin() - (Bounds.SphereRadius / Sin) * GetDirection(),
			D = Bounds.Origin - U;
	FLOAT	dsqr = D | D,
			E = GetDirection() | D;
	if(E > 0.0f && E * E >= dsqr * Square(Cos))
	{
		D = Bounds.Origin - GetOrigin();
		dsqr = D | D;
		E = -(GetDirection() | D);
		if(E > 0.0f && E * E >= dsqr * Square(Sin))
			return dsqr <= Square(Bounds.SphereRadius);
		else
			return TRUE;
	}

	return FALSE;
}

void USpotLightComponent::Attach()
{
	Super::Attach();

	if ( PreviewInnerCone )
	{
		PreviewInnerCone->ConeRadius = Radius;
		PreviewInnerCone->ConeAngle = InnerConeAngle;
		PreviewInnerCone->Translation = Translation;
	}

	if ( PreviewOuterCone )
	{
		PreviewOuterCone->ConeRadius = Radius;
		PreviewOuterCone->ConeAngle = OuterConeAngle;
		PreviewOuterCone->Translation = Translation;
	}
}

FLinearColor USpotLightComponent::GetDirectIntensity(const FVector& Point) const
{
	FLOAT	ClampedInnerConeAngle = Clamp(InnerConeAngle,0.0f,89.0f) * (FLOAT)PI / 180.0f,
			ClampedOuterConeAngle = Clamp(OuterConeAngle * (FLOAT)PI / 180.0f,ClampedInnerConeAngle + 0.001f,89.0f * (FLOAT)PI / 180.0f + 0.001f),
			OuterCone = appCos(ClampedOuterConeAngle),
			InnerCone = appCos(ClampedInnerConeAngle);

	FVector LightVector = (Point - GetOrigin()).SafeNormal();
	FLOAT SpotAttenuation = Square(Clamp<FLOAT>(((LightVector | GetDirection()) - OuterCone) / (InnerCone - OuterCone),0.0f,1.0f));
	return Super::GetDirectIntensity(Point) * SpotAttenuation;
}

/**
* @return ELightComponentType for the light component class 
*/
ELightComponentType USpotLightComponent::GetLightType() const
{
	return LightType_Spot;
}

void USpotLightComponent::PostLoad()
{
	Super::PostLoad();

	if ( GIsEditor
	&& !IsTemplate(RF_ClassDefaultObject)
	// FReloadObjectArcs call PostLoad() *prior* to instancing components for objects being reloaded, so this check is invalid in that case
	&& (GUglyHackFlags&HACK_IsReloadObjArc) == 0 )
	{
		if ( PreviewInnerCone != NULL && PreviewInnerCone->GetOuter() != GetOuter() )
		{
			// so if we are here, then the owning light actor was definitely created after the fixup code was added, so there is some way that this bug is still occurring
			// I need to figure out how this is occurring so let's annoy the designer into bugging me.
			debugf(TEXT("%s has an invalid PreviewInnerCone '%s' even though package has been resaved since this bug was fixed.  Please let Ron know about this immediately!"), *GetFullName(), *PreviewInnerCone->GetFullName());
			//@todo ronp - remove this once we've verified that this is no longer occurring.
			appMsgf(AMT_OK, TEXT("%s has an invalid PreviewInnerCone '%s' even though package has been resaved since this bug was fixed.  Please let Ron know about this immediately! (this message has already been written to the log)"), *GetFullName(), *PreviewInnerCone->GetFullName());
		}
		else if ( PreviewOuterCone != NULL && PreviewOuterCone->GetOuter() != GetOuter() )
		{
			// so if we are here, then the owning light actor was definitely created after the fixup code was added, so there is some way that this bug is still occurring
			// I need to figure out how this is occurring so let's annoy the designer into bugging me.
			debugf(TEXT("%s has an invalid PreviewOuterCone '%s' even though package has been resaved since this bug was fixed.  Please let Ron know about this immediately!"), *GetFullName(), *PreviewOuterCone->GetFullName());
			//@todo ronp - remove this once we've verified that this is no longer occurring.
			appMsgf(AMT_OK, TEXT("%s has an invalid PreviewOuterCone '%s' even though package has been resaved since this bug was fixed.  Please let Ron know about this immediately! (this message has already been written to the log)"), *GetFullName(), *PreviewOuterCone->GetFullName());
		}
	}
}

