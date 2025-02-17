/*=============================================================================
	ParticleModules_Parameter.cpp: 
	Parameter-related particle module implementations.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#include "EnginePrivate.h"
#include "EngineParticleClasses.h"
#include "EnginePhysicsClasses.h"
#include "EngineMaterialClasses.h"
#include "UnNovodexSupport.h"

/*-----------------------------------------------------------------------------
	Abstract base modules used for categorization.
-----------------------------------------------------------------------------*/
IMPLEMENT_CLASS(UParticleModuleParameterBase);

/*-----------------------------------------------------------------------------
	UParticleModuleParameterDynamic implementation.
-----------------------------------------------------------------------------*/
IMPLEMENT_CLASS(UParticleModuleParameterDynamic);

    //## BEGIN PROPS ParticleModuleParameterDynamic
//    TArrayNoInit<struct FEmitterDynamicParameter> DynamicParams;
    //## END PROPS ParticleModuleParameterDynamic

/**
 *	Called on a particle that is freshly spawned by the emitter.
 *	
 *	@param	Owner		The FParticleEmitterInstance that spawned the particle.
 *	@param	Offset		The modules offset into the data payload of the particle.
 *	@param	SpawnTime	The time of the spawn.
 */
void UParticleModuleParameterDynamic::Spawn(FParticleEmitterInstance* Owner, INT Offset, FLOAT SpawnTime)
{
	SPAWN_INIT;
	{
		PARTICLE_ELEMENT(FEmitterDynamicParameterPayload, DynamicPayload);

		for (INT ParamIndex = 0; ParamIndex < 4; ParamIndex++)
		{
			DynamicPayload.DynamicParameterValue[ParamIndex] = 
				DynamicParams(ParamIndex).bUseEmitterTime ? 
					DynamicParams(ParamIndex).ParamValue.GetValue(Owner->EmitterTime) :
					DynamicParams(ParamIndex).ParamValue.GetValue(Particle.RelativeTime);
		}
	}
}

/**
 *	Called on a particle that is being updated by its emitter.
 *	
 *	@param	Owner		The FParticleEmitterInstance that 'owns' the particle.
 *	@param	Offset		The modules offset into the data payload of the particle.
 *	@param	DeltaTime	The time since the last update.
 */
void UParticleModuleParameterDynamic::Update(FParticleEmitterInstance* Owner, INT Offset, FLOAT DeltaTime)
{
	BEGIN_UPDATE_LOOP;
	{
		PARTICLE_ELEMENT(FEmitterDynamicParameterPayload, DynamicPayload);

		for (INT ParamIndex = 0; ParamIndex < 4; ParamIndex++)
		{
			DynamicPayload.DynamicParameterValue[ParamIndex] = 
				DynamicParams(ParamIndex).bUseEmitterTime ? 
					DynamicParams(ParamIndex).ParamValue.GetValue(Owner->EmitterTime) :
					DynamicParams(ParamIndex).ParamValue.GetValue(Particle.RelativeTime);
		}
	}
	END_UPDATE_LOOP;
}

/**
 *	Returns the number of bytes that the module requires in the particle payload block.
 *
 *	@param	Owner		The FParticleEmitterInstance that 'owns' the particle.
 *
 *	@return	UINT		The number of bytes the module needs per particle.
 */
UINT UParticleModuleParameterDynamic::RequiredBytes(FParticleEmitterInstance* Owner)
{
	return sizeof(FEmitterDynamicParameterPayload);
}

// For Cascade
/**
 *	Called when the module is created, this function allows for setting values that make
 *	sense for the type of emitter they are being used in.
 *
 *	@param	Owner			The UParticleEmitter that the module is being added to.
 */
void UParticleModuleParameterDynamic::SetToSensibleDefaults(UParticleEmitter* Owner)
{
}

/** 
 *	PostEditChange...
 */
void UParticleModuleParameterDynamic::PostEditChange(UProperty* PropertyThatChanged)
{
}

/** 
 *	Fill an array with each Object property that fulfills the FCurveEdInterface interface.
 *
 *	@param	OutCurve	The array that should be filled in.
 */
void UParticleModuleParameterDynamic::GetCurveObjects(TArray<FParticleCurvePair>& OutCurves)
{
	FParticleCurvePair* NewCurve;

	for (INT ParamIndex = 0; ParamIndex < 4; ParamIndex++)
	{
		NewCurve = new(OutCurves) FParticleCurvePair;
		check(NewCurve);
		NewCurve->CurveObject = DynamicParams(ParamIndex).ParamValue.Distribution;
		NewCurve->CurveName = FString::Printf(TEXT("%s (DP%d)"), 
			*(DynamicParams(ParamIndex).ParamName.ToString()), ParamIndex);
	}
}

