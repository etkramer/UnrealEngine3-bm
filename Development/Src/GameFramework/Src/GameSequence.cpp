/**
*
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

#include "GameFramework.h"


IMPLEMENT_CLASS(USeqAct_ModifyProperty)


/**
* Grabs the list of all attached Objects, and then attempts to import any specified property
* values for each one.
*/
void USeqAct_ModifyProperty::Activated()
{
	if (Properties.Num() > 0 && Targets.Num() > 0)
	{
		// for each Object found,
		for (INT Idx = 0; Idx < Targets.Num(); Idx++)
		{
			UObject *Obj = Targets(Idx);
			if (Obj != NULL)
			{
				// iterate through each property
				for (INT propIdx = 0; propIdx < Properties.Num(); propIdx++)
				{
					if (Properties(propIdx).bModifyProperty)
					{
						// find the property field
						UProperty *prop = Cast<UProperty>(Obj->FindObjectField(Properties(propIdx).PropertyName,1));
						if (prop != NULL)
						{
							debugf(TEXT("Applying property %s for object %s"),*prop->GetName(),*Obj->GetName());
							// import the property text for the new Object
							prop->ImportText(*(Properties(propIdx).PropertyValue),(BYTE*)Obj + prop->Offset,0,NULL);
						}
						else
						{
							debugf(TEXT("failed to find property in %s"),*Obj->GetName());
							// auto-add the pawn if property wasn't found on the controller
							if (Cast<AController>(Obj) != NULL)
							{
								Targets.AddUniqueItem(Cast<AController>(Obj)->Pawn);
							}
						}
					}
				}
			}
		}
	}
	else
	{
		debugf(TEXT("no properties/targets %d"),Targets.Num());
	}
}

void USeqAct_ModifyProperty::CheckForErrors()
{
	Super::CheckForErrors();

	if (GWarn != NULL && GWarn->MapCheck_IsActive())
	{
		GWarn->MapCheck_Add(MCTYPE_WARNING, NULL, *FString::Printf(TEXT("\"Modify Property\" is for prototyping only and should be removed (%s)"), *GetPathName()));
	}
}
