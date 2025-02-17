//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
// Confidential.
//=============================================================================

#include "UnrealEd.h"
#include "GearEditor.h"
#include "GearEditorClasses.h"
#include "GearEditorCommandlets.h"
#include "GearEditorFactories.h"
#include "GearEditorCookerHelper.h"

#define STATIC_LINKING_MOJO 1

// Register things.
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name) FName GEAREDITOR_##name;
#define AUTOGENERATE_FUNCTION(cls,idx,name) IMPLEMENT_FUNCTION(cls,idx,name)
#include "GearEditorClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef AUTOGENERATE_NAME
#undef NAMES_ONLY

// Register natives.
#define NATIVES_ONLY
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name)
#define AUTOGENERATE_FUNCTION(cls,idx,name)
#include "GearEditorClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef AUTOGENERATE_NAME
#undef NATIVES_ONLY
#undef NAMES_ONLY

/**
 * Initialize registrants, basically calling StaticClass() to create the class and also 
 * populating the lookup table.
 *
 * @param	Lookup	current index into lookup table
 */
void AutoInitializeRegistrantsGearEditor( INT& Lookup )
{
	AUTO_INITIALIZE_REGISTRANTS_GEAREDITOR;
}

/**
 * Auto generates names.
 */
void AutoGenerateNamesGearEditor()
{
	#define NAMES_ONLY
	#define AUTOGENERATE_FUNCTION(cls,idx,name)
    #define AUTOGENERATE_NAME(name) GEAREDITOR_##name = FName(TEXT(#name));
	#include "GearEditorClasses.h"
	#undef AUTOGENERATE_FUNCTION
	#undef AUTOGENERATE_NAME
	#undef NAMES_ONLY
}


#include "RemoteControl.h"
#include "RemoteControlPageFactory.h"
#include "RemoteControlGearFlagsPage.h"
#if USING_REMOTECONTROL

static TRemoteControlPageFactory<WxRemoteControlGearFlagsPage> GearRemoteControl;

#endif // USING_REMOTECONTROL



IMPLEMENT_CLASS(UUIContainerThumbnailRenderer);
IMPLEMENT_CLASS(UGearEditorEngine);
IMPLEMENT_CLASS(UGearUnrealEdEngine);



void UGearBloodInfoFactoryNew::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(NAME_Object);
}

/**
* Initializes property values for intrinsic classes.  It is called immediately after the class default object
* is initialized against its archetype, but before any objects of this class are created.
*/
void UGearBloodInfoFactoryNew::InitializeIntrinsicPropertyValues()
{
	SupportedClass		= UGearBloodInfo::StaticClass();
	bCreateNew			= TRUE;
	bEditAfterNew		= TRUE;
	Description			= TEXT("Gear Blood Info");
}

UObject* UGearBloodInfoFactoryNew::FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn)
{
	return StaticConstructObject(Class,InParent,Name,Flags);
}

IMPLEMENT_CLASS(UGearBloodInfoFactoryNew);



void UGenericBrowserType_GearBloodInfo::Init()
{
	SupportInfo.AddItem(FGenericBrowserTypeInfo(UGearBloodInfo::StaticClass(), FColor(128,128,200), NULL, 0, this));
}

IMPLEMENT_CLASS(UGenericBrowserType_GearBloodInfo);





/** Helper function for GetDecalMaterialsFromGame which will add DecalMaterials to the passed in array **/
void GetDecalMaterialsFromDecalData( TArray<UMaterialInterface*>& OutArray, TArrayNoInit<FDecalData> InDecalData )
{
	// now set the DecalMaterial settings
	for( INT Idx = 0; Idx < InDecalData.Num(); ++Idx )
	{
		const FDecalData& DecalData = InDecalData(Idx);
		if( DecalData.DecalMaterial != NULL )
		{
			//warnf( TEXT( "DecalData.DecalMaterial %s"), *DecalData.DecalMaterial->GetFullName() );
			OutArray.AddUniqueItem( DecalData.DecalMaterial );
		}		
	}
}


/** This allows the indiv game to return the set of decal materials it uses based off its internal objects (e.g. PhysicalMaterials)**/
class TArray<UMaterialInterface*> UGearEditorEngine::GetDecalMaterialsFromGame( UObject* InObject ) const
{
	TArray<UMaterialInterface*> Retval;

	//warnf( TEXT("%s"), *InObject->GetFullName() );

	UPhysicalMaterial* PhysMat = Cast<UPhysicalMaterial>(InObject);
	if( PhysMat != NULL )
	{
		UGearPhysicalMaterialProperty* PhysMatProp = Cast<UGearPhysicalMaterialProperty>(PhysMat->PhysicalMaterialProperty);

		if( PhysMatProp != NULL )
		{
			//warnf( TEXT( "Found PhysMatProp") );
			// check DefaultFXInfo
			GetDecalMaterialsFromDecalData( Retval,	PhysMatProp->DefaultFXInfoBallistic.DecalData );
			GetDecalMaterialsFromDecalData( Retval,	PhysMatProp->DefaultFXInfoBallistic.DecalData_AR );
			GetDecalMaterialsFromDecalData( Retval,	PhysMatProp->DefaultFXInfoBallistic.DecalData_NoGore );

			GetDecalMaterialsFromDecalData( Retval,	PhysMatProp->DefaultFXInfoExplosion.DecalData );
			GetDecalMaterialsFromDecalData( Retval,	PhysMatProp->DefaultFXInfoExplosion.DecalData_AR );
			GetDecalMaterialsFromDecalData( Retval,	PhysMatProp->DefaultFXInfoExplosion.DecalData_NoGore );

			// check FXInfos
			for( INT Idx = 0; Idx < PhysMatProp->FXInfoBallistic.Num(); ++Idx )
			{
				const FImpactFXBallistic& Impact = PhysMatProp->FXInfoBallistic(Idx);

				GetDecalMaterialsFromDecalData( Retval,	Impact.DecalData );
				GetDecalMaterialsFromDecalData( Retval,	Impact.DecalData_AR );
				GetDecalMaterialsFromDecalData( Retval,	Impact.DecalData_NoGore );
			}

			// check FXInfos
			for( INT Idx = 0; Idx < PhysMatProp->FXInfoExplosion.Num(); ++Idx )
			{
				const FImpactFXExplosion& Impact = PhysMatProp->FXInfoExplosion(Idx);

				GetDecalMaterialsFromDecalData( Retval,	Impact.DecalData );
				GetDecalMaterialsFromDecalData( Retval,	Impact.DecalData_AR );
				GetDecalMaterialsFromDecalData( Retval,	Impact.DecalData_NoGore );
			}

			// check the blood info
			if( PhysMatProp->BloodInfo != NULL )
			{
				for( INT Idx = 0; Idx < PhysMatProp->BloodInfo->BloodInfo.Num(); ++Idx )
				{
					GetDecalMaterialsFromDecalData( Retval,	PhysMatProp->BloodInfo->BloodInfo(Idx).DecalData );
				}
			}
		}
	}

	return Retval;
}