/**
 *	Retrieve the ParticleSysParams associated with this module.
 *
 *	@param	ParticleSysParamList	The list of FParticleSysParams to add to
 */
void UParticleModuleParameterDynamic::GetParticleSysParamsUtilized(TArray<FString>& ParticleSysParamList)
{
}

/**
 *	Retrieve the distributions that use ParticleParameters in this module.
 *
 *	@param	ParticleParameterList	The list of ParticleParameter distributions to add to
 */
void UParticleModuleParameterDynamic::GetParticleParametersUtilized(TArray<FString>& ParticleParameterList)
{
}

/**
 *	Helper function for retriving the material from interface...
 */
UMaterial* UParticleModuleParameterDynamic_RetrieveMaterial(UMaterialInterface* InMaterialInterface)
{
	UMaterial* Material = Cast<UMaterial>(InMaterialInterface);
	UMaterialInstanceConstant* MIC = Cast<UMaterialInstanceConstant>(InMaterialInterface);
	UMaterialInstanceTimeVarying* MITV = Cast<UMaterialInstanceTimeVarying>(InMaterialInterface);

	if (MIC)
	{
		UMaterialInterface* Parent = MIC->Parent;
		Material = Cast<UMaterial>(Parent);
		while (!Material && Parent)
		{
			MIC = Cast<UMaterialInstanceConstant>(Parent);
			if (MIC)
			{
				Parent = MIC->Parent;
			}
			MITV = Cast<UMaterialInstanceTimeVarying>(Parent);
			if (MITV)
			{
				Parent = MITV->Parent;
			}

			Material = Cast<UMaterial>(Parent);
		}
	}

	if (MITV)
	{
		UMaterialInterface* Parent = MITV->Parent;
		Material = Cast<UMaterial>(Parent);
		while (!Material && Parent)
		{
			MIC = Cast<UMaterialInstanceConstant>(Parent);
			if (MIC)
			{
				Parent = MIC->Parent;
			}
			MITV = Cast<UMaterialInstanceTimeVarying>(Parent);
			if (MITV)
			{
				Parent = MITV->Parent;
			}

			Material = Cast<UMaterial>(Parent);
		}
	}

	return Material;
}

/**
 *	Helper function to find the DynamicParameter expression in a material
 */
UMaterialExpressionDynamicParameter* UParticleModuleParameterDynamic_GetDynamicParameterExpression(UMaterial* InMaterial)
{
	UMaterialExpressionDynamicParameter* DynParamExp = NULL;
	for (INT ExpIndex = 0; ExpIndex < InMaterial->Expressions.Num(); ExpIndex++)
	{
		DynParamExp = Cast<UMaterialExpressionDynamicParameter>(InMaterial->Expressions(ExpIndex));
		if (DynParamExp != NULL)
		{
			break;
		}
	}

	return DynParamExp;
}

/**
 *	Update the parameter names with the given material...
 *
 *	@param	InMaterialInterface	Pointer to the material interface
 *
 */
void UParticleModuleParameterDynamic::UpdateParameterNames(UMaterialInterface* InMaterialInterface)
{
	UMaterial* Material = UParticleModuleParameterDynamic_RetrieveMaterial(InMaterialInterface);
	if (Material == NULL)
	{
		return;
	}

	// Check the expressions...
	UMaterialExpressionDynamicParameter* DynParamExp = UParticleModuleParameterDynamic_GetDynamicParameterExpression(Material);
	if (DynParamExp == NULL)
	{
		return;
	}

	for (INT ParamIndex = 0; ParamIndex < 4; ParamIndex++)
	{
		DynamicParams(ParamIndex).ParamName = FName(*(DynParamExp->ParamNames(ParamIndex)));
	}
}

/**
 *	Refresh the module...
 */
void UParticleModuleParameterDynamic::RefreshModule(UInterpCurveEdSetup* EdSetup, UParticleEmitter* InEmitter, INT InLODLevel)
{
	// Find the material for this emitter...
	UParticleLODLevel* LODLevel = InEmitter->LODLevels((InLODLevel < InEmitter->LODLevels.Num()) ? InLODLevel : 0);
	if (LODLevel)
	{
		UMaterialInterface* MaterialInterface = LODLevel->RequiredModule ? LODLevel->RequiredModule->Material : NULL;
		if (MaterialInterface)
		{
			UpdateParameterNames(MaterialInterface);
			for (INT ParamIndex = 0; ParamIndex < 4; ParamIndex++)
			{
				FString TempName = FString::Printf(TEXT("%s (DP%d)"), 
					*(DynamicParams(ParamIndex).ParamName.ToString()), ParamIndex);
				EdSetup->ChangeCurveName(
					DynamicParams(ParamIndex).ParamValue.Distribution, 
					TempName);
			}
		}
	}
}
