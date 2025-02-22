/*=============================================================================
	UnClass.cpp: Object class implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "CorePrivate.h"

#include "UnPropertyTag.h"

//@script patcher
#include "UnScriptPatcher.h"

/*-----------------------------------------------------------------------------
	UField implementation.
-----------------------------------------------------------------------------*/

UField::UField( ENativeConstructor, UClass* InClass, const TCHAR* InName, const TCHAR* InPackageName, EObjectFlags InFlags, UField* InSuperField )
: UObject				( EC_NativeConstructor, InClass, InName, InPackageName, InFlags )
, SuperField			( InSuperField )
, Next					( NULL )
{}
UField::UField( EStaticConstructor, const TCHAR* InName, const TCHAR* InPackageName, EObjectFlags InFlags )
: UObject				( EC_StaticConstructor, InName, InPackageName, InFlags )
, Next					( NULL )
{}
UField::UField( UField* InSuperField )
:	SuperField( InSuperField )
{}
/**
 * Static constructor, called once during static initialization of global variables for native 
 * classes. Used to e.g. register object references for native- only classes required for realtime
 * garbage collection or to associate UProperties.
 */
void UField::StaticConstructor()
{
	UClass* TheClass = GetClass();
	TheClass->EmitObjectReference( STRUCT_OFFSET( UField, SuperField ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( UField, Next ) );
}
UClass* UField::GetOwnerClass() const
{
	const UObject* Obj;
	for( Obj=this; Obj->GetClass()!=UClass::StaticClass(); Obj=Obj->GetOuter() );
	return (UClass*)Obj;
}
UStruct* UField::GetOwnerStruct() const
{
	const UObject* Obj;
	for ( Obj=this; Obj && !Obj->IsA(UStruct::StaticClass()); Obj=Obj->GetOuter() );
	return (UStruct*)Obj;
}
void UField::Bind()
{
}
void UField::PostLoad()
{
	Super::PostLoad();
	Bind();
}
void UField::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );

	Ar << SuperField << Next;
}
INT UField::GetPropertiesSize() const
{
	return 0;
}
UBOOL UField::MergeBools()
{
	return 1;
}
void UField::AddCppProperty( UProperty* Property )
{
	appErrorf(TEXT("UField::AddCppProperty"));
}
void UField::Register()
{
	Super::Register();
	if( SuperField )
	{
		SuperField->ConditionalRegister();
	}
}
IMPLEMENT_CLASS(UField)

/*-----------------------------------------------------------------------------
	UStruct implementation.
-----------------------------------------------------------------------------*/

#if SERIAL_POINTER_INDEX
void *GSerializedPointers[MAX_SERIALIZED_POINTERS];
DWORD GTotalSerializedPointers = 0;
DWORD SerialPointerIndex(void *ptr)
{
    for (DWORD i = 0; i < GTotalSerializedPointers; i++)
    {
        if (GSerializedPointers[i] == ptr)
            return i;
    }
    check(GTotalSerializedPointers < MAX_SERIALIZED_POINTERS);
    GSerializedPointers[GTotalSerializedPointers] = ptr;
    return(GTotalSerializedPointers++);
}
#endif


//
// Constructors.
//
UStruct::UStruct( ENativeConstructor, INT InSize, const TCHAR* InName, const TCHAR* InPackageName, EObjectFlags InFlags, UStruct* InSuperStruct )
:	UField			( EC_NativeConstructor, UClass::StaticClass(), InName, InPackageName, InFlags, InSuperStruct )
,	ScriptText		( NULL )
,	CppText			( NULL )
,	Children		( NULL )
,	PropertiesSize	( InSize )
,	Script			()
,	TextPos			( 0 )
,	Line			( 0 )
,	MinAlignment	( 1 )
,	RefLink			( NULL )
,	PropertyLink	( NULL )
,	ConfigLink	    ( NULL )
,	ConstructorLink	( NULL )
,	ComponentPropertyLink ( NULL )
,	TransientPropertyLink ( NULL )
{}
UStruct::UStruct( EStaticConstructor, INT InSize, const TCHAR* InName, const TCHAR* InPackageName, EObjectFlags InFlags )
:	UField			( EC_StaticConstructor, InName, InPackageName, InFlags )
,	ScriptText		( NULL )
,	CppText			( NULL )
,	Children		( NULL )
,	PropertiesSize	( InSize )
,	Script			()
,	TextPos			( 0 )
,	Line			( 0 )
,	MinAlignment	( 1 )
,	RefLink			( NULL )
,	PropertyLink	( NULL )
,	ConfigLink	    ( NULL )
,	ConstructorLink	( NULL )
,	ComponentPropertyLink ( NULL )
,	TransientPropertyLink ( NULL )
{}
UStruct::UStruct( UStruct* InSuperStruct )
:	UField( InSuperStruct )
,	PropertiesSize( InSuperStruct ? InSuperStruct->GetPropertiesSize() : 0 )
,	MinAlignment( Max(InSuperStruct ? InSuperStruct->GetMinAlignment() : 1,1) )
,	RefLink( NULL )
{}

/**
 * Static constructor called once per class during static initialization via IMPLEMENT_CLASS
 * macro. Used to e.g. emit object reference tokens for realtime garbage collection or expose
 * properties for native- only classes.
 */
void UStruct::StaticConstructor()
{
	UClass* TheClass = GetClass();

	TheClass->EmitObjectReference( STRUCT_OFFSET( UStruct, ScriptText ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( UStruct, CppText ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( UStruct, Children ) );

	//@todo rtgc: most of the below block shouldn't be needed as those point to properties which should already be serialized elsewhere
	TheClass->EmitObjectReference( STRUCT_OFFSET( UStruct, RefLink ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( UStruct, PropertyLink ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( UStruct, ConfigLink ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( UStruct, ConstructorLink ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( UStruct, ComponentPropertyLink ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( UStruct, TransientPropertyLink ) );

	TheClass->EmitObjectArrayReference( STRUCT_OFFSET( UStruct, ScriptObjectReferences ) );
}

//
// Add a property.
//
void UStruct::AddCppProperty( UProperty* Property )
{
	Property->Next = Children;
	Children       = Property;
}

//
// Link offsets.
//
void UStruct::Link( FArchive& Ar, UBOOL Props )
{
//@script patcher (LinkStart)
LinkStart:
	// Link the properties.
	if( Props )
	{
		PropertiesSize	= 0;
		MinAlignment	= 1;
		UStruct* InheritanceSuper = GetInheritanceSuper();
		if( InheritanceSuper )
		{
			Ar.Preload( InheritanceSuper );
			PropertiesSize	= InheritanceSuper->GetPropertiesSize();
			MinAlignment	= InheritanceSuper->GetMinAlignment();
#if XBOX
			// The Xenon compiler doesn't crack open the padding of base classes.
			PropertiesSize	= Align( PropertiesSize, MinAlignment );
#endif
		}

		UProperty* Prev = NULL;
		for( UField* Field=Children; Field; Field=Field->Next )
		{
			// calling Preload here is required in order to load the value of Field->Next
			Ar.Preload( Field );
			if( Field->GetOuter()!=this )
			{
				break;
			}

			UProperty* Property = Cast<UProperty>( Field );
			if( Property )
			{
				Property->Link( Ar, Prev );
				PropertiesSize	= Property->Offset + Property->GetSize();
				Prev			= Property;
				MinAlignment	= Max( MinAlignment, Property->GetMinAlignment() );
			}
		}

		if( GetFName() == NAME_Matrix ||
			GetFName() == NAME_Plane ||
			GetFName() == NAME_SHVector)
		{
			MinAlignment = 16;
		}

		// @todo gcc: it is not apparent when (platform/cpu/compiler/etc) we need to align qwords and doubles
#if	PLATFORM_64BITS
		else if( GetFName() == NAME_QWord )
		{
			MinAlignment = 8;
		}
		else if( GetFName() == NAME_Double )
		{
			MinAlignment = 8;
		}
		else if( GetFName() == NAME_Pointer )
		{
			MinAlignment = 8;
		}
#elif PS3
		else if( GetFName() == NAME_QWord )
		{
			MinAlignment = 8;
		}
		else if( GetFName() == NAME_Double )
		{
			MinAlignment = 8;
		}
#endif
		else if( GetFName() == NAME_Color )
		{
			MinAlignment = 4;
		}
		else
		{
			MinAlignment = Max( MinAlignment, 4 );
		}
	}
	else
	{
		UProperty* Prev = NULL;
		for( UField* Field=Children; Field && Field->GetOuter()==this; Field=Field->Next )
		{
			UProperty* Property = Cast<UProperty>( Field );
			if( Property )
			{
				UBoolProperty*	BoolProperty	= Cast<UBoolProperty>( Property, CLASS_IsAUBoolProperty );
				INT				SavedOffset		= Property->Offset;
				BITFIELD		SavedBitMask	= BoolProperty ? BoolProperty->BitMask : 0;

				Property->Link( Ar, Prev );

				Property->Offset				= SavedOffset;
				Prev							= Property;
				if( BoolProperty )
				{
					BoolProperty->BitMask = SavedBitMask;
				}
			}
		}
	}

	//@{
	//@script patcher
	if ( HasAnyFlags(RF_PendingFieldPatches) && Ar.IsLoading() && Props )
	{
		// there are member properties or functions that will be added by a script patch 
		ULinker* LinkerAr = Ar.GetLinker();
		if ( LinkerAr != NULL )
		{
			checkSlow(LinkerAr == GetLinker());

			// since the list of UFields that we're going to be adding might be out of order, we'll need to sort them first, so that
			// the value of 'Next' for the first field we add is either an existing UField in this struct or NULL (indicating that the
			// new field should be last in the linked list)
			TArray<UField*> UnsortedFields, SortedFields;

			// first, find the exports that contain the fields we're going to add
			for ( INT ExportIndex = LinkerAr->ExportMap.Num() - 1; ExportIndex >= 0; ExportIndex-- )
			{
				FObjectExport& Export = LinkerAr->ExportMap(ExportIndex);
				if ( !Export.HasAnyFlags(EF_ScriptPatcherExport) )
				{
					// reached the last export that was originally part of the linker - stop here
					break;
				}

				if ( Export.OuterIndex == GetLinkerIndex() + 1 )
				{
					if ( Export._Object == NULL )
					{
						GetLinker()->CreateExport(ExportIndex);
					}

					if ( Export._Object != NULL )
					{
						UField* FieldObject = Cast<UField>(Export._Object);
						if ( FieldObject != NULL )
						{
							// calling Preload is required in order to load the value of FieldObject->Next; we must call Preload on all fields that we'll
							// be adding, before attempting to sort them so that they all have the correct values for Next
							Ar.Preload( FieldObject );
							UnsortedFields.AddItem(FieldObject);
						}
					}
				}
			}

			// here we sort the fields according to the order in which they should be added to the struct
			while ( UnsortedFields.Num() > 0 )
			{
				UField* FieldObject = UnsortedFields.Pop();

				// by default, we insert at the end of the list
				INT InsertIndex = SortedFields.Num();
				for ( INT SortedIndex = 0; SortedIndex < SortedFields.Num(); SortedIndex++ )
				{
					UField* SortedField = SortedFields(SortedIndex);

					// if the new field's Next value is this previously processed field, insert the new field just before this one
					if ( FieldObject->Next == SortedField )
					{
						InsertIndex = SortedIndex;
						break;
					}

					// otherwise, if the previously processed field's Next is this object, insert the new field just after this one
					else if ( SortedField->Next == FieldObject )
					{
						// found the correct place for this field
						InsertIndex = SortedIndex + 1;
						break;
					}
				}
				SortedFields.InsertItem(FieldObject, InsertIndex);
			}

			// now the list of fields is sorted; we add the last field first, as it's Next value will correspond to an existing field in the
			// struct (or NULL), which is the only way we can determine where the correct insert position is.
			for ( INT FieldIndex = SortedFields.Num() - 1; FieldIndex >= 0; FieldIndex-- )
			{
				UField* FieldObject = SortedFields(FieldIndex);

				// find the right place for this new export using the value of its Next member, which will be valid since the reference
				// was part of this export's object patch data
				UField* InsertPoint = NULL;

				// if FieldObject->Next is the same value as the struct's Children, we're inserting this new field as the first field thus
				// replacing the value of Children
				if ( FieldObject->Next != Children )
				{
					// go through the list, searching for the field that has the same value for Next as the one we're inserting; then we just
					// change the existing field's Next value to point to the new field to insert it into the list.
					for( UField* ExistingField=Children; ExistingField && ExistingField->GetOuter()==this; ExistingField=ExistingField->Next )
					{
						if ( ExistingField->Next == FieldObject->Next )
						{
							InsertPoint = ExistingField;
							break;
						}
					}
				}

				if ( InsertPoint != NULL )
				{
					InsertPoint->Next = FieldObject;
				}
				else
				{
					// if InsertPoint is NULL, either the struct didn't previously have any members or this field was added to the
					// top (for properties) or bottom (for functions) of the owning struct/class
					Children = FieldObject;
				}
			}
		}


		// clear the flag and relink everything
		ClearFlags(RF_PendingFieldPatches);
		goto LinkStart;
	}
	//@}

#if !__INTEL_BYTE_ORDER__
	// Object.uc declares FColor as BGRA which doesn't match up with what we'd like to use on
	// Xenon to match up directly with the D3D representation of D3DCOLOR. We manually fiddle 
	// with the property offsets to get everything to line up.
	// In any case, on big-endian systems we want to byte-swap this.
	//@todo cooking: this should be moved into the data cooking step.
	if( GetFName() == NAME_Color )
	{
		UProperty*	ColorComponentEntries[4];
		UINT		ColorComponentIndex = 0;

		for( UField* Field=Children; Field && Field->GetOuter()==this; Field=Field->Next )
		{
			UProperty* Property = Cast<UProperty>( Field );
			check(Property);
			ColorComponentEntries[ColorComponentIndex++] = Property;
		}
		check( ColorComponentIndex == 4 );

		Exchange( ColorComponentEntries[0]->Offset, ColorComponentEntries[3]->Offset );
		Exchange( ColorComponentEntries[1]->Offset, ColorComponentEntries[2]->Offset );
	}
#endif

	// Link the references, structs, and arrays for optimized cleanup.
	// Note: Could optimize further by adding UProperty::NeedsDynamicRefCleanup, excluding things like arrays of ints.
	UProperty** PropertyLinkPtr		= &PropertyLink;
	UProperty** ConfigLinkPtr		= &ConfigLink;
	UProperty** ConstructorLinkPtr	= &ConstructorLink;
	UProperty** RefLinkPtr			= (UProperty**)&RefLink;
	UProperty** ComponentPropertyLinkPtr = &ComponentPropertyLink;
	UProperty** TransientPropertyLinkPtr = &TransientPropertyLink;

	for( TFieldIterator<UProperty> It(this); It; ++It)
	{
		UProperty* Property = *It;
		if ( Property->HasAnyPropertyFlags(CPF_Component) )
		{
			*ComponentPropertyLinkPtr = Property;
			ComponentPropertyLinkPtr = &(*ComponentPropertyLinkPtr)->ComponentPropertyLinkNext;
		}
		if( Property->ContainsObjectReference() )
		{
			*RefLinkPtr = Property;
			RefLinkPtr=&(*RefLinkPtr)->NextRef;
		}
		if( Property->HasAnyPropertyFlags(CPF_NeedCtorLink) )
		{
			*ConstructorLinkPtr = Property;
			ConstructorLinkPtr  = &(*ConstructorLinkPtr)->ConstructorLinkNext;
		}
		if( Property->HasAnyPropertyFlags(CPF_Config) )
		{
			*ConfigLinkPtr = Property;
			ConfigLinkPtr  = &(*ConfigLinkPtr)->ConfigLinkNext;
		}
		if( Property->HasAnyPropertyFlags(CPF_Net) && !GIsEditor )
		{
			Property->RepOwner = Property;
			FArchive TempAr;
			INT iCode = Property->RepOffset;
			Property->GetOwnerClass()->SerializeExpr( iCode, TempAr );
		}
		if( Property->HasAnyPropertyFlags(CPF_Transient|CPF_DuplicateTransient) )
		{
			*TransientPropertyLinkPtr = Property;
			TransientPropertyLinkPtr  = &(*TransientPropertyLinkPtr)->TransientPropertyLinkNext;
		}

		*PropertyLinkPtr = Property;
		PropertyLinkPtr  = &(*PropertyLinkPtr)->PropertyLinkNext;
	}
	*PropertyLinkPtr    = NULL;
	*ConfigLinkPtr      = NULL;
	*ConstructorLinkPtr = NULL;
	*RefLinkPtr			= NULL;
	*ComponentPropertyLinkPtr = NULL;
	*TransientPropertyLinkPtr = NULL;
}

/**
 * Serializes the passed in property with the struct's data residing in Data.
 *
 * @param	Property		property to serialize
 * @param	Ar				the archive to use for serialization
 * @param	Data			pointer to the location of the beginning of the struct's property data
 */
void UStruct::SerializeBinProperty( UProperty* Property, FArchive& Ar, BYTE* Data ) const
{
	if( Property->ShouldSerializeValue(Ar) )
	{
		UProperty* OldSerializedProperty = GSerializedProperty;
		for( INT Idx=0; Idx<Property->ArrayDim; Idx++ )
		{
			GSerializedProperty = Property;
			Property->SerializeItem( Ar, Data + Property->Offset + Idx * Property->ElementSize, 0 );
		}
		GSerializedProperty = OldSerializedProperty;
	}
}

//
// Serialize all of the class's data that belongs in a particular
// bin and resides in Data.
//
void UStruct::SerializeBin( FArchive& Ar, BYTE* Data, INT MaxReadBytes ) const
{
	if( Ar.IsObjectReferenceCollector() )
	{
		for( UProperty* RefLinkProperty=RefLink; RefLinkProperty!=NULL; RefLinkProperty=RefLinkProperty->NextRef )
		{
			SerializeBinProperty( RefLinkProperty, Ar, Data );
		}
	}
	else
	{
		for( TFieldIterator<UProperty> It(this); It; ++It )
		{
			UProperty* Property = *It;
			SerializeBinProperty( Property, Ar, Data );
		}
	}
}
/**
 * Serializes the class properties that reside in Data if they differ from the corresponding values in DefaultData
 *
 * @param	Ar				the archive to use for serialization
 * @param	Data			pointer to the location of the beginning of the property data
 * @param	DefaultData		pointer to the location of the beginning of the data that should be compared against
 * @param	DefaultsCount	size of the block of memory located at DefaultData 
 */
void UStruct::SerializeBinEx( FArchive& Ar, BYTE* Data, BYTE* DefaultData, INT DefaultsCount ) const
{
	if ( DefaultData == NULL || DefaultsCount == 0 )
	{
		SerializeBin(Ar, Data, 0);
		return;
	}

	for( TFieldIterator<UProperty,CASTCLASS_UProperty> It(this); It; ++It )
	{
		UProperty* Property = *It;
		if( Property->ShouldSerializeValue(Ar) )
		{
			for( INT Idx=0; Idx<Property->ArrayDim; Idx++ )
			{
				const INT Offset = Property->Offset + Idx * Property->ElementSize;
				if ( !Property->Matches(Data, (Offset + Property->ElementSize <= DefaultsCount) ? DefaultData : NULL, Idx, FALSE, Ar.GetPortFlags()) )
				{
					UProperty* OldSerializedProperty = GSerializedProperty;
					GSerializedProperty = Property;

					Property->SerializeItem( Ar, Data + Offset, 0, DefaultData + Offset );
					
					GSerializedProperty = OldSerializedProperty;
				}
			}
		}
	}
}

void UStruct::SerializeTaggedProperties( FArchive& Ar, BYTE* Data, UStruct* DefaultsStruct, BYTE* Defaults, INT DefaultsCount/*=0*/ ) const
{
	FName PropertyName(NAME_None);

	check(Ar.IsLoading() || Ar.IsSaving() || GIsUCCMake);

	UClass* DefaultsClass = Cast<UClass>(DefaultsStruct);
	UScriptStruct* DefaultsScriptStruct = Cast<UScriptStruct>(DefaultsStruct);

	if( Ar.IsLoading() )
	{
		// Load tagged properties.

		// This code assumes that properties are loaded in the same order they are saved in. This removes a n^2 search 
		// and makes it an O(n) when properties are saved in the same order as they are loaded (default case). In the 
		// case that a property was reordered the code falls back to a slower search.
		UProperty*	Property			= PropertyLink;
		UBOOL		AdvanceProperty		= 0;
		INT			RemainingArrayDim	= Property ? Property->ArrayDim : 0;

		// Load all stored properties, potentially skipping unknown ones.
		while( 1 )
		{
			FPropertyTag Tag;
			Ar << Tag;
			if( Tag.Name == NAME_None )
				break;
			PropertyName = Tag.Name;

			//@{
			//@compatibility
#if 0
			if( DefaultsClass != NULL )
			{
				/*
				example of how to add code to maintain binary compatibility when property names are changed or moved..
				the following example code remaps MaterialExpressionNormalize.Vector to MaterialExpressionNormalize.VectorInput
				(this would be necessary to prevent loss of existing data for MaterialExpressionNormalize.Vector after it has been
				renamed to VectorInput)

				FName ClassName = DefaultsClass->GetFName();
				if( Tag.Name == NAME_Vector && ClassName == NAME_MaterialExpressionNormalize )
				{
					Tag.Name = TEXT("VectorInput");
				}
				*/
			}
			else if ( DefaultsStruct != NULL )
			{
			}
#endif
			//@}

			// Move to the next property to be serialized
			if( AdvanceProperty && --RemainingArrayDim <= 0 )
			{
				Property = Property->PropertyLinkNext;
				// Skip over properties that don't need to be serialized.
				while( Property && !Property->ShouldSerializeValue( Ar ) )
				{
					Property = Property->PropertyLinkNext;
				}
				AdvanceProperty		= 0;
				RemainingArrayDim	= Property ? Property->ArrayDim : 0;
			}

			// If this property is not the one we expect (e.g. skipped as it matches the default value), do the brute force search.
			if( Property == NULL || Property->GetFName() != Tag.Name )
			{
				UProperty* CurrentProperty = Property;
				// Search forward...
				for ( ; Property; Property=Property->PropertyLinkNext )
				{
					if( Property->GetFName() == Tag.Name )
					{
						break;
					}
				}
				// ... and then search from the beginning till we reach the current property if it's not found.
				if( Property == NULL )
				{
					for( Property = PropertyLink; Property && Property != CurrentProperty; Property = Property->PropertyLinkNext )
					{
						if( Property->GetFName() == Tag.Name )
						{
							break;
						}
					}

					if( Property == CurrentProperty )
					{
						// Property wasn't found.
						Property = NULL;
					}
				}

				RemainingArrayDim = Property ? Property->ArrayDim : 0;
			}


			//@{
			//@compatibility
			// Check to see if we are loading an old InterpCurve Struct.
			UBOOL bNeedCurveFixup = FALSE;
			if( Ar.Ver() < VER_NEW_CURVE_AUTO_TANGENTS && Tag.Type == NAME_StructProperty && Cast<UStructProperty>(Property, CLASS_IsAUStructProperty) )
			{
				FName StructName = ((UStructProperty*)Property)->Struct->GetFName();
				if( StructName == NAME_InterpCurveFloat || StructName == NAME_InterpCurveVector2D ||
					StructName == NAME_InterpCurveVector || StructName == NAME_InterpCurveTwoVectors ||
					StructName == NAME_InterpCurveQuat )
				{
					bNeedCurveFixup = TRUE;
				}
			}
			//@}


			UBOOL bSkipSkipWarning = FALSE;

			if( !Property )
			{
				//@{
				//@compatibility
				if ( Tag.Name == NAME_InitChild2StartBone )
				{
					UProperty* NewProperty = FindField<UProperty>(DefaultsClass, TEXT("BranchStartBoneName"));
					if (NewProperty != NULL && NewProperty->IsA(UArrayProperty::StaticClass()) && ((UArrayProperty*)NewProperty)->Inner->IsA(UNameProperty::StaticClass()))
					{
						FName OldName;
						Ar << OldName;
						((TArray<FName>*)(Data + NewProperty->Offset))->AddItem(OldName);
						AdvanceProperty = FALSE;
						continue;
					}
				}
				//@}
				
				debugfSlow( NAME_Warning, TEXT("Property %s of %s not found for package:  %s"), *Tag.Name.ToString(), *GetFullName(), *Ar.GetArchiveName() );
			}
			// editoronly properties should be skipped if we are NOT the editor, or we are 
			// the editor but are cooking for console (editoronly implies notforconsole)
			else if ((Property->PropertyFlags & CPF_EditorOnly) && (!GIsEditor || (GCookingTarget & UE3::PLATFORM_Console)))
			{
				debugfSuppressed(NAME_DevLoad, TEXT("Skipping editor-only property %s"), *Tag.Name.ToString());
				bSkipSkipWarning = TRUE;
			}
			// notforconsole properties should be skipped if we are cooking for a console
			// or we are running on a console
			else if ((Property->PropertyFlags & CPF_NotForConsole) && ((GCookingTarget & UE3::PLATFORM_Console) || CONSOLE))
			{
				debugfSuppressed(NAME_DevLoad, TEXT("Skipping not-for-console property %s"), *Tag.Name.ToString());
				bSkipSkipWarning = TRUE;
			}
			// check for valid array index
			else if( Tag.ArrayIndex >= Property->ArrayDim || Tag.ArrayIndex < 0 )
			{
				debugf( NAME_Warning, TEXT("Array bounds in %s of %s: %i/%i for package:  %s"), *Tag.Name.ToString(), *GetName(), Tag.ArrayIndex, Property->ArrayDim, *Ar.GetArchiveName() );
			}
			else if( Tag.Type==NAME_StrProperty && Property->GetID()==NAME_NameProperty )  
			{ 
				FString str;  
				Ar << str; 
				*(FName*)(Data + Property->Offset + Tag.ArrayIndex * Property->ElementSize ) = FName(*str);  
				AdvanceProperty = TRUE;
				continue; 
			}
			else if ( Tag.Type == NAME_ByteProperty && Property->GetID() == NAME_IntProperty )
			{
				// this property's data was saved as a BYTE, but the property has been changed to an INT.  Since there is no loss of data
				// possible, we can auto-convert to the right type.
				BYTE PreviousValue;

				// de-serialize the previous value
				Ar << PreviousValue;

				// now copy the value into the object's address spaace
				*(INT*)(Data + Property->Offset + Tag.ArrayIndex * Property->ElementSize) = PreviousValue;
				AdvanceProperty = TRUE;
				continue;
			}
			else if( Tag.Type!=Property->GetID() )
			{
				debugf( NAME_Warning, TEXT("Type mismatch in %s of %s - Previous (%s) Current(%s) for package:  %s"), *Tag.Name.ToString(), *GetName(), *Tag.Type.ToString(), *Property->GetID().ToString(), *Ar.GetArchiveName() );
			}
			else if( Tag.Type==NAME_StructProperty && Tag.StructName!=CastChecked<UStructProperty>(Property)->Struct->GetFName() )
			{
				debugf( NAME_Warning, TEXT("Property %s of %s struct type mismatch %s/%s for package:  %s"), *Tag.Name.ToString(), *GetName(), *Tag.StructName.ToString(), *CastChecked<UStructProperty>(Property)->Struct->GetName(), *Ar.GetArchiveName() );
			}
			else if( !Property->ShouldSerializeValue(Ar) )
			{
				debugf( NAME_Warning, TEXT("Property %s of %s is not serializable for package:  %s"), *Tag.Name.ToString(), *GetName(), *Ar.GetArchiveName() );
			}
			else
			{
				//@hack: to allow the components array to always be loaded correctly in the editor, don't serialize it
				UBOOL bSkipProperty =
					!GIsGame													// if we are in the editor
					&& !Ar.IsTransacting()										// and we are not transacting
					&& DefaultsClass != NULL									// and we are serializing object data
					&& Property->GetFName() == NAME_Components					// and this property's name is 'Components'
					&& Property->GetOwnerClass()->GetFName() == NAME_Actor		// and this property is declared in 'Actor'
					&& !((UObject*)Data)->HasAnyFlags(RF_ClassDefaultObject)	// and we aren't serializing the default object
					&& (Ar.GetPortFlags()&PPF_Duplicate) == 0;					// and we aren't duplicating an actor

				if ( !bSkipProperty )
				{
					BYTE* DestAddress = Data + Property->Offset + Tag.ArrayIndex * Property->ElementSize;

					// This property is ok.			
					Tag.SerializeTaggedProperty( Ar, Property, DestAddress, Tag.Size, NULL );


					//@{
					//@compatibility
					// If we're fixing up interp curves, we need to set the curve method property manually.
					if( bNeedCurveFixup )
					{
						UScriptStruct* CurveStruct = Cast<UStructProperty>(Property, CLASS_IsAUStructProperty)->Struct;
						checkSlow(CurveStruct);

						UProperty *CurveMethodProperty = FindField<UByteProperty>(CurveStruct, TEXT("InterpMethod"));

						// Old packages store the interp method value one less than what it should be
						*(BYTE*)((BYTE*)DestAddress + CurveMethodProperty->Offset) = *(BYTE*)((BYTE*)DestAddress + CurveMethodProperty->Offset) + 1;
					}
					//@}


					AdvanceProperty = TRUE;
					continue;
				}
			}

			AdvanceProperty = FALSE;

			// Skip unknown or bad property.
			debugfSlow( bSkipSkipWarning ? NAME_DevLoad : NAME_Warning, TEXT("Skipping %i bytes of type %s for package:  %s"), Tag.Size, *Tag.Type.ToString(), *Ar.GetArchiveName() );
			
			BYTE B;
			for( INT i=0; i<Tag.Size; i++ )
			{
				Ar << B;
			}
		}
	}
	else
	{
		// Find defaults.
		BYTE* DefaultData   = Defaults;

		/** If TRUE, it means that we want to serialize all properties of this struct if any properties differ from defaults */
		UBOOL bUseAtomicSerialization = FALSE;
		if( DefaultsStruct )
		{
			if ( DefaultsClass != NULL )
			{
				if ( DefaultsCount <= 0 )
				{
					UObject* Archetype = DefaultData ? (UObject*)DefaultData : ((UObject*)Data)->GetArchetype();
					if ( Archetype != NULL )
					{
						DefaultsCount = Archetype->GetClass()->GetDefaultsCount();
					}
				}
			}
			else if ( DefaultsScriptStruct != NULL )
			{
				bUseAtomicSerialization = DefaultsScriptStruct->ShouldSerializeAtomically(Ar);
				if ( DefaultsCount <= 0 )
				{
					DefaultsCount = DefaultsScriptStruct->GetDefaultsCount();
				}
			}
		}

		// Save tagged properties.

		// Iterate over properties in the order they were linked and serialize them.
		for( UProperty* Property = PropertyLink; Property; Property = Property->PropertyLinkNext )
		{
			if( Property->ShouldSerializeValue(Ar) )
			{
				//@hack: to allow the components array to always be loaded correctly in the editor, don't serialize it
				if ( !GIsGame													// if we are in the editor
					&& DefaultsClass != NULL									// and we are serializing object data
					&& Property->GetFName() == NAME_Components					// and this property's name is 'Components'
					&& Property->GetOwnerClass()->GetFName() == NAME_Actor		// and this property is declared in 'Actor'
					&& !((UObject*)Data)->HasAnyFlags(RF_ClassDefaultObject)	// and we aren't serializing the default object
					&& (Ar.GetPortFlags()&PPF_Duplicate) == 0)					// and we aren't duplicating an object
				{
					continue;
				}

				PropertyName = Property->GetFName();
				for( INT Idx=0; Idx<Property->ArrayDim; Idx++ )
				{
					const INT Offset = Property->Offset + Idx * Property->ElementSize;
#if !CONSOLE
					// make sure all FRawDistributions are up-to-date (this is NOT a one-time, Ar.Ver()'able thing
					// because we must make sure FDist is up-to-date with after any change to a UDist that wasn't
					// baked out in a GetValue() call)
					if (Property->IsA(UStructProperty::StaticClass()))
					{
						FName StructName = ((UStructProperty*)Property)->Struct->GetFName();
						if (StructName == NAME_RawDistributionFloat)
						{
							FRawDistributionFloat* RawDistribution = (FRawDistributionFloat*)(Data + Offset);
							RawDistribution->Initialize();
						}
						else if (StructName == NAME_RawDistributionVector)
						{
							FRawDistributionVector* RawDistribution = (FRawDistributionVector*)(Data + Offset);
							RawDistribution->Initialize();
						}
					}
#endif

					if( (!IsA(UClass::StaticClass())&&!DefaultData) || !Property->Matches( Data, (Offset+Property->ElementSize<=DefaultsCount) ? DefaultData : NULL, Idx, FALSE, Ar.GetPortFlags()) )
					{
						BYTE* DefaultValue = (!bUseAtomicSerialization && DefaultData && (Offset + Property->ElementSize <= DefaultsCount)) ? DefaultData + Offset : NULL;
						FPropertyTag Tag( Ar, Property, Idx, Data + Offset, DefaultValue );
						Ar << Tag;

						// need to know how much data this call to SerializeTaggedProperty consumes, so mark where we are
						INT DataOffset = Ar.Tell();

						Tag.SerializeTaggedProperty( Ar, Property, Data + Offset, 0, DefaultValue );

						// set the tag's size
						Tag.Size = Ar.Tell() - DataOffset;

						if ( Tag.Size >  0 )
						{
							// mark our current location
							DataOffset = Ar.Tell();

							// go back and re-serialize the size now that we know it
							Ar.Seek(Tag.SizeOffset);
							Ar << Tag.Size;

							// return to the current location
							Ar.Seek(DataOffset);
						}
					}
				}
			}
		}
		FName Temp(NAME_None);
		Ar << Temp;
	}
}
void UStruct::FinishDestroy()
{
	Script.Empty();
	Super::FinishDestroy();
}
void UStruct::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );

	// Serialize stuff.
	Ar << ScriptText << Children;
	Ar << CppText;
	// Compiler info.
	Ar << Line << TextPos;

	//@script patcher: if script patches were applied to our linker, then we need to check to see if this struct's bytecode is being patched.
	// the linker should have a mapping of ExportMap index => bytecode patches, so we can check with negligable performance impact 
	FScriptPatchData* PatchedBytecode = NULL;
	
	// Script code.
	INT ScriptSize = Script.Num();
	if ( Ar.IsLoading() )
	{
		Ar << ScriptSize;

		//@{
		//@script patcher
		ULinker* LinkerAr = Ar.GetLinker();
		if ( LinkerAr != NULL )
		{
			checkSlow(LinkerAr == GetLinker());
			PatchedBytecode = GetLinker()->FindBytecodePatch(GetLinkerIndex());
			if ( PatchedBytecode != NULL )
			{
				// first, increment the file reader beyond the bytecode we would have serialized
				Ar.Seek(Ar.Tell() + ScriptSize);

				// update the number of bytes which will serialized with the number of bytes in the patch, so that we allocate the correct number of elements in the Script array
#if SCRIPT_PATCHER_SECOND_PASS
				ScriptSize = PatchedBytecode->Data.Num();
#else
				ScriptSize = PatchedBytecode->Size;
#endif
			}
		}
		//@}

		Script.Empty( ScriptSize );
		Script.Add( ScriptSize );
	}

	// Ensure that last byte in script code is EX_EndOfScript to work around script debugger implementation.
	else if( Ar.IsSaving() )
	{
		if ( GIsUCCMake && ScriptSize && Ar.GetLinker() != NULL )
		{
			if ( Script(ScriptSize-1) == EX_EndOfScript )
			{
				debugf(TEXT("%s had bytecode that looked like EX_EndOfScript - Script Count: %i"), *GetFullName(), Script.Num());
			}
			Script.AddItem( EX_EndOfScript );
			ScriptSize++;
		}

		Ar << ScriptSize;
	}

	//@script patcher: if there is a bytecode patch for this struct, we'll serialize from there instead of the linker's file reader.  But, we'll need to make sure to
	// increment the linker's pos by the size of the bytecode we would have serialized.
	if ( PatchedBytecode == NULL )
	{
		// no bytecode patch for this struct - serialize normally [i.e. from disk]
		INT iCode = 0;
		while( iCode < ScriptSize )
		{	
			SerializeExpr( iCode, Ar );
		}
		if( iCode != ScriptSize )
		{	
			appErrorf( TEXT("Script serialization mismatch: Got %i, expected %i"), iCode, ScriptSize );
		}
	}
	else
	{
		//@script patcher
		// now we swap the linker's current loader with the archive that contains the object data for the exports that this script patcher added
		FArchive* SavedLoader = GetLinker()->Loader;
		GetLinker()->Loader = GetLinker()->GetScriptPatchArchive();

		// then create a patch reader which will contain only the function's updated bytecode
		FPatchReader BytecodePatcher(*PatchedBytecode);
		BytecodePatcher.SetLoader(GetLinker());

		// and serialize it!
		INT iCode = 0;
		while( iCode < ScriptSize )
		{	
			SerializeExpr( iCode, BytecodePatcher );
		}
		if( iCode != ScriptSize )
		{	
			appErrorf( TEXT("Script serialization mismatch: Got %i, expected %i"), iCode, ScriptSize );
		}

		// now restore the linker's previous loader
		GetLinker()->Loader = SavedLoader;
	}

	if( Ar.IsLoading() )
	{
		// Collect references to objects embedded in script and store them in easily accessible array. This is skipped if
		// the struct is disregarded for GC as the references won't be of any use.
		ScriptObjectReferences.Empty();
		if( !IsDisregardedForGC() )
		{
			FArchiveObjectReferenceCollector ObjectReferenceCollector( &ScriptObjectReferences );
			INT iCode2 = 0;
			while( iCode2 < Script.Num() )
			{	
				SerializeExpr( iCode2, ObjectReferenceCollector );
			}
		}
		// Link the properties.
		Link( Ar, TRUE );
	}
}

/**
 * Used by various commandlets to purge Editor only data from the object.
 * 
 * @param TargetPlatform Platform the object will be saved for (ie PC vs console cooking, etc)
 */
void UStruct::StripData(UE3::EPlatformType TargetPlatform)
{
	Super::StripData(TargetPlatform);

	// Get rid of cpp and script text.
	if ( TargetPlatform != UE3::PLATFORM_Windows )
	{
		// Retain script text when cooking for PC so that modders can compile script.
		ScriptText	= NULL;
	}
	CppText		= NULL;
}

//
// Serialize an expression to an archive.
// Returns expression token.
//
EExprToken UStruct::SerializeExpr( INT& iCode, FArchive& Ar )
{
	EExprToken Expr=(EExprToken)0;
	
	#define XFER(T) {Ar << *(T*)&Script(iCode); iCode += sizeof(T); }

//@script patcher: FNames are two ints so we need to serialize out two of them when patching and not try to get the actual FName.
#if SUPPORTS_SCRIPTPATCH_CREATION
	#define XFERNAME() \
	{ \
		if( !GIsScriptPatcherActive ) \
		{ \
			XFER(FName) \
		} \
		else \
		{ \
			XFER(NAME_INDEX); XFER(INT); \
		} \
	}
#else
	#define XFERNAME() XFER(FName)
#endif

//@script patcher
#if SUPPORTS_SCRIPTPATCH_CREATION
	#define XFERPTR(T) \
	if( !GIsScriptPatcherActive ) \
	{ \
   	    T AlignedPtr = NULL; \
		DWORD TempCode; \
        if (!Ar.IsLoading()) \
		{ \
			appMemcpy( &TempCode, &Script(iCode), sizeof(T) ); \
			AlignedPtr = (T)appDWORDToPointer(TempCode); \
		} \
		Ar << AlignedPtr; \
		if (!Ar.IsSaving()) \
		{ \
			TempCode = appPointerToDWORD(AlignedPtr); \
			appMemcpy( &Script(iCode), &TempCode, sizeof(T) ); \
		} \
		iCode += sizeof(DWORD); \
	} \
	else \
	{ \
		XFER(INT) \
	}
#else
	#define XFERPTR(T) \
	{ \
   	    T AlignedPtr = NULL; \
		DWORD TempCode; \
        if (!Ar.IsLoading()) \
		{ \
			appMemcpy( &TempCode, &Script(iCode), sizeof(T) ); \
			AlignedPtr = (T)appDWORDToPointer(TempCode); \
		} \
		Ar << AlignedPtr; \
		if (!Ar.IsSaving()) \
		{ \
			TempCode = appPointerToDWORD(AlignedPtr); \
			appMemcpy( &Script(iCode), &TempCode, sizeof(T) ); \
		} \
		iCode += sizeof(DWORD); \
	}
#endif


#if !CONSOLE
	//DEBUGGER: To mantain compatability between debug and non-debug clases
	#define HANDLE_OPTIONAL_DEBUG_INFO \
    if (iCode < Script.Num()) \
    { \
	    int RemPos = Ar.Tell(); \
	    int OldiCode = iCode;	\
	    XFER(BYTE); \
	    int NextCode = Script(iCode-1); \
	    int GVERSION = -1;	\
	    if ( NextCode == EX_DebugInfo ) \
	    {	\
		    XFER(INT); \
		    GVERSION = *(INT*)&Script(iCode-sizeof(INT));	\
	    }	\
	    iCode = OldiCode;	\
	    Ar.Seek( RemPos );	\
	    if ( GVERSION == 100 )	\
		    SerializeExpr( iCode, Ar );	\
    }
#else
	// Console builds cannot deal with debug classes.
	#define HANDLE_OPTIONAL_DEBUG_INFO (void(0))
#endif

	// Get expr token.
	XFER(BYTE);
	Expr = (EExprToken)Script(iCode-1);
	if( Expr >= EX_FirstNative )
	{
		// Native final function with id 1-127.
		while( SerializeExpr( iCode, Ar ) != EX_EndFunctionParms );
		HANDLE_OPTIONAL_DEBUG_INFO; //DEBUGGER
	}
	else if( Expr >= EX_ExtendedNative )
	{
		// Native final function with id 256-16383.
		XFER(BYTE);
		while( SerializeExpr( iCode, Ar ) != EX_EndFunctionParms );
		HANDLE_OPTIONAL_DEBUG_INFO; //DEBUGGER
	}
	else switch( Expr )
	{
		case EX_PrimitiveCast:
		{
			// A type conversion.
			XFER(BYTE); //which kind of conversion
			SerializeExpr( iCode, Ar );
			break;
		}
		case EX_InterfaceCast:
		{
			// A conversion from an object varible to a native interface variable.  We use a different bytecode to avoid the branching each time we process a cast token
			XFERPTR(UClass*);	// the interface class to convert to
			SerializeExpr( iCode, Ar );
			break;
		}
		case EX_Let:
		case EX_LetBool:
		case EX_LetDelegate:
		{
			SerializeExpr( iCode, Ar ); // Variable expr.
			SerializeExpr( iCode, Ar ); // Assignment expr.
			break;
		}
		case EX_Jump:
		{
			XFER(WORD); // Code offset.
			break;
		}
		case EX_LocalVariable:
		case EX_InstanceVariable:
		case EX_DefaultVariable:
		case EX_LocalOutVariable:
		{
			XFERPTR(UProperty*);
			break;
		}
		case EX_DebugInfo:
		{
			XFER(INT);	// Version
			XFER(INT);	// Line number
			XFER(INT);	// Character pos
			XFER(BYTE); // OpCode
			break;
		}
		case EX_BoolVariable:
		case EX_InterfaceContext:
		{
			SerializeExpr(iCode,Ar);
			break;
		}
		case EX_Nothing:
		case EX_EndOfScript:
		case EX_EndFunctionParms:
		case EX_EmptyParmValue:
		case EX_IntZero:
		case EX_IntOne:
		case EX_True:
		case EX_False:
		case EX_NoObject:
		case EX_EmptyDelegate:
		case EX_Self:
		case EX_IteratorPop:
		case EX_Stop:
		case EX_IteratorNext:
		case EX_EndParmValue:
		{
			break;
		}
		case EX_ReturnNothing:
		{
			XFERPTR(UProperty*); // the return value property
			break;
		}
		case EX_EatReturnValue:
		{
			XFERPTR(UProperty*); // the return value property
			break;
		}
		case EX_Return:
		{
			SerializeExpr( iCode, Ar ); // Return expression.
			break;
		}
		case EX_FinalFunction:
		{
			XFERPTR(UStruct*); // Stack node.
			while( SerializeExpr( iCode, Ar ) != EX_EndFunctionParms ); // Parms.
			HANDLE_OPTIONAL_DEBUG_INFO; //DEBUGGER
			break;
		}
		case EX_VirtualFunction:
		case EX_GlobalFunction:
		{
			XFERNAME(); // Virtual function name.
			while( SerializeExpr( iCode, Ar ) != EX_EndFunctionParms ); // Parms.
			HANDLE_OPTIONAL_DEBUG_INFO; //DEBUGGER
			break;
		}
		case EX_DelegateFunction:
		{
			XFER(BYTE); // local prop
			XFERPTR(UProperty*);	// Delegate property
			XFERNAME();		// Delegate function name (in case the delegate is NULL)
			break;
		}
		case EX_NativeParm:
		{
			XFERPTR(UProperty*);
			break;
		}
		case EX_ClassContext:
		case EX_RSSContext:
		case EX_Context:
		{
			SerializeExpr( iCode, Ar ); // Object expression.
			XFER(WORD);					// Code offset for NULL expressions.
			XFER(VariableSizeType);		// Zero-fill size if skipped.
			SerializeExpr( iCode, Ar ); // Context expression.
			break;
		}
		case EX_DynArrayIterator:
		{
			SerializeExpr( iCode, Ar ); // Array expression
			SerializeExpr( iCode, Ar ); // Array item expression
			XFER(BYTE);	// Index parm present
			SerializeExpr( iCode, Ar ); // Index parm
			XFER(WORD); // Code offset
			break;
		}
		case EX_ArrayElement:
		case EX_DynArrayElement:
		{
			SerializeExpr( iCode, Ar ); // Index expression.
			SerializeExpr( iCode, Ar ); // Base expression.
			break;
		}
		case EX_DynArrayLength:
		{
			SerializeExpr( iCode, Ar ); // Base expression.
			break;
		}
		case EX_DynArrayAdd:
		{
			SerializeExpr( iCode, Ar ); // Base expression
			SerializeExpr( iCode, Ar );	// Count
			break;
		}
		case EX_DynArrayInsert:
		case EX_DynArrayRemove:
		{
			SerializeExpr( iCode, Ar ); // Base expression
			SerializeExpr( iCode, Ar ); // Index
			SerializeExpr( iCode, Ar ); // Count
			break;
 		}
		case EX_DynArrayAddItem:
		case EX_DynArrayRemoveItem:
		{
			SerializeExpr( iCode, Ar ); // Array property expression
			XFER(WORD);					// Number of bytes to skip if NULL context encountered
			SerializeExpr( iCode, Ar ); // Item
			break;
		}
		case EX_DynArrayInsertItem:
		{
			SerializeExpr( iCode, Ar );	// Base expression
			XFER(WORD);					// Number of bytes to skip if NULL context encountered
			SerializeExpr( iCode, Ar ); // Index
			SerializeExpr( iCode, Ar );	// Item
			break;
		}
		case EX_DynArrayFind:
		{
			SerializeExpr( iCode, Ar ); // Array property expression
			XFER(WORD);					// Number of bytes to skip if NULL context encountered
			SerializeExpr( iCode, Ar ); // Search item
			break;
		}
		case EX_DynArrayFindStruct:
		{
			SerializeExpr( iCode, Ar ); // Array property expression
			XFER(WORD);					// Number of bytes to skip if NULL context encountered
			SerializeExpr( iCode, Ar );	// Property name
			SerializeExpr( iCode, Ar ); // Search item
			break;
		}
		case EX_Conditional:
		{
			SerializeExpr( iCode, Ar ); // Bool Expr
			XFER(WORD); // Skip
			SerializeExpr( iCode, Ar ); // Result Expr 1
			XFER(WORD); // Skip2
			SerializeExpr( iCode, Ar ); // Result Expr 2
			break;
		}
		case EX_New:
		{
			SerializeExpr( iCode, Ar ); // Parent expression.
			SerializeExpr( iCode, Ar ); // Name expression.
			SerializeExpr( iCode, Ar ); // Flags expression.
			SerializeExpr( iCode, Ar ); // Class expression.
			break;
		}
		case EX_IntConst:
		{
			XFER(INT);
			break;
		}
		case EX_FloatConst:
		{
			XFER(FLOAT);
			break;
		}
		case EX_StringConst:
		{
			do XFER(BYTE) while( Script(iCode-1) );
			break;
		}
		case EX_UnicodeStringConst:
		{
			do XFER(WORD) while( Script(iCode-1) );
			break;
		}
		case EX_ObjectConst:
		{
			XFERPTR(UObject*);
			break;
		}
		case EX_NameConst:
		{
			XFERNAME();
			break;
		}
		case EX_RotationConst:
		{
			XFER(INT); XFER(INT); XFER(INT);
			break;
		}
		case EX_VectorConst:
		{
			XFER(FLOAT); XFER(FLOAT); XFER(FLOAT);
			break;
		}
		case EX_ByteConst:
		case EX_IntConstByte:
		{
			XFER(BYTE);
			break;
		}
		case EX_MetaCast:
		{
			XFERPTR(UClass*);
			SerializeExpr( iCode, Ar );
			break;
		}
		case EX_DynamicCast:
		{
			XFERPTR(UClass*);
			SerializeExpr( iCode, Ar );
			break;
		}
		case EX_JumpIfNot:
		{
			XFER(WORD); // Code offset.
			SerializeExpr( iCode, Ar ); // Boolean expr.
			break;
		}
		case EX_Iterator:
		{
			SerializeExpr( iCode, Ar ); // Iterator expr.
			XFER(WORD); // Code offset.
			break;
		}
		case EX_Switch:
		{
			XFER(VariableSizeType);		// Value size.
			SerializeExpr( iCode, Ar ); // Switch expr.
			break;
		}
		case EX_Assert:
		{
			XFER(WORD); // Line number.
			XFER(BYTE); // debug mode or not
			SerializeExpr( iCode, Ar ); // Assert expr.
			break;
		}
		case EX_Case:
		{
			WORD W;
//			WORD* W=(WORD*)&Script(iCode);
			XFER(WORD); // Code offset.
			appMemcpy(&W, &Script(iCode-sizeof(WORD)), sizeof(WORD));
			if( W != MAXWORD )
				SerializeExpr( iCode, Ar ); // Boolean expr.
			break;
		}
		case EX_LabelTable:
		{
			check((iCode&3)==0);
#if SUPPORTS_SCRIPTPATCH_CREATION
			//@script patcher
			if ( !GIsScriptPatcherActive )
			{
#endif
				for( ; ; )
				{
					FLabelEntry* E = (FLabelEntry*)&Script(iCode);
					XFER(FLabelEntry);
					if( E->Name == NAME_None )
						break;
				}
#if SUPPORTS_SCRIPTPATCH_CREATION
			}
			else
			{
				//@script patcher: when script patches are being generated, serialized FName indexes aren't converted into actual FNames,
				// so in order to determine whether this label is the "break" label, we need to look at the linker's NameMap
				for( ; ; )
				{
					FLabelEntry* E = (FLabelEntry*)&Script(iCode);
					XFER(FLabelEntry);
					const INT NameIndex = E->Name.GetIndex();
					if ( GetLinker() )
					{
						if ( GetLinker()->NameMap(NameIndex) == NAME_None )
						{
							break;
						}
					}
					else
					{
						if ( E->iCode == MAXWORD )
						{
							break;
						}
					}
				}
			}
#endif
			break;
		}
		case EX_GotoLabel:
		{
			SerializeExpr( iCode, Ar ); // Label name expr.
			break;
		}
		case EX_Skip:
		{
			XFER(WORD);					// Skip size.
			SerializeExpr( iCode, Ar ); // Expression to possibly skip.
			break;
		}
		case EX_DefaultParmValue:
		{
			XFER(WORD); // Size of the expression for this default parameter - used by the VM to skip over the expression
						// if a value was specified in the function call

			HANDLE_OPTIONAL_DEBUG_INFO;	// DI_NewStack
			SerializeExpr( iCode, Ar ); // Expression for this default parameter value
			HANDLE_OPTIONAL_DEBUG_INFO;	// DI_PrevStack
			XFER(BYTE);	// EX_EndParmValue
			break;
		}
		case EX_StructCmpEq:
		case EX_StructCmpNe:
		{
			XFERPTR(UStruct*); // Struct.
			SerializeExpr( iCode, Ar ); // Left expr.
			SerializeExpr( iCode, Ar ); // Right expr.
			break;
		}
		case EX_StructMember:
		{
			XFERPTR(UProperty*);		// the struct property we're accessing
			XFERPTR(UStruct*);			// the struct which contains the property
			XFER(BYTE);					// byte indicating whether a local copy of the struct must be created in order to access the member property
			XFER(BYTE);					// byte indicating whether the struct member will be modified by the expression it's being used in
			SerializeExpr( iCode, Ar ); // expression corresponding to the struct member property.
			break;
		}
		case EX_DelegateProperty:
		{
			XFERNAME();	// Name of function we're assigning to the delegate.
			break;
		}
		case EX_InstanceDelegate:
		{
			XFERNAME();	// the name of the function assigned to the delegate
			break;
		}
		case EX_EqualEqual_DelDel:
		case EX_NotEqual_DelDel:
		case EX_EqualEqual_DelFunc:
		case EX_NotEqual_DelFunc:
		{
			while( SerializeExpr( iCode, Ar ) != EX_EndFunctionParms );
			HANDLE_OPTIONAL_DEBUG_INFO; //DEBUGGER
			break;
		}
		default:
		{
			// This should never occur.
			appErrorf( TEXT("Bad expr token %02x"), (BYTE)Expr );
			break;
		}
	}
	return Expr;
	#undef XFER
	#undef XFERPTR
	#undef XFERNAME
}

void UStruct::PostLoad()
{
	Super::PostLoad();
}

/**
 * Creates new copies of components
 * 
 * @param	Data						pointer to the address of the UComponent referenced by this UComponentProperty
 * @param	DefaultData					pointer to the address of the default value of the UComponent referenced by this UComponentProperty
 * @param	DefaultsCount		the size of the buffer pointed to by DefaultValue
 * @param	Owner						the object that contains the component currently located at Data
 * @param	InstanceFlags				contains the mappings of instanced objects and components to their templates
 */
void UStruct::InstanceComponentTemplates( BYTE* Data, BYTE* DefaultData, INT DefaultsCount, UObject* Owner, FObjectInstancingGraph* InstanceGraph )
{
	checkSlow(Data);
	checkSlow(Owner);

	for ( UProperty* Property = ComponentPropertyLink; Property != NULL; Property = Property->ComponentPropertyLinkNext )
	{
		Property->InstanceComponents( Data + Property->Offset, (DefaultData && (Property->Offset < DefaultsCount)) ? DefaultData + Property->Offset : NULL, Owner, InstanceGraph );
	}
}

/**
 * Instances any UObjectProperty values that still match the default value.
 *
 * @param	Value				the address where the pointers to the instanced object should be stored.  This should always correspond to (for class member properties) the address of the
 *								UObject which contains this data, or (for script structs) the address of the struct's data
 *								address of the struct's data
 * @param	DefaultValue		the address where the pointers to the default value is stored.  Evaluated the same way as Value
 * @param	DefaultsCount		the size of the buffer pointed to by DefaultValue
 * @param	OwnerObject			the object that contains the destination data.  Will be the used as the Outer for any newly instanced subobjects.
 * @param	InstanceGraph		contains the mappings of instanced objects and components to their templates
 */
void UStruct::InstanceSubobjectTemplates( BYTE* Value, BYTE* DefaultValue, INT DefaultsCount, UObject* OwnerObject, FObjectInstancingGraph* InstanceGraph/*=NULL*/ ) const
{
	checkSlow(Value);
	checkSlow(OwnerObject);

	for ( UProperty* Property = RefLink; Property != NULL; Property = Property->NextRef )
	{
		if ( Property->ContainsInstancedObjectProperty() )
		{
			Property->InstanceSubobjects(Value + Property->Offset, (DefaultValue && (Property->Offset < DefaultsCount)) ? DefaultValue + Property->Offset : NULL, OwnerObject, InstanceGraph);
		}
	}
}

void UStruct::PropagateStructDefaults()
{
	// flag any functions which contain struct properties that have defaults
	for( TFieldIterator<UFunction,CASTCLASS_UFunction,0> Functions(this); Functions; ++Functions )
	{
		UFunction* Function = *Functions;
		for ( TFieldIterator<UStructProperty,CASTCLASS_UStructProperty,0> Parameters(Function); Parameters; ++Parameters )
		{
			UStructProperty* Prop = *Parameters;
			if ( (Prop->PropertyFlags&CPF_Parm) == 0 && Prop->Struct->GetDefaultsCount() > 0 )
			{
				Function->FunctionFlags |= FUNC_HasDefaults;
				break;
			}
		}
	}
}

IMPLEMENT_CLASS(UStruct);

/*-----------------------------------------------------------------------------
	UScriptStruct.
-----------------------------------------------------------------------------*/
UScriptStruct::UScriptStruct( ENativeConstructor, INT InSize, const TCHAR* InName, const TCHAR* InPackageName, EObjectFlags InFlags, UScriptStruct* InSuperStruct )
:	UStruct			( EC_NativeConstructor, InSize, InName, InPackageName, InFlags, InSuperStruct )
,	StructDefaults	()
{}
UScriptStruct::UScriptStruct( EStaticConstructor, INT InSize, const TCHAR* InName, const TCHAR* InPackageName, EObjectFlags InFlags )
:	UStruct			( EC_StaticConstructor, InSize, InName, InPackageName, InFlags )
,	StructDefaults	()
{}
UScriptStruct::UScriptStruct( UScriptStruct* InSuperStruct )
:	UStruct( InSuperStruct )
,	StructDefaults	()
{}

void UScriptStruct::AllocateStructDefaults()
{
	// We must use the struct's aligned size so that if Struct's aligned size is larger than its PropertiesSize, we don't overrun the defaults when
	// UStructProperty::CopyCompleteValue performs an appMemcpy using the struct property's ElementSize (which is always aligned)
	const INT BufferSize = Align(GetPropertiesSize(),GetMinAlignment());

	StructDefaults.Empty( BufferSize );
	StructDefaults.AddZeroed( BufferSize );
}

void UScriptStruct::Serialize( FArchive& Ar )
{
	Super::Serialize(Ar);

	// serialize the struct's flags
	Ar << StructFlags;

	// serialize the struct's defaults

	// look to see if our parent struct is a script struct, and has any defaults
	BYTE* SuperDefaults = Cast<UScriptStruct>(GetSuperStruct()) ? ((UScriptStruct*)GetSuperStruct())->GetDefaults() : NULL;

	// mark the archive we are serializing defaults
	Ar.StartSerializingDefaults();
	if( Ar.IsLoading() || Ar.IsSaving() )
	{
		// Allocate struct defaults on load.
		if( Ar.IsLoading() )
		{
			AllocateStructDefaults();
		}
		// Ensure struct defaults has enough memory to hold data.
		else
		{
			check(StructDefaults.Num()==Align(GetPropertiesSize(),GetMinAlignment()));
		}

		if( Ar.WantBinaryPropertySerialization() )
		{
			SerializeBin( Ar, &StructDefaults(0), 0 );
		}
		else
		{
			SerializeTaggedProperties( Ar, &StructDefaults(0), GetSuperStruct(), SuperDefaults );
		}
	}
	else
	{
		if( StructDefaults.Num() == 0 )
		{
			check(StructDefaults.Num()==Align(GetPropertiesSize(),GetMinAlignment()));
		}

		StructDefaults.CountBytes( Ar );
		SerializeBin( Ar, &StructDefaults(0), 0 );
	}

	// mark the archive we that we are no longer serializing defaults
	Ar.StopSerializingDefaults();
}

void UScriptStruct::PropagateStructDefaults()
{
	BYTE* DefaultData = GetDefaults();
	if ( DefaultData != NULL )
	{
		for( TFieldIterator<UStructProperty,CASTCLASS_UStructProperty,0> It(this); It; ++It )
		{
			UStructProperty* StructProperty = *It;

			// don't overwrite the values of properties which are marked native, since these properties
			// cannot be serialized by script.  For example, this would otherwise overwrite the 
			// VfTableObject property of UObject, causing all UObjects to have a NULL v-table.
			if ( (StructProperty->PropertyFlags&CPF_Native) == 0 )
			{
				StructProperty->InitializeValue( DefaultData + StructProperty->Offset );
			}
		}
	}

	Super::PropagateStructDefaults();
}

void UScriptStruct::FinishDestroy()
{
	DefaultStructPropText=TEXT("");
	Super::FinishDestroy();
}
IMPLEMENT_CLASS(UScriptStruct);

/*-----------------------------------------------------------------------------
	UState.
-----------------------------------------------------------------------------*/

UState::UState( UState* InSuperState )
: UStruct( InSuperState )
{}

UState::UState( ENativeConstructor, INT InSize, const TCHAR* InName, const TCHAR* InPackageName, EObjectFlags InFlags, UState* InSuperState )
: UStruct( EC_NativeConstructor, InSize, InName, InPackageName, InFlags, InSuperState )
, ProbeMask( 0 )
, IgnoreMask( 0 )
, StateFlags( 0 )
, LabelTableOffset( 0 )
{}

UState::UState( EStaticConstructor, INT InSize, const TCHAR* InName, const TCHAR* InPackageName, EObjectFlags InFlags )
: UStruct( EC_StaticConstructor, InSize, InName, InPackageName, InFlags )
, ProbeMask( 0 )
, IgnoreMask( 0 )
, StateFlags( 0 )
, LabelTableOffset( 0 )
{}

/**
 * Static constructor called once per class during static initialization via IMPLEMENT_CLASS
 * macro. Used to e.g. emit object reference tokens for realtime garbage collection or expose
 * properties for native- only classes.
 */
void UState::StaticConstructor()
{
	UClass* TheClass = GetClass();
	//@todo rtgc: UState::TMap<FName,UFunction*> FuncMap;
}

void UState::Serialize( FArchive& Ar )
{
	//@script patcher (bRebuildFunctionMap)
	const UBOOL bRebuildFunctionMap = HasAnyFlags(RF_PendingFieldPatches);
	Super::Serialize( Ar );

	Ar.ThisContainsCode();

	// Class/State-specific union info.
	Ar << ProbeMask << IgnoreMask;
	Ar << LabelTableOffset << StateFlags;
	// serialize the function map
	//@todo ronp - why is this even serialized anyway?
	Ar << FuncMap;

	//@script patcher
	if ( bRebuildFunctionMap )
	{
		for ( TFieldIterator<UFunction,CASTCLASS_UFunction,FALSE> It(this); It; ++It )
		{
			// if this is a UFunction, we must also add it to the owning struct's lookup map
			FuncMap.Set(It->GetFName(),*It);
		}
	}
}
IMPLEMENT_CLASS(UState);

/*-----------------------------------------------------------------------------
	UClass implementation.
-----------------------------------------------------------------------------*/

/**
 * Static constructor called once per class during static initialization via IMPLEMENT_CLASS
 * macro. Used to e.g. emit object reference tokens for realtime garbage collection or expose
 * properties for native- only classes.
 */
void UClass::StaticConstructor()
{
	UClass* TheClass = GetClass();
	TheClass->EmitObjectReference( STRUCT_OFFSET( UClass, ClassWithin ) );
	TheClass->EmitObjectArrayReference( STRUCT_OFFSET( UClass, NetFields ) );
	//@todo rtgc: I don't believe we need to handle TArray<FRepRecord> UClass::ClassReps;
	TheClass->EmitObjectReference( STRUCT_OFFSET( UClass, ClassDefaultObject ) );
}

/**
 * Callback used to allow object register its direct object references that are not already covered by
 * the token stream.
 *
 * @param ObjectArray	array to add referenced objects to via AddReferencedObject
 */
void UClass::AddReferencedObjects( TArray<UObject*>& ObjectArray )
{
	Super::AddReferencedObjects( ObjectArray );
	for( TMap<FName,UComponent*>::TIterator It(ComponentNameToDefaultObjectMap); It; ++It )
	{
		AddReferencedObject( ObjectArray, It.Value() );
	}
	for ( TMap<UClass*,UProperty*>::TIterator It(Interfaces); It; ++It )
	{
		AddReferencedObject( ObjectArray, It.Key() );
	}
}

UObject* UClass::GetDefaultObject( UBOOL bForce /* = FALSE */ )
{
	if ( ClassDefaultObject == NULL )
	{
		UBOOL bCreateObject = bForce;
		if ( !bCreateObject )
		{
			// when running make, only create default objects for intrinsic classes
			if ( !GIsUCCMake )
			{
				bCreateObject = TRUE;
			}
			else
			{
				bCreateObject = HasAnyClassFlags(CLASS_Intrinsic) || (GUglyHackFlags&HACK_ClassLoadingDisabled) != 0;
			}
		}

		if ( bCreateObject )
		{
			QWORD LoadFlags = RF_Public|RF_ClassDefaultObject|RF_NeedLoad;

			UClass* ParentClass = GetSuperClass();
			UObject* ParentDefaultObject = NULL;
			if ( ParentClass != NULL )
			{
				ParentDefaultObject = ParentClass->GetDefaultObject(bForce);
			}

			if ( ParentDefaultObject != NULL || this == UObject::StaticClass() )
			{
				ClassDefaultObject = StaticConstructObject(this, GetOuter(), NAME_None, LoadFlags, ParentDefaultObject);

				// Perform static construction.
				if( HasAnyFlags(RF_Native) && ClassDefaultObject != NULL )
				{
					// only allowed to not have a static constructor during make
					check(ClassStaticConstructor||GIsUCCMake);
					if ( ClassStaticConstructor != NULL &&
						(!GetSuperClass() || GetSuperClass()->ClassStaticConstructor!=ClassStaticConstructor) )
					{
						(ClassDefaultObject->*ClassStaticConstructor)();
					}

					ConditionalLink();
				}
			}
		}
	}
	return ClassDefaultObject;
}

//
// Register the native class.
//
void UClass::Register()
{
	Super::Register();

	// Get stashed registration info.
	const TCHAR* InClassConfigName = *(TCHAR**)&ClassConfigName;
	ClassConfigName = InClassConfigName;

	// Propagate inherited flags.
	if( SuperField )
	{
		UClass* SuperClass = GetSuperClass();
		ClassFlags |= (SuperClass->ClassFlags & CLASS_Inherit);
		ClassCastFlags |= SuperClass->ClassCastFlags;
	}

	// Ensure that native classes receive a default object as soon as they are registered, so that
	// the class static constructor can be called (which uses the class default object) immediately.
	GetDefaultObject();
}

/**
 * Ensures that UClass::Link() isn't called until it is valid to do so.  For intrinsic classes, this shouldn't occur
 * until their non-intrinsic parents have been fully loaded (otherwise the intrinsic class's UProperty linked lists
 * won't contain any properties from the parent class)
 */
void UClass::ConditionalLink()
{
	// We need to know whether we're running the make commandlet before allowing any classes to be linked, since classes must
	// be allowed to link prior to serialization during make (since .u files may not exist).  However, GIsUCCMake can't be
	// set until after we've checked for outdated script files - otherwise, GIsUCCMake wouldn't be set if the user was attempting
	// to load a map and the .u files were outdated.  The check for outdated script files required GConfig, which isn't initialized until
	// appInit() returns;  appInit() calls UObject::StaticInit(), which registers all classes contained in Core.  Therefore, we need to delay
	// any linking until we can be sure that we're running make or not - by checking for GSys (which is set immediately after appInit() returns)
	// we can tell whether the value of GUglyHackFlags is reliable or not.
	if ( GSys != NULL && bNeedsPropertiesLinked )
	{
		// script classes are normally linked at the end of UClass::Serialize, so only intrinsic classes should be
		// linked manually.  During make, .u files may not exist, so allow all classes to be linked from here.
		if ( HasAnyClassFlags(CLASS_Intrinsic) || (GUglyHackFlags&HACK_ClassLoadingDisabled) != 0 )
		{
			UBOOL bReadyToLink = TRUE;

			UClass* ParentClass = GetSuperClass();
			if ( ParentClass != NULL )
			{
				ParentClass->ConditionalLink();

				// we're not ready to link until our parent has successfully linked, or we aren't able to load classes from disk
				bReadyToLink = ParentClass->PropertyLink != NULL || (GUglyHackFlags&HACK_ClassLoadingDisabled) != 0;
			}

			if ( bReadyToLink )
			{
				// we must have a class default object by this point.
				checkSlow(ClassDefaultObject!=NULL);
				if ( SuperField && HasAnyClassFlags(CLASS_Intrinsic) )
				{
					// re-propagate the class flags from the parent class, so that any class flags that were loaded from disk will be
					// correctly inherited
					UClass* SuperClass = GetSuperClass();
					ClassFlags |= (SuperClass->ClassFlags & CLASS_Inherit);
					ClassCastFlags |= SuperClass->ClassCastFlags;
				}

				// now link the class, which sets up PropertyLink, ConfigLink, ConstructorLink, etc.
				FArchive DummyAr;
				Link(DummyAr,FALSE);

				// we may have inherited some properties which require construction (arrays, strings, etc.),
				// so now we should reinitialize the class default object against its archetype.
				ClassDefaultObject->InitClassDefaultObject(this);
				if ( ClassStaticInitializer != NULL &&
					(ParentClass == NULL || ParentClass->ClassStaticInitializer!=ClassStaticInitializer) )
				{
					// now that we've linked the class and constructed our own versions of any ctor props,
					// we're ready to allow the intrinsic classes to initialize their values from C++
					(ClassDefaultObject->*ClassStaticInitializer)();
				}

				// ok, everything is now hooked up - all UProperties of this class are linked in (including inherited properties)
				// and all values have been initialized (where desired)...now we're ready to load config and localized values
				if ( !GIsUCCMake )
				{
					ClassDefaultObject->LoadConfig();
					ClassDefaultObject->LoadLocalized();
				}
			}
		}
	}
}

/**
 * Find the class's native constructor.
 */
void UClass::Bind()
{
	UStruct::Bind();
	check(GIsEditor || GetSuperClass() || this==UObject::StaticClass());
	if( !ClassConstructor && HasAnyFlags(RF_Native) )
	{
		// Find the native implementation.
		TCHAR ProcName[MAX_SPRINTF]=TEXT("");
		appSprintf( ProcName, TEXT("autoclass%s"), *GetName() );

		// Find export from the DLL.
		UPackage* ClassPackage = GetOuterUPackage();
		UClass** ClassPtr = (UClass**)ClassPackage->GetExport( ProcName, 0 );
		if( ClassPtr )
		{
			check(*ClassPtr);
			check(*ClassPtr==this);
			ClassConstructor = (*ClassPtr)->ClassConstructor;
		}
		else if( !GIsEditor )
		{
			appErrorf( TEXT("Can't bind to native class %s"), *GetPathName() );
		}
	}
	if( !ClassConstructor && GetSuperClass() )
	{
		// Chase down constructor in parent class.
		GetSuperClass()->Bind();
		ClassConstructor = GetSuperClass()->ClassConstructor;

		// propagate casting flags.
		ClassCastFlags |= GetSuperClass()->ClassCastFlags;
	}
	check(GIsEditor || ClassConstructor);
}

/**
 * Finds the component that is contained within this object that has the specified component name.
 * This version routes the call to the class's default object.
 *
 * @param	ComponentName	the component name to search for
 * @param	bRecurse		if TRUE, also searches all objects contained within this object for the component specified
 *
 * @return	a pointer to a component contained within this object that has the specified component name, or
 *			NULL if no components were found within this object with the specified name.
 */
UComponent* UClass::FindComponent( FName ComponentName, UBOOL bRecurse/*=FALSE*/ )
{
	UComponent* Result = NULL;

	UComponent** TemplateComponent = ComponentNameToDefaultObjectMap.Find(ComponentName);
	if ( TemplateComponent != NULL )
	{
		Result = *TemplateComponent;
	}

	if ( Result == NULL && ClassDefaultObject != NULL )
	{
		Result = ClassDefaultObject->FindComponent(ComponentName,bRecurse);
	}
	return NULL;
}

UBOOL UClass::ChangeParentClass( UClass* NewParentClass )
{
	check(NewParentClass);

	// changing parent classes is only allowed when running make
	if ( !GIsUCCMake )
	{
		return FALSE;
	}

	// only native script classes should ever change their parent on-the-fly, and
	// we can only change parents if we haven't been parsed yet.
	if ( HasAnyClassFlags(CLASS_Parsed|CLASS_Intrinsic) )
	{
		return FALSE;
	}

	// if we don't have an existing parent class, can't change it
	UClass* CurrentParent = GetSuperClass();
	if ( CurrentParent == NULL )
	{
		return FALSE;
	}

	// Cannot change our parent to a class that is a child of this class
	if ( NewParentClass->IsChildOf(this) )
	{
		return FALSE;
	}

	// First, remove the class flags that were inherited from the old parent class
	DWORD InheritedClassFlags = (CurrentParent->ClassFlags&CLASS_Inherit);
	ClassFlags &= ~InheritedClassFlags;
	ClassCastFlags &= ~StaticClassCastFlags;

	// then propagate the new parent's inheritable class flags
	ClassFlags |= (NewParentClass->ClassFlags&CLASS_ScriptInherit)|StaticClassFlags;
	ClassCastFlags |= NewParentClass->ClassCastFlags;

	// if this class already has a class default object, we'll need to detach it
	if ( ClassDefaultObject != NULL )
	{
		// ClassConfigName - figure out if this class defined its own StaticConfigName
		if ( ClassDefaultObject != NULL && !ClassDefaultObject->HasUniqueStaticConfigName() )
		{
			// if we inherited the StaticConfigName function from our ParentClass, change our ConfigName
			// to the new parent's ConfigName
			if ( NewParentClass->ClassDefaultObject != NULL )
			{
				ClassConfigName = NewParentClass->StaticConfigName();
			}
		}

		// break down any existing properties
		ClassDefaultObject->ExitProperties((BYTE*)ClassDefaultObject, this);

		// reset the object's vftable so that it doesn't cause an access violation when it's destructed
		(*UObject::StaticClass()->ClassConstructor)( ClassDefaultObject );

		// clear the reference
		ClassDefaultObject = NULL;
	}

	// next, copy the category info from the new parent class
#if !CONSOLE
	HideCategories = NewParentClass->HideCategories;
	AutoExpandCategories = NewParentClass->AutoExpandCategories;
	AutoCollapseCategories = NewParentClass->AutoCollapseCategories;
#endif

	// we're now considered misaligned
	SetFlags(RF_MisalignedObject);
	SuperField = NewParentClass;

	// finally, tell all our children to re-initialize themselves so that the new
	// data is propagated correctly
	for ( TObjectIterator<UClass> It; It; ++It )
	{
		if ( It->GetSuperClass() == this )
		{
			It->ChangeParentClass(this);
		}
	}

	return TRUE;
}


UBOOL UClass::HasNativesToExport( UObject* InOuter )
{
	if( HasAnyFlags( RF_Native ) )
	{
		if( HasAnyFlags( RF_TagExp ) )
		{
			return TRUE;
		}

		if( ScriptText && GetOuter() == InOuter )
		{
			for( TFieldIterator<UFunction,CASTCLASS_UFunction,FALSE> Function(this); Function; ++Function )
			{
				if( Function->FunctionFlags & FUNC_Native )
				{
					return TRUE;
				}
			}
		}
	}

	return FALSE;			
}


/**
 * Determines whether this class's memory layout is different from the C++ version of the class.
 * 
 * @return	TRUE if this class or any of its parent classes is marked as misaligned
 */
UBOOL UClass::IsMisaligned()
{
	UBOOL bResult = FALSE;
	for ( UClass* TestClass = this; TestClass; TestClass = TestClass->GetSuperClass() )
	{
		if ( TestClass->HasAnyFlags(RF_MisalignedObject) )
		{
			bResult = TRUE;
			break;
		}
	}

	return bResult;
}

/**
 * Returns the struct/ class prefix used for the C++ declaration of this struct/ class.
 * Classes deriving from AActor have an 'A' prefix and other UObject classes an 'U' prefix.
 *
 * @return Prefix character used for C++ declaration of this struct/ class.
 */
const TCHAR* UClass::GetPrefixCPP()
{
	UClass* TheClass				= this;
	UBOOL	IsActorClass		= FALSE;
#if !EXPORT_NATIVEINTERFACE_UCLASS
	UBOOL	bIsInterfaceClass	= HasAnyClassFlags(CLASS_Interface);
	while( TheClass && !bIsInterfaceClass && !IsActorClass )
	{
		IsActorClass	= TheClass->GetFName() == NAME_Actor;
		TheClass			= TheClass->GetSuperClass();
	}
	return IsActorClass
		? TEXT("A")
		: bIsInterfaceClass
			? TEXT("I")
			: TEXT("U");
#else
	while( TheClass && !IsActorClass )
	{
		IsActorClass	= TheClass->GetFName() == NAME_Actor;
		TheClass			= TheClass->GetSuperClass();
	}
	return IsActorClass ? TEXT("A") : TEXT("U");
#endif
}

FString UClass::GetDescription() const
{
	// Look up the the classes name in the INT file and return the class name if there is no match.
	FString Description = Localize( TEXT("Objects"), *GetName(), TEXT("Descriptions"), GetLanguage(), 1 );
	if( Description.Len() )
		return Description;
	else
		return FString( GetName() );
}

//	UClass UObject implementation.

IMPLEMENT_COMPARE_POINTER( UField, UnClass, { return A->GetNetIndex() - B->GetNetIndex(); } )

void UClass::FinishDestroy()
{
	// Empty arrays.
	//warning: Must be emptied explicitly in order for intrinsic classes
	// to not show memory leakage on exit.
	NetFields.Empty();

	ClassDefaultObject = NULL;
	DefaultPropText=TEXT("");

	Super::FinishDestroy();
}
void UClass::PostLoad()
{
	check(ClassWithin);
	Super::PostLoad();

	// Postload super.
	if( GetSuperClass() )
	{
		GetSuperClass()->ConditionalPostLoad();
	}
}
void UClass::Link( FArchive& Ar, UBOOL Props )
{
	Super::Link( Ar, Props );
	if( !GIsEditor )
	{
		NetFields.Empty();
		ClassReps = SuperField ? GetSuperClass()->ClassReps : TArray<FRepRecord>();
		for( TFieldIterator<UField,CASTCLASS_None,FALSE> It(this); It; ++It )
		{
			UProperty* P;
			UFunction* F;
			if( (P=Cast<UProperty>(*It))!=NULL )
			{
				if( P->PropertyFlags&CPF_Net )
				{
					NetFields.AddItem( *It );
					if( P->GetOuter()==this )
					{
						P->RepIndex = ClassReps.Num();
						for( INT i=0; i<P->ArrayDim; i++ )
							new(ClassReps)FRepRecord(P,i);
					}
				}
			}
			else if( (F=Cast<UFunction>(*It))!=NULL )
			{
				if( (F->FunctionFlags&FUNC_Net) && !F->GetSuperFunction() )
					NetFields.AddItem( *It );
			}
		}
		NetFields.Shrink();
		Sort<USE_COMPARE_POINTER(UField,UnClass)>( &NetFields(0), NetFields.Num() );
	}

	// Emit tokens for all properties that are unique to this class.
	if( Props )
	{
		// Iterate over properties defined in this class
		for( TFieldIterator<UProperty,CASTCLASS_UProperty,FALSE> It(this); It; ++It)
		{
			UProperty* Property = *It;
			Property->EmitReferenceInfo( &ThisOnlyReferenceTokenStream, 0 );
		}
	}

	bNeedsPropertiesLinked = PropertyLink == NULL && (GUglyHackFlags&HACK_ClassLoadingDisabled) == 0;
}

void UClass::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );

	// Variables.
	Ar << ClassFlags;
	Ar << ClassWithin << ClassConfigName;
	Ar << ComponentNameToDefaultObjectMap;
	Ar << Interfaces;
#if !CONSOLE
	if ( (GCookingTarget & UE3::PLATFORM_Console)==0 || (!Ar.IsSaving() || Ar.GetLinker() == NULL) )
	{
		Ar << HideCategories << AutoExpandCategories << AutoCollapseCategories;
	}
#endif

	// Defaults.

	// mark the archive as serializing defaults
	Ar.StartSerializingDefaults();

	if( Ar.IsLoading() )
	{
		check((DWORD)Align(GetPropertiesSize(), GetMinAlignment()) >= sizeof(UObject));
		check(!GetSuperClass() || !GetSuperClass()->HasAnyFlags(RF_NeedLoad));
		Ar << ClassDefaultObject;

		// In order to ensure that the CDO inherits config & localized property values from the parent class, we can't initialize the CDO until
		// the parent class's CDO has serialized its data from disk and called LoadConfig/LoadLocalized - this occurs in ULinkerLoad::Preload so the
		// call to InitClassDefaultObject [for non-intrinsic classes] is deferred until then.
		// When running make, we don't load data from .ini/.int files, so there's no need to wait
		if ( GIsUCCMake )
		{
			ClassDefaultObject->InitClassDefaultObject(this);
		}

		ClassUnique = 0;
	}
	else
	{
#if SUPPORTS_SCRIPTPATCH_CREATION
		//@script patcher
		check(GIsScriptPatcherActive||GetDefaultsCount()==GetPropertiesSize());
#else
		check(GetDefaultsCount()==GetPropertiesSize());
#endif

		// only serialize the class default object if the archive allows serialization of ObjectArchetype
		// otherwise, serialize the properties that the ClassDefaultObject references
		// The logic behind this is the assumption that the reason for not serializing the ObjectArchetype
		// is because we are performing some actions on objects of this class and we don't want to perform
		// that action on the ClassDefaultObject.  However, we do want to perform that action on objects that
		// the ClassDefaultObject is referencing, so we'll serialize it's properties instead of serializing
		// the object itself
		if ( !Ar.IsIgnoringArchetypeRef() )
		{
			Ar << ClassDefaultObject;
		}
		else if ( ClassDefaultObject != NULL )
		{
			ClassDefaultObject->Serialize(Ar);
		}
	}

	// mark the archive we that we are no longer serializing defaults
	Ar.StopSerializingDefaults();
}

/** serializes the passed in object as this class's default object using the given archive
 * @param Object the object to serialize as default
 * @param Ar the archive to serialize from
 */
void UClass::SerializeDefaultObject(UObject* Object, FArchive& Ar)
{
	Object->SerializeNetIndex(Ar);

	// tell the archive that it's allowed to load data for transient properties
	Ar.StartSerializingDefaults();

	//@script patcher: if script patches were applied to our linker, then we need to check to see if this struct's bytecode is being patched.
	// the linker should have a mapping of ExportMap index => bytecode patches, so we can check with negligable performance impact 
	FPatchData* PatchedDefaults = NULL;

	if( GIsUCCMake || ((Ar.IsLoading() || Ar.IsSaving()) && !Ar.WantBinaryPropertySerialization()) )
	{
	    // Script code.
	    if ( Ar.IsLoading() )
	    {
			//@{
			//@script patcher
		    ULinker* LinkerAr = Ar.GetLinker();
		    if ( LinkerAr != NULL )
		    {
			    ULinkerLoad* Linker = Object->GetLinker();
			    checkSlow(LinkerAr == Linker);
			    
			    PatchedDefaults = Linker->FindDefaultsPatch(Object->GetLinkerIndex());
			    if ( PatchedDefaults != NULL )
			    {
				    // first, move the regular file reader beyond the data that we're going to be skipping
				    FObjectExport& CDOExport = Linker->ExportMap(Object->GetLinkerIndex());
				    Ar.Seek(CDOExport.SerialOffset + CDOExport.SerialSize);

				    // then create a patch reader containing only the modified default properties for this object
				    FPatchReader BytecodePatcher(*PatchedDefaults);
				    BytecodePatcher.SetLoader(Linker);

				    // need to re-serialize the NetIndex from the patch reader
				    Object->SerializeNetIndex(BytecodePatcher);

				    // then read in all the property data
				    SerializeTaggedProperties(BytecodePatcher, (BYTE*)Object, GetSuperClass(), (BYTE*)Object->GetArchetype());
			    }
		    }
			//@}
	    }

	    if ( PatchedDefaults == NULL )
	    {
		    // class default objects do not always have a vtable (such as when saving the object
		    // during make), so use script serialization as opposed to native serialization to
		    // guarantee that all property data is loaded into the correct location
		    SerializeTaggedProperties(Ar, (BYTE*)Object, GetSuperClass(), (BYTE*)Object->GetArchetype());
		}
	}
	else if ( Ar.GetPortFlags() != 0 )
	{
		SerializeBinEx(Ar, (BYTE*)Object, (BYTE*)Object->GetArchetype(), GetSuperClass()->GetPropertiesSize());
	}
	else
	{
		SerializeBin(Ar, (BYTE*)Object, 0);
	}
	Ar.StopSerializingDefaults();
}

void UClass::PropagateStructDefaults()
{
	BYTE* DefaultData = GetDefaults();
	if ( DefaultData != NULL )
	{
		for( TFieldIterator<UStructProperty,CASTCLASS_UStructProperty,0> It(this); It; ++It )
		{
			UStructProperty* StructProperty = *It;

			// don't overwrite the values of properties which are marked native, since these properties
			// cannot be serialized by script.  For example, this would otherwise overwrite the 
			// VfTableObject property of UObject, causing all UObjects to have a NULL v-table.
			if ( (StructProperty->PropertyFlags&CPF_Native) == 0 )
			{
				StructProperty->InitializeValue( DefaultData + StructProperty->Offset );
			}
		}
	}

	Super::PropagateStructDefaults();
}
/*-----------------------------------------------------------------------------
	UClass constructors.
-----------------------------------------------------------------------------*/

/**
 * Internal constructor.
 */
UClass::UClass()
:	ClassWithin( UObject::StaticClass() )
{}

/**
 * Create a new UClass given its superclass.
 */
UClass::UClass( UClass* InBaseClass )
:	UState( InBaseClass )
,	ClassWithin( UObject::StaticClass() )
,	ClassDefaultObject( NULL )
,	bNeedsPropertiesLinked( TRUE )
{
	UClass* ParentClass = GetSuperClass();
	if( ParentClass )
	{
		ClassWithin = ParentClass->ClassWithin;
		Bind();

		// if this is a native class, we may have defined a StaticConfigName() which overrides
		// the one from the parent class, so get our config name from there
		if ( HasAnyFlags(RF_Native) )
		{
			ClassConfigName = StaticConfigName();
		}
		else
		{
			// otherwise, inherit our parent class's config name
			ClassConfigName = ParentClass->ClassConfigName;
		}
	}

	if ( !GIsUCCMake )
	{
		// this must be after the call to Bind(), so that the class has a ClassStaticConstructor
		UObject* DefaultObject = GetDefaultObject();
		if ( DefaultObject != NULL )
		{
			DefaultObject->InitClassDefaultObject(this);
			DefaultObject->LoadConfig();
			DefaultObject->LoadLocalized();
		}
	}
}

/**
 * UClass autoregistry constructor.
 * warning: Called at DLL init time.
 */
UClass::UClass
(
	ENativeConstructor,
	DWORD			InSize,
	DWORD			InClassFlags,
	DWORD			InClassCastFlags,
	UClass*			InSuperClass,
	UClass*			InWithinClass,
	const TCHAR*	InNameStr,
	const TCHAR*    InPackageName,
	const TCHAR*    InConfigName,
	EObjectFlags	InFlags,
	void			(*InClassConstructor)(void*),
	void			(UObject::*InClassStaticConstructor)(),
	void			(UObject::*InClassStaticInitializer)()
)
:	UState					( EC_NativeConstructor, InSize, InNameStr, InPackageName, InFlags, InSuperClass!=this ? InSuperClass : NULL )
,	ClassFlags				( InClassFlags | CLASS_Native )
,	ClassCastFlags			( InClassCastFlags )
,	ClassUnique				( 0 )
,	ClassWithin				( InWithinClass )
,	ClassConfigName			()
,	NetFields				()
,	ClassDefaultObject		( NULL )
,	ClassConstructor		( InClassConstructor )
,	ClassStaticConstructor	( InClassStaticConstructor )
,	ClassStaticInitializer	( InClassStaticInitializer )
,	bNeedsPropertiesLinked	( TRUE )
{
	*(const TCHAR**)&ClassConfigName = InConfigName;
}

/**
 * Called when statically linked.
 */
UClass::UClass
(
	EStaticConstructor,
	DWORD			InSize,
	DWORD			InClassFlags,
	DWORD			InClassCastFlags,
	const TCHAR*	InNameStr,
	const TCHAR*    InPackageName,
	const TCHAR*    InConfigName,
	EObjectFlags	InFlags,
	void			(*InClassConstructor)(void*),
	void			(UObject::*InClassStaticConstructor)(),
	void			(UObject::*InClassStaticInitializer)()
)
:	UState					( EC_StaticConstructor, InSize, InNameStr, InPackageName, InFlags )
,	ClassFlags				( InClassFlags | CLASS_Native )
,	ClassCastFlags			( InClassCastFlags )
,	ClassUnique				( 0 )
,	ClassWithin				( NULL )
,	ClassConfigName			()
,	NetFields				()
,	ClassDefaultObject		( NULL )
,	ClassConstructor		( InClassConstructor )
,	ClassStaticConstructor	( InClassStaticConstructor )
,	ClassStaticInitializer	( InClassStaticInitializer )
,	bNeedsPropertiesLinked	( TRUE )
{
	*(const TCHAR**)&ClassConfigName = InConfigName;
}

IMPLEMENT_CLASS(UClass);

/*-----------------------------------------------------------------------------
	FLabelEntry.
-----------------------------------------------------------------------------*/

FLabelEntry::FLabelEntry( FName InName, INT iInCode )
:	Name	(InName)
,	iCode	(iInCode)
{}
FArchive& operator<<( FArchive& Ar, FLabelEntry &Label )
{
#if SUPPORTS_SCRIPTPATCH_CREATION
	//@script patcher
	if( !GIsScriptPatcherActive ) 
	{
#endif
		Ar << Label.Name;
		Ar << Label.iCode;
#if SUPPORTS_SCRIPTPATCH_CREATION
	}
	else
	{
		//@script patcher
		// use class layout trickery to get the data we need
		Ar << *(NAME_INDEX*)&Label.Name; // NameIndex
		Ar << *(INT*)(((BYTE*)&Label.Name)+sizeof(NAME_INDEX)); // Number
		Ar << Label.iCode;
	}
#endif

	return Ar;
}

/*-----------------------------------------------------------------------------
	UFunction.
-----------------------------------------------------------------------------*/

UFunction::UFunction( UFunction* InSuperFunction )
: UStruct( InSuperFunction ), FirstStructWithDefaults(NULL)
{}
void UFunction::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );

	Ar.ThisContainsCode();

	// Function info.
	Ar << iNative;
	Ar << OperPrecedence;
	Ar << FunctionFlags;

	// Replication info.
	if( FunctionFlags & FUNC_Net )
		Ar << RepOffset;

	// Precomputation.
	if( Ar.IsLoading() )
	{
		NumParms          = 0;
		ParmsSize         = 0;
		ReturnValueOffset = MAXWORD;
		for( UProperty* Property=Cast<UProperty>(Children); Property; Property=Cast<UProperty>(Property->Next) )
		{
			if (Property->PropertyFlags & CPF_Parm)
			{
				NumParms++;
				ParmsSize = Property->Offset + Property->GetSize();
				if( Property->PropertyFlags & CPF_ReturnParm )
					ReturnValueOffset = Property->Offset;
			}
			else if ( (FunctionFlags&FUNC_HasDefaults) != 0 )
			{
				UStructProperty* StructProp = Cast<UStructProperty>(Property,CLASS_IsAUStructProperty);
				if ( StructProp && StructProp->Struct->GetDefaultsCount() )
				{
					FirstStructWithDefaults = StructProp;
					break;
				}
			}
			else
			{
				break;
			}
		}
	}

	Ar << FriendlyName;
}
void UFunction::PostLoad()
{
	Super::PostLoad();
}
UProperty* UFunction::GetReturnProperty()
{
	for( TFieldIterator<UProperty,CASTCLASS_UProperty> It(this); It && (It->PropertyFlags & CPF_Parm); ++It )
		if( It->PropertyFlags & CPF_ReturnParm )
			return *It;
	return NULL;
}
void UFunction::Bind()
{
	UClass* OwnerClass = GetOwnerClass();

	// if this isn't a native function, or this function belongs to a native interface class (which has no C++ version), 
	// use ProcessInternal (call into script VM only) as the function pointer for this function
	if( !HasAnyFunctionFlags(FUNC_Native) || OwnerClass->HasAnyClassFlags(CLASS_Interface) )
	{
		// Use UnrealScript processing function.
		check(iNative==0);
		Func = &UObject::ProcessInternal;
	}
	else if( iNative != 0 )
	{
		// Find hardcoded native.
		check(iNative<EX_Max);
		check(GNatives[iNative]!=0);
		Func = GNatives[iNative];
	}
	else
	{
		// Find dynamic native.
		TCHAR Proc[MAX_SPRINTF]=TEXT("");
		appSprintf( Proc, TEXT("int%s%sexec%s"), OwnerClass->GetPrefixCPP(), *OwnerClass->GetName(), *GetName() );
		UPackage* ClassPackage = OwnerClass->GetOuterUPackage();
		Native* Ptr = (Native*)ClassPackage->GetExport( Proc, TRUE );
		if( Ptr )
		{
			Func = *Ptr;
		}
	}
}
void UFunction::Link( FArchive& Ar, UBOOL Props )
{
	Super::Link( Ar, Props );
}
IMPLEMENT_CLASS(UFunction);

/*-----------------------------------------------------------------------------
	UConst.
-----------------------------------------------------------------------------*/

UConst::UConst( UConst* InSuperConst, const TCHAR* InValue )
:	UField( InSuperConst )
,	Value( InValue )
{}
void UConst::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );
	Ar << Value;
}
IMPLEMENT_CLASS(UConst);
