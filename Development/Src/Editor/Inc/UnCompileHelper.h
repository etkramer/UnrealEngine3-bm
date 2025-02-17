/*=============================================================================
UnCompileHelper.h: UnrealScript compiler helper classes.
Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnClassTree.h"

#ifndef __UNCOMPILEHELPER_H__
#define __UNCOMPILEHELPER_H__

extern class FCompilerMetadataManager*	GScriptHelper;

/*-----------------------------------------------------------------------------
	FPropertyBase.
-----------------------------------------------------------------------------*/

/**
 * Property data type enums.
 * @warning: if values in this enum are modified, you must update:
 * - these are also used in the GConversions table, indexed by enum value
 * - FPropertyBase::GetSize() hardcodes the sizes for each property type
 */
enum EPropertyType
{
	CPT_None			= 0,
	CPT_Byte			= 1,
	CPT_Int				= 2,
	CPT_Bool			= 3,
	CPT_Float			= 4,
	CPT_ObjectReference	= 5,
	CPT_Name			= 6,
	CPT_Delegate		= 7,
	CPT_Interface		= 8,
	CPT_Range           = 9,
	CPT_Struct			= 10,
	CPT_Vector          = 11,
	CPT_Rotation        = 12,
	CPT_String          = 13,
	CPT_Map				= 14,

	// when you add new property types, make sure you add the corresponding entry
	// in the PropertyTypeToNameMap array in UnScrCom.cpp!!
	CPT_MAX				= 15,
};

enum EPropertyReferenceType
{
	/** no reference */
	CPRT_None					=0,

	/** we're referencing this property in order to assign a value to it */
	CPRT_AssignValue			=1,

	/** we're referencing this property's value in such a way that doesn't require it be initialized (if checks, iterators, etc.) */
	CPRT_SimpleReference		=2,

	/** we're referecning this property's value in such a away that requires it to be initialized (assigning to another property, using in a function call, etc) */
	CPRT_AssignmentReference	=3,

	/** we're referencing this property in a way that both changes its value and references the value (combination of CPRT_AssignmentReference and CPRT_AssignValue) */
	CPRT_DualReference			=4,
};

enum EFunctionExportFlags
{
	FUNCEXPORT_Virtual		=0x00000001,	// function should be exported as a virtual function
	FUNCEXPORT_Final		=0x00000002,	// function declaration included "final" keyword.  Used to differentiate between functions that have FUNC_Final only because they're private
	FUNCEXPORT_NoExport		=0x00000004,	// only DECLARE_FUNCTION stub should be exported to header file
	FUNCEXPORT_Const		=0x00000008,	// export C++ declaration as const
	FUNCEXPORT_NoExportHeader =0x00000010,	// don't export the C++ declaration, only the DECLARE_FUNCTION wrapper
};

enum EPropertyHeaderExportFlags
{
	PROPEXPORT_Public		=0x00000001,	// property should be exported as public
	PROPEXPORT_Private		=0x00000002,	// property should be exported as private
	PROPEXPORT_Protected	=0x00000004,	// property should be exported as protected
};

#ifndef CASE_TEXT
#define CASE_TEXT(txt) case txt: return TEXT(#txt)
#endif

const TCHAR* GetPropertyTypeText( EPropertyType Type );
const TCHAR* GetPropertyRefText( EPropertyReferenceType Type );

/**
 * Basic information describing a type.
 */
class FPropertyBase
{
public:
	// Variables.
	EPropertyType Type;
	INT ArrayDim;
	QWORD PropertyFlags;

	/**
	 * A mask of EPropertyHeaderExportFlags which are used for modifying how this property is exported to the native class header
	 */
	DWORD PropertyExportFlags;
	union
	{
		class UEnum* Enum;
		class UClass* PropertyClass;
		class UScriptStruct* Struct;
		class UFunction* Function;
		DWORD BitMask;
		INT StringSize;
	};

	UClass*	MetaClass;

	/**
	 * For static array properties which use an enum value to specify the array size, corresponds to the UEnum that was used for the size.
	 */
	UEnum*	ArrayIndexEnum;

	FName	DelegateName;

	/** Raw string (not type-checked) used for specifying special text when exporting a property to the *Classes.h file */
	FString	ExportInfo;

	/** Map of key value pairs that will be added to the package's UMetaData for this property */
	TMap<FName, FString> MetaData;

	EPropertyReferenceType ReferenceType;

	/** @name Constructors */
	//@{
	FPropertyBase( EPropertyType InType, EPropertyReferenceType InRefType = CPRT_None )
	:	Type(InType), ArrayDim(1), PropertyFlags(0), PropertyExportFlags(PROPEXPORT_Public)
	, BitMask(0), MetaClass(NULL), ArrayIndexEnum(NULL), ReferenceType(InRefType), DelegateName(NAME_None)
	{}
	FPropertyBase( UEnum* InEnum, EPropertyReferenceType InRefType = CPRT_None )
	:	Type(CPT_Byte), ArrayDim(1), PropertyFlags(0), PropertyExportFlags(PROPEXPORT_Public)
	, Enum(InEnum), MetaClass(NULL), ArrayIndexEnum(NULL), ReferenceType(InRefType), DelegateName(NAME_None)
	{}
	FPropertyBase( UClass* InClass, UClass* InMetaClass=NULL, EPropertyReferenceType InRefType = CPRT_None )
	:	Type(CPT_ObjectReference), ArrayDim(1), PropertyFlags(0), PropertyExportFlags(PROPEXPORT_Public)
	, PropertyClass(InClass), MetaClass(InMetaClass), ArrayIndexEnum(NULL), ReferenceType(InRefType), DelegateName(NAME_None)
	{
		// if this is an interface class, we use the UInterfaceProperty class instead of UObjectProperty
		if ( InClass->HasAnyClassFlags(CLASS_Interface) )
		{
			Type = CPT_Interface;
		}
	}
	FPropertyBase( UScriptStruct* InStruct, EPropertyReferenceType InRefType = CPRT_None )
	:	Type(CPT_Struct), ArrayDim(1), PropertyFlags(0), PropertyExportFlags(PROPEXPORT_Public)
	, Struct(InStruct), MetaClass(NULL), ArrayIndexEnum(NULL), ReferenceType(InRefType), DelegateName(NAME_None)
	{}
	FPropertyBase( UProperty* Property, EPropertyReferenceType InRefType = CPRT_None )
	: PropertyExportFlags(PROPEXPORT_Public), DelegateName(NAME_None)
	{
		checkSlow(Property);

		UBOOL DynArray=0;
		QWORD PropagateFlags = 0;
		if( Property->GetClass()==UArrayProperty::StaticClass() )
		{
			DynArray = 1;
			// if we're an array, save up Parm flags so we can propagate them.
			// below the array will be assigned the inner property flags. This allows propagation of Parm flags (out, optional..)
			//@note: we need to explicitly specify CPF_Const instead of adding it to CPF_ParmFlags because CPF_ParmFlags is treated as exclusive;
			// i.e., flags that are in CPF_ParmFlags are not allowed in other variable types and vice versa
			PropagateFlags = Property->PropertyFlags & (CPF_ParmFlags | CPF_Const);
			Property = CastChecked<UArrayProperty>(Property)->Inner;
		}
		if( Property->GetClass()==UByteProperty::StaticClass() )
		{
			*this = FPropertyBase(CPT_Byte);
			Enum = Cast<UByteProperty>(Property)->Enum;
		}
		else if( Property->GetClass()==UIntProperty::StaticClass() )
		{
			*this = FPropertyBase(CPT_Int);
		}
		else if( Property->GetClass()==UBoolProperty::StaticClass() )
		{
			*this = FPropertyBase(CPT_Bool);
			BitMask = Cast<UBoolProperty>(Property)->BitMask;
		}
		else if( Property->GetClass()==UFloatProperty::StaticClass() )
		{
			*this = FPropertyBase(CPT_Float);
		}
		else if( Property->GetClass()==UClassProperty::StaticClass() )
		{
			*this = FPropertyBase(CPT_ObjectReference);
			PropertyClass = Cast<UClassProperty>(Property)->PropertyClass;
			MetaClass = Cast<UClassProperty>(Property)->MetaClass;
		}
		else if( Property->GetClass()==UComponentProperty::StaticClass() )
		{
			*this = FPropertyBase(CPT_ObjectReference);
			PropertyClass = Cast<UComponentProperty>(Property)->PropertyClass;
		}
		else if( Property->GetClass()==UObjectProperty::StaticClass() )
		{
			*this = FPropertyBase(CPT_ObjectReference);
			PropertyClass = Cast<UObjectProperty>(Property)->PropertyClass;
		}
		else if( Property->GetClass()==UNameProperty::StaticClass() )
		{
			*this = FPropertyBase(CPT_Name);
		}
		else if( Property->GetClass()==UStrProperty::StaticClass() )
		{
			*this = FPropertyBase(CPT_String);
		}
		else if ( Property->GetClass() == UMapProperty::StaticClass() )
		{
			*this = FPropertyBase(CPT_Map);
		}
		else if( Property->GetClass()==UStructProperty::StaticClass() )
		{
			*this = FPropertyBase(CPT_Struct);
			Struct = Cast<UStructProperty>(Property,CLASS_IsAUStructProperty)->Struct;
		}
		else if( Property->GetClass()==UDelegateProperty::StaticClass() )
		{
			*this = FPropertyBase(CPT_Delegate);
			Function = Cast<UDelegateProperty>(Property)->Function;
		}
		else if ( Property->GetClass()==UInterfaceProperty::StaticClass() )
		{
			*this = FPropertyBase(CPT_Interface);
			PropertyClass = Cast<UInterfaceProperty>(Property)->InterfaceClass;
		}
		else
		{
			appErrorf( TEXT("Unknown property type '%s'"), *Property->GetFullName() );
		}
		ArrayDim = DynArray ? 0 : Property->ArrayDim;
		PropertyFlags = Property->PropertyFlags | PropagateFlags;
		ReferenceType = InRefType;
		ArrayIndexEnum = Property->ArraySizeEnum;
	}
	//@}

	/** @name Functions */
	//@{
	INT GetSize() const //hardcoded sizes!!
	{
		static const INT ElementSizes[CPT_MAX] =
		{
			0,							sizeof(BYTE),		sizeof(INT),			sizeof(DWORD),
			sizeof(FLOAT),				sizeof(UObject*),	sizeof(FName),			sizeof(FScriptDelegate),
			sizeof(FScriptInterface),				0,		0,						0,
			0,										0,		sizeof(TMap<BYTE,BYTE>)
		};

		INT ElementSize;
		if ( Type == CPT_Struct )
		{
			if ( Struct->GetFName() == NAME_Pointer )
			{
				ElementSize = sizeof(void*);
			}
			else
			{
				ElementSize = Struct->GetPropertiesSize();
			}
		}
		else
		{
			ElementSize = ElementSizes[Type];
		}

		return ElementSize * ArrayDim;
	}

	/**
	 * Returns whether this token represents a vector
	 * 
	 * @return	TRUE if this token represents a vector
	 */
	UBOOL IsVector() const
	{
		return Type==CPT_Struct && Struct->GetFName() == NAME_Vector;
	}
	
	/**
	 * Returns whether this token represents a rotator
	 * 
	 * @return	TRUE if this token represents a rotator
	 */
	UBOOL IsRotator() const
	{
		return Type==CPT_Struct && Struct->GetFName() == NAME_Rotator;
	}

	/**
	 * Returns whether this token represents a pointer
	 * 
	 * @return	TRUE if this token represents a pointer
	 */
	UBOOL IsPointer() const
	{
		return Type == CPT_Struct && Struct->GetFName() == NAME_Pointer;
	}

	/**
	 * Returns whether this token represents an object reference
	 */
	UBOOL IsObject() const
	{
		return Type == CPT_ObjectReference || Type == CPT_Interface;
	}

	/**
	 * Determines whether this token represents a dynamic array.
	 */
	UBOOL IsDynamicArray() const
	{
		return ArrayDim == 0;
	}

	/**
	 * Determines whether this token's type is compatible with another token's type.
	 *
	 * @param	Other							the token to check against this one.
	 *											Given the following example expressions, VarA is Other and VarB is 'this':
	 *												VarA = VarB;
	 *
	 *												function func(type VarB) {}
	 *												func(VarA);
	 *
	 *												static operator==(type VarB_1, type VarB_2) {}
	 *												if ( VarA_1 == VarA_2 ) {}
	 *
	 * @param	bDisallowGeneralization			controls whether it should be considered a match if this token's type is a generalization
	 *											of the other token's type (or vice versa, when dealing with structs
	 * @param	bIgnoreImplementedInterfaces	controls whether two types can be considered a match if one type is an interface implemented
	 *											by the other type.
	 */
	UBOOL MatchesType( const FPropertyBase& Other, UBOOL bDisallowGeneralization, UBOOL bIgnoreImplementedInterfaces=FALSE ) const
	{
		check(Type!=CPT_None || !bDisallowGeneralization);

		UBOOL bIsObjectType = IsObject();
		UBOOL bOtherIsObjectType = Other.IsObject();
		UBOOL bIsObjectComparison = bIsObjectType && bOtherIsObjectType;
		UBOOL bReverseClassChainCheck = TRUE;

		// If converting to an l-value, we require an exact match with an l-value.
		if( (PropertyFlags&CPF_OutParm) != 0 )
		{
			// if the other type is not an l-value, disallow
			if ( (Other.PropertyFlags&CPF_OutParm) == 0 )
			{
				return FALSE;
			}

			// if the other type is const and we are not const, disallow
			if ( (Other.PropertyFlags&CPF_Const) != 0 && (PropertyFlags&CPF_Const) == 0 )
			{
				return FALSE;
			}

			if ( Type == CPT_Struct )
			{
				// Allow derived structs to be passed by reference, unless this is a dynamic array of structs
				bDisallowGeneralization = bDisallowGeneralization || IsDynamicArray() || Other.IsDynamicArray();
			}

			// if Type == CPT_ObjectReference, out object function parm; allow derived classes to be passed in
			// if Type == CPT_Interface, out interface function parm; allow derived classes to be passed in
			else if ( (PropertyFlags & CPF_Const) == 0 || !IsObject() )
			{
				// all other variable types must match exactly when passed as the value to an 'out' parameter
				bDisallowGeneralization = TRUE;
			}
			
			// both types are objects, but one is an interface and one is an object reference
			else if ( bIsObjectComparison && Type != Other.Type )
			{
				return FALSE;
			}
		}
		else if (Type == CPT_ObjectReference && Other.Type != CPT_Interface
			&&	(ReferenceType == CPRT_AssignmentReference || ReferenceType == CPRT_DualReference || (PropertyFlags & CPF_ReturnParm)))
		{
			bReverseClassChainCheck = FALSE;
		}

		// Check everything.
		if( Type==CPT_None && (Other.Type==CPT_None || !bDisallowGeneralization) )
		{
			// If Other has no type, accept anything.
			return TRUE;
		}
		else if( Type != Other.Type && !bIsObjectComparison )
		{
			// Mismatched base types.
			return FALSE;
		}
		else if( ArrayDim != Other.ArrayDim )
		{
			// Mismatched array dimensions.
			return FALSE;
		}
		else if( Type==CPT_Byte )
		{
			// Make sure enums match, or we're generalizing.
			return Enum==Other.Enum || (Enum==NULL && !bDisallowGeneralization);
		}
		else if( bIsObjectType )
		{
			check(PropertyClass!=NULL);

			// Make sure object types match, or we're generalizing.
			if( bDisallowGeneralization )
			{
				// Exact match required.
				return PropertyClass==Other.PropertyClass && MetaClass==Other.MetaClass;
			}
			else if( Other.PropertyClass==NULL )
			{
				// Cannonical 'None' matches all object classes.
				return TRUE;
			}
			else
			{
				// Generalization is ok (typical example of this check would look like: VarA = VarB;, where this is VarB and Other is VarA)
				if ( Other.PropertyClass->IsChildOf(PropertyClass) )
				{
					if ( !bIgnoreImplementedInterfaces || ((Type == CPT_Interface) == (Other.Type == CPT_Interface)) )
					{
						if ( PropertyClass!=UClass::StaticClass() || MetaClass == NULL || Other.MetaClass->IsChildOf(MetaClass) ||
							(bReverseClassChainCheck && (Other.MetaClass == NULL || MetaClass->IsChildOf(Other.MetaClass))) )
						{
							return TRUE;
						}
					}
				}
				// check the opposite class chain for object types
				else if (bReverseClassChainCheck && Type != CPT_Interface && bIsObjectComparison && PropertyClass != NULL && PropertyClass->IsChildOf(Other.PropertyClass))
				{
					if (Other.PropertyClass!=UClass::StaticClass() || MetaClass == NULL || Other.MetaClass == NULL || MetaClass->IsChildOf(Other.MetaClass) || Other.MetaClass->IsChildOf(MetaClass))
					{
						return TRUE;
					}
				}

				if ( PropertyClass->HasAnyClassFlags(CLASS_Interface) && !bIgnoreImplementedInterfaces )
				{
					if ( Other.PropertyClass->ImplementsInterface(PropertyClass) )
					{
						return TRUE;
					}
				}

				return FALSE;
			}
		}
		else if( Type==CPT_Struct )
		{
			check(Struct!=NULL);
			check(Other.Struct!=NULL);

			if ( Struct == Other.Struct )
			{
				// struct types match exactly 
				return TRUE;
			}

			// returning FALSE here prevents structs related through inheritance from being used interchangeably, such as passing a derived struct as the value for a parameter
			// that expects the base struct, or vice versa.  An easier example is assignment (e.g. Vector = Plane or Plane = Vector).
			// there are two cases to consider (let's use vector and plane for the example):
			// - Vector = Plane;
			//		in this expression, 'this' is the vector, and Other is the plane.  This is an unsafe conversion, as the destination property type is used to copy the r-value to the l-value
			//		so in this case, the VM would call CopyCompleteValue on the FPlane struct, which would copy 16 bytes into the l-value's buffer;  However, the l-value buffer will only be
			//		12 bytes because that is the size of FVector
			// - Plane = Vector;
			//		in this expression, 'this' is the plane, and Other is the vector.  This is a safe conversion, since only 12 bytes would be copied from the r-value into the l-value's buffer
			//		(which would be 16 bytes).  The problem with allowing this conversion is that what to do with the extra member (e.g. Plane.W); should it be left alone? should it be zeroed?
			//		difficult to say what the correct behavior should be, so let's just ignore inheritance for the sake of determining whether two structs are identical

			// Previously, the logic for determining whether this is a generalization of Other was reversed; this is very likely the culprit behind all current issues with 
			// using derived structs interchangeably with their base versions.  The inheritance check has been fixed; for now, allow struct generalization and see if we can find any further
			// issues with allowing conversion.  If so, then we disable all struct generalization by returning FALSE here.
 			// return FALSE;

			if ( bDisallowGeneralization )
			{
				return FALSE;
			}
			// if we don't need an exact match, see if we're generalizing.
			else
			{
				// Generalization is ok if this is not a dynamic array
				if ( !IsDynamicArray() && !Other.IsDynamicArray() )
				{
					if ( Other.Struct->IsChildOf(Struct) )
					{
						// this is the old behavior - it would allow memory overwrites if you assigned a derived value to a base variable; e.g. Vector = Plane;
// 						debugfSuppressed(NAME_DevCompile, TEXT("FPropertyBase::MatchesType PREVENTING match - Src:%s Destination:%s"), *Other.Struct->GetPathName(), *Struct->GetPathName());
					}
					else if ( Struct->IsChildOf(Other.Struct) )
					{
// 						debugfSuppressed(NAME_DevCompile, TEXT("FPropertyBase::MatchesType ALLOWING match - Src:%s Destination:%s"), *Other.Struct->GetPathName(), *Struct->GetPathName());
						return TRUE;
					}
				}

				return FALSE;
			}
		}
		else
		{
			// General match.
			return TRUE;
		}
	}

	FString Describe()
	{
		return FString::Printf(
			TEXT("Type:%s  Flags:%I64i  Enum:%s  PropertyClass:%s  Struct:%s  Function:%s  MetaClass:%s"),
			GetPropertyTypeText(Type), PropertyFlags,
			Enum!=NULL?*Enum->GetName():TEXT(""),
			PropertyClass!=NULL?*PropertyClass->GetName():TEXT("NULL"),
			Struct!=NULL?*Struct->GetName():TEXT("NULL"),
			Function!=NULL?*Function->GetName():TEXT("NULL"),
			MetaClass!=NULL?*MetaClass->GetName():TEXT("NULL")
			);
	}
	//@}
};

//
// Token types.
//
enum ETokenType
{
	TOKEN_None				= 0x00,		// No token.
	TOKEN_Identifier		= 0x01,		// Alphanumeric identifier.
	TOKEN_Symbol			= 0x02,		// Symbol.
	TOKEN_Const				= 0x03,		// A constant.
	TOKEN_Max				= 0x0D
};

/*-----------------------------------------------------------------------------
	FToken.
-----------------------------------------------------------------------------*/
//
// Information about a token that was just parsed.
//
class FToken : public FPropertyBase
{
public:
	/** @name Variables */
	//@{
	/** Type of token. */
	ETokenType	TokenType;
	/** Name of token. */
	FName		TokenName;
	/** Starting position in script where this token came from. */
	INT			StartPos;
	/** Starting line in script. */
	INT			StartLine;
	/** Always valid. */
	TCHAR		Identifier[NAME_SIZE];
	/** property that corresponds to this FToken - null if this Token doesn't correspond to a UProperty */
	UProperty*	TokenProperty;
	/** function that corresponds to this FToken - null if this Token doesn't correspond to a function */
	class FFunctionData*	TokenFunction;
	union
	{
		// TOKEN_Const values.
		BYTE	Byte;								 // If CPT_Byte.
		INT		Int;								 // If CPT_Int.
		UBOOL	Bool;								 // If CPT_Bool.
		FLOAT	Float;								 // If CPT_Float.
		UObject* Object;							 // If CPT_ObjectReference or CPT_Interface.
		BYTE	NameBytes[sizeof(FName)];			 // If CPT_Name.
		TCHAR	String[MAX_STRING_CONST_SIZE];		 // If CPT_String or IsPointer().
		BYTE	VectorBytes[sizeof(FVector)];		 // If CPT_Struct && IsVector().
		BYTE	RotationBytes[sizeof(FRotator)];	 // If CPT_Struct && IsRotator().
		BYTE	StructBytes[1024];					 // If CPT_Struct.!!hardcoded size
	};
	//@}

	/**
	 * Copies the properties from this token into another.
	 *
	 * @param	Other	the token to copy this token's properties to.
	 */
	void Clone( const FToken& Other )
	{
		// none of FPropertyBase's members require special handling
		(FPropertyBase&)*this	= (FPropertyBase&)Other;

		TokenType				= Other.TokenType;
		TokenName				= Other.TokenName;
		StartPos				= Other.StartPos;
		StartLine				= Other.StartLine;
		TokenProperty			= Other.TokenProperty;
		TokenFunction			= Other.TokenFunction;

		appStrncpy(Identifier, Other.Identifier, sizeof(Identifier));
		appMemcpy(String, Other.String, sizeof(String));
	}

	FString GetValue()
	{
		if ( TokenType == TOKEN_Const )
		{
			if (IsVector())
			{
				FVector& Vect = *(FVector*)VectorBytes;
				return FString::Printf(TEXT("FVector(%f,%f,%f)"),Vect.X, Vect.Y, Vect.Z);
			}
			else if (IsRotator())
			{
				FRotator& Rot = *(FRotator*)RotationBytes;
				return FString::Printf(TEXT("FRotator(%i,%i,%i)"), Rot.Pitch, Rot.Yaw, Rot.Roll);
			}
			else
			{
				switch ( Type )
				{
				case CPT_Byte:				return FString::Printf(TEXT("%u"), Byte);
				case CPT_Int:				return FString::Printf(TEXT("%i"), Int);
				// Don't use GTrue/GFalse here because they can be localized
				case CPT_Bool:				return FString::Printf(TEXT("%s"), Bool ? *(FName::GetEntry(NAME_TRUE)->GetNameString()) : *(FName::GetEntry(NAME_FALSE)->GetNameString()));
				case CPT_Float:				return FString::Printf(TEXT("%f"), Float);
				case CPT_Name:				return FString::Printf(TEXT("%s"), *(*(FName*)NameBytes).ToString());
				case CPT_String:			return String;
				case CPT_Vector:
					{
						FVector& Vect = *(FVector*)VectorBytes;
						return FString::Printf(TEXT("FVector(%f,%f,%f)"),Vect.X, Vect.Y, Vect.Z);
					}
				case CPT_Rotation:
					{
						FRotator& Rot = *(FRotator*)RotationBytes;
						return FString::Printf(TEXT("FRotator(%i,%i,%i)"), Rot.Pitch, Rot.Yaw, Rot.Roll);
					}

				// unsupported
				case CPT_Range:
				case CPT_Struct:
				case CPT_ObjectReference:
				case CPT_Interface:
				case CPT_Delegate:
					return TEXT("");

				}

				return TEXT("???");
			}
		}
		else
		{
			return TEXT("N/A");
		}
	}

	// Constructors.
	FToken( EPropertyReferenceType InRefType = CPRT_None )
	: FPropertyBase( CPT_None, InRefType ), TokenProperty(NULL), TokenFunction(NULL)
	{
		InitToken( CPT_None, InRefType );
	}

	FToken( UProperty* Property, EPropertyReferenceType InRefType = CPRT_None )
	: FPropertyBase(Property, InRefType), TokenProperty(Property), TokenFunction(NULL)
	{
		StartPos = 0;
		StartLine = 0;

		TokenName = TokenProperty->GetFName();
		appStrcpy(Identifier, *TokenName.ToString());
		TokenType = TOKEN_Identifier;
	}

	// copy constructors
	FToken( const FPropertyBase& InType )
	: FPropertyBase( CPT_None, InType.ReferenceType ), TokenProperty(NULL), TokenFunction(NULL)
	{
		InitToken( CPT_None, InType.ReferenceType );
		(FPropertyBase&)*this = InType;
	}

	// Inlines.
	void InitToken( EPropertyType InType, EPropertyReferenceType InRefType = CPRT_None )
	{
		(FPropertyBase&)*this = FPropertyBase(InType, InRefType);
		TokenType		= TOKEN_None;
		TokenName		= NAME_None;
		StartPos		= 0;
		StartLine		= 0;
		*Identifier		= 0;
		appMemzero(String, MAX_STRING_CONST_SIZE);
	}
	UBOOL Matches( const TCHAR* Str ) const
	{
		return (TokenType==TOKEN_Identifier || TokenType==TOKEN_Symbol) && appStricmp(Identifier,Str)==0;
	}
	UBOOL Matches( const FName& Name ) const
	{
		return TokenType==TOKEN_Identifier && TokenName==Name;
	}
	void AttemptToConvertConstant( const FPropertyBase& NewType )
	{
		check(TokenType==TOKEN_Const);
		switch( NewType.Type )
		{
		case CPT_Int:		{INT        V(0);           if( GetConstInt     (V) ) SetConstInt     (V); break;}
		case CPT_Bool:		{UBOOL      V(0);           if( GetConstBool    (V) ) SetConstBool    (V); break;}
		case CPT_Float:		{FLOAT      V(0.f);         if( GetConstFloat   (V) ) SetConstFloat   (V); break;}
		case CPT_Name:		{FName      V(NAME_None);   if( GetConstName    (V) ) SetConstName    (V); break;}
		case CPT_Struct:
			{
				if( NewType.IsVector() )
				{
					FVector V( 0.f, 0.f, 0.f );
					if( GetConstVector( V ) )
						SetConstVector( V );
				}
				else if( NewType.IsRotator() )
				{
					FRotator V( 0, 0, 0 );
					if( GetConstRotation( V ) )
						SetConstRotation( V );
				}
				//!!struct conversion support would be nice
				break;
			}
		case CPT_String:
			{
				break;
			}
		case CPT_Byte:
			{
				BYTE V=0;
				if( NewType.Enum==NULL && GetConstByte(V) )
					SetConstByte(NULL,V); 
				break;
			}
		case CPT_ObjectReference:
			{
				UObject* Ob=NULL; 
				if( GetConstObject( NewType.PropertyClass, Ob ) )
					SetConstObject( Ob ); 
				break;
			}
		}
	}

	// Setters.

	void SetTokenProperty( UProperty* Property )
	{
		TokenProperty = Property;
		TokenName = Property->GetFName();
		appStrcpy(Identifier, *TokenName.ToString());
		TokenType = TOKEN_Identifier;
	}

	void SetTokenFunction( FFunctionData* Function )
	{
		TokenFunction = Function;
	}
	void SetConstByte( UEnum* InEnum, BYTE InByte )
	{
		(FPropertyBase&)*this = FPropertyBase(CPT_Byte);
		Enum			= InEnum;
		Byte			= InByte;
		TokenType		= TOKEN_Const;
	}
	void SetConstInt( INT InInt )
	{
		(FPropertyBase&)*this = FPropertyBase(CPT_Int);
		Int				= InInt;
		TokenType		= TOKEN_Const;
	}
	void SetConstBool( UBOOL InBool )
	{
		(FPropertyBase&)*this = FPropertyBase(CPT_Bool);
		Bool 			= InBool;
		TokenType		= TOKEN_Const;
	}
	void SetConstFloat( FLOAT InFloat )
	{
		(FPropertyBase&)*this = FPropertyBase(CPT_Float);
		Float			= InFloat;
		TokenType		= TOKEN_Const;
	}
	void SetConstObject( UObject* InObject )
	{
		(FPropertyBase&)*this = FPropertyBase(CPT_ObjectReference);
		PropertyClass	= InObject ? InObject->GetClass() : NULL;
		Object			= InObject;
		TokenType		= TOKEN_Const;
		if( PropertyClass==UClass::StaticClass() )
			MetaClass = CastChecked<UClass>(InObject);
	}
	void SetConstDelegate()
	{
		(FPropertyBase&)*this = FPropertyBase(CPT_Delegate);
		TokenType = TOKEN_Const;
	}
	void SetConstName( FName InName )
	{
		(FPropertyBase&)*this = FPropertyBase(CPT_Name);
		*(FName *)NameBytes = InName;
		TokenType		= TOKEN_Const;
	}
	void SetConstString( TCHAR* InString, INT MaxLength=MAX_STRING_CONST_SIZE )
	{
		check(MaxLength>0);
		(FPropertyBase&)*this = FPropertyBase(CPT_String);
		if( InString != String )
			appStrncpy( String, InString, MaxLength );
		TokenType = TOKEN_Const;
	}
	void SetConstVector( FVector &InVector )
	{
		(FPropertyBase&)*this   = FPropertyBase(CPT_Struct);
		static UScriptStruct* VectorStruct = FindObjectChecked<UScriptStruct>(UObject::StaticClass(), TEXT("Vector"));
		Struct                  = VectorStruct;
		*(FVector *)VectorBytes = InVector;
		TokenType		        = TOKEN_Const;
	}
	void SetConstRotation( FRotator &InRotation )
	{
		(FPropertyBase&)*this      = FPropertyBase(CPT_Struct);
		static UScriptStruct* RotatorStruct = FindObjectChecked<UScriptStruct>(UObject::StaticClass(), TEXT("Rotator"));
		Struct                     = RotatorStruct;
		*(FRotator *)RotationBytes = InRotation;
		TokenType		           = TOKEN_Const;
	}
	//!!struct constants

	// Getters.
	UBOOL GetConstByte( BYTE& B ) const
	{
		if( TokenType==TOKEN_Const && Type==CPT_Byte )
		{
			B = Byte;
			return 1;
		}
		else if( TokenType==TOKEN_Const && Type==CPT_Int && Int>=0 && Int<256 )
		{
			B = (BYTE) Int;
			return 1;
		}
		else if( TokenType==TOKEN_Const && Type==CPT_Float && Float>=0 && Float<255 && Float==appTrunc(Float))
		{
			B = (BYTE) Float;
			return 1;
		}
		else return 0;
	}
	UBOOL GetConstInt( INT& I ) const
	{
		if( TokenType==TOKEN_Const && Type==CPT_Int )
		{
			I = Int;
			return 1;
		}
		else if( TokenType==TOKEN_Const && Type==CPT_Byte )
		{
			I = Byte;
			return 1;
		}
		else if( TokenType==TOKEN_Const && Type==CPT_Float && Float==appTrunc(Float))
		{
			I = (INT) Float;
			return 1;
		}
		else return 0;
	}
	UBOOL GetConstBool( UBOOL& B ) const
	{
		if( TokenType==TOKEN_Const && Type==CPT_Bool )
		{
			B = Bool;
			return 1;
		}
		else return 0;
	}
	UBOOL GetConstFloat( FLOAT& R ) const
	{
		if( TokenType==TOKEN_Const && Type==CPT_Float )
		{
			R = Float;
			return 1;
		}
		else if( TokenType==TOKEN_Const && Type==CPT_Int )
		{
			R = Int;
			return 1;
		}
		else if( TokenType==TOKEN_Const && Type==CPT_Byte )
		{
			R = Byte;
			return 1;
		}
		else return 0;
	}
	UBOOL GetConstObject( UClass* DesiredClass, UObject*& Ob ) const
	{
		if( TokenType==TOKEN_Const && Type==CPT_ObjectReference && (DesiredClass==NULL || PropertyClass->IsChildOf(DesiredClass)) )
		{
			Ob = Object;
			return 1;
		}
		return 0;
	}
	UBOOL GetConstName( FName& n ) const
	{
		if( TokenType==TOKEN_Const && Type==CPT_Name )
		{
			n = *(FName *)NameBytes;
			return 1;
		}
		return 0;
	}
	UBOOL GetConstVector( FVector& v ) const
	{
		if( TokenType==TOKEN_Const && IsVector() )
		{
			v = *(FVector *)VectorBytes;
			return 1;
		}
		return 0;
	}
	//!!struct constants
	UBOOL GetConstRotation( FRotator& r ) const
	{
		if( TokenType==TOKEN_Const && IsRotator() )
		{
			r = *(FRotator *)RotationBytes;
			return 1;
		}
		return 0;
	}

	FString Describe()
	{
		return FString::Printf(
			TEXT("Property:%s  Type:%s  TokenName:%s  Value:%s  Struct:%s  Flags:%I64i  RefType:%s"),
			TokenProperty!=NULL?*TokenProperty->GetName():TEXT("NULL"),
			GetPropertyTypeText(Type), *TokenName.ToString(), *GetValue(),
			Struct!=NULL?*Struct->GetName():TEXT("NULL"),
			PropertyFlags, GetPropertyRefText(ReferenceType)
			);
	}
};

/**
 * A group of FTokens.  Used for keeping track of reference chains tokens
 * e.g. SomeObject.default.Foo.DoSomething()
 */
class FTokenChain : public TArray<FToken>
{
public:
	FToken& operator+=( const FToken& NewToken )
	{
		FToken& Token = (*this)(AddZeroed()) = NewToken;
		return Token;
	}
};

/**
 * Information about a function being compiled.
 */
struct FFuncInfo
{
	/** @name Variables */
	//@{
	/** Name of the function or operator. */
	FToken		Function;
	/** Binary operator precedence. */
	INT			Precedence;
	/** Function flags. */
	DWORD		FunctionFlags;
	/** Function flags which are only required for exporting */
	DWORD		FunctionExportFlags;
	/** Index of native function. */
	INT			iNative;
	/** Number of parameters expected for operator. */
	INT			ExpectParms;
	/** Pointer to the UFunction corresponding to this FFuncInfo */
	UFunction*	FunctionReference;
	//@}

	/** Constructor. */
	FFuncInfo()
	:	Function		()
	,	Precedence		(0)
	,	FunctionFlags   (0)
	,	FunctionExportFlags(0)
	,	iNative			(0)
	,	ExpectParms		(0)
	,	FunctionReference(NULL)
	{}

	FFuncInfo( const FFuncInfo& Other )
	:	Precedence		(Other.Precedence)
	,	FunctionFlags   (Other.FunctionFlags)
	,	FunctionExportFlags(Other.FunctionExportFlags)
	,	iNative			(Other.iNative)
	,	ExpectParms		(Other.ExpectParms)
	,	FunctionReference(Other.FunctionReference)
	{
		Function.Clone(Other.Function);
	}
};

/**
 * Struct for storing information about an expression that is being used as the default value for an optional parameter.
 */
struct FDefaultParameterValue
{
	/** the original text for this expression - useful for complex expressions (such as context expressions, etc.) */
	FString			RawExpression;

	/** 
	 * Contains the FTokens that belong to each "chunk" of the complete expression.
	 * e.g. given the complete expression 'SomeObject.default.Foo', ParsedExpression will
	 * contain 3 members
	 */
	FTokenChain			ParsedExpression;

	/**
	 * The evaluated bytecode for the complete expression
	 */
	TArray<BYTE>	EvaluatedExpression;

	INT				InputLine;
	INT				InputPos;


	/**
	 * Copies the stored bytecode for the default value expression into the buffer specified
	 * 
	 * @param	Stream	the buffer to insert the stream into - typically TopNode->Script
	 * @return	whether a value was emitted
	 */
	UBOOL EmitValue( TArray<BYTE>& Stream )
	{
		if ( EvaluatedExpression.Num() )
		{
			INT iStart = Stream.Add(EvaluatedExpression.Num());
			appMemcpy( &Stream(iStart), &EvaluatedExpression(0), EvaluatedExpression.Num() );
			return true;
		}

		return false;
	}

	FDefaultParameterValue()
	: InputLine(1), InputPos(1)
	{}
};

/**
 * Stores "compiler" data about an FToken.  "Compiler" data is data that is associated with a
 * specific property, function or class that is only needed during script compile.
 * This class is designed to make adding new compiler data very simple.
 *
 * - stores the raw evaluated bytecode associated with an FToken
 */
struct FTokenData
{
	/** The token tracked by this FTokenData. */
	FToken			Token;

	/** For optional function parameters, holds the default value data */
	FDefaultParameterValue* DefaultValue;
	
	/**
	 * Creates a FDefaultParameterValue for this FTokenData.
	 *
	 * @return	the default value associated with this token
	 */
	FDefaultParameterValue* SetDefaultValue()
	{
		if ( DefaultValue == NULL )
		{
			DefaultValue = new FDefaultParameterValue();
		}

		return DefaultValue;
	}

	/**
	 * Copies the stored bytecode for the expression represented by this FTokenData
	 * into the buffer specified
	 * 
	 * @param	Stream	the buffer to insert the stream into - typically TopNode->Script
	 * @return	whether a value was emitted
	 */
	UBOOL EmitValue( TArray<BYTE>& Stream )
	{
		if ( DefaultValue != NULL )
			return DefaultValue->EmitValue(Stream);

		return false;
	}

	/** @name Constructors */
	//@{
	/**
	 * Defalt constructor
	 */
	FTokenData() : DefaultValue(NULL)
	{}

	/**
	 * Copy constructor
	 */
	FTokenData( const FToken& inToken )
	: Token(inToken), DefaultValue(NULL)
	{}
	//@}
};

/**
 * Class for storing data about a list of properties.  Though FToken contains a reference to its
 * associated UProperty, it's faster lookup to use the UProperty as the key in a TMap.
 */
class FPropertyData : public TMap<UProperty*,FTokenData>
{
public:

	/**
	 * (debug) Dumps the values of this FPropertyData to the log file
	 * 
	 * @param	Indent	number of spaces to insert at the beginning of each line
	 */	
	void Dump( INT Indent )
	{
		for ( TMap<UProperty*,FTokenData>::TIterator It(*this); It; ++It )
		{
			FToken& Token = It.Value().Token;
			if ( Token.Type != CPT_None )
				debugf(TEXT("%s%s"), appSpc(Indent), *Token.Describe());
		}
	}
};

/**
 * Class for storing additional data about compiled structs and struct properties
 */
class FStructData
{
public:
	/** info about the struct itself */
	FToken			StructData;

private:
	/** info for the properties contained in this struct */
	FPropertyData	StructPropertyData;

public:
	/**
	 * Adds a new struct property token
	 * 
	 * @param	PropertyToken	token that should be added to the list
	 */
	void AddStructProperty( const FToken& PropertyToken )
	{
		check(PropertyToken.TokenProperty);
		StructPropertyData.Set(PropertyToken.TokenProperty, PropertyToken);
	}

	FPropertyData& GetStructPropertyData()
	{
		return StructPropertyData;
	}
	const FPropertyData& GetStructPropertyData() const
	{
		return StructPropertyData;
	}

	/**
	* (debug) Dumps the values of this FStructData to the log file
	* 
	* @param	Indent	number of spaces to insert at the beginning of each line
	*/	
	void Dump( INT Indent )
	{
		debugf(TEXT("%s%s"), appSpc(Indent), *StructData.Describe());

		debugf(TEXT("%sproperties:"), appSpc(Indent));
		StructPropertyData.Dump(Indent + 4);
	}

	/** Constructor */
	FStructData( const FToken& StructToken ) : StructData(StructToken) {}
};

/**
 * Class for storing additional data about compiled function properties.
 */
class FFunctionData
{
	/** info about the function associated with this FFunctionData */
	FFuncInfo		FunctionData;

	/** return value for this function */
	FToken			ReturnTypeData;

	/** function parameter data */
	FPropertyData	ParameterData;

	/** function local property data */
	FPropertyData	LocalData;

	/**
	 * Adds a new parameter token
	 * 
	 * @param	PropertyToken	token that should be added to the list
	 */
	void AddParameter( const FToken& PropertyToken )
	{
		check(PropertyToken.TokenProperty);
		ParameterData.Set(PropertyToken.TokenProperty, PropertyToken);
	}

	/**
	 * Adds a new local property token
	 * 
	 * @param	PropertyToken	token that should be added to the list
	 */
	void AddLocalProperty( const FToken& PropertyToken )
	{
		check(PropertyToken.TokenProperty);
		LocalData.Set(PropertyToken.TokenProperty, PropertyToken);
	}

	/**
	 * Sets the value of the return token for this function
	 * 
	 * @param	PropertyToken	token that should be added
	 */
	void SetReturnData( const FToken& PropertyToken )
	{
		check(PropertyToken.TokenProperty);
		ReturnTypeData = PropertyToken;
	}

public:
	/** Constructors */
	FFunctionData() {}
	FFunctionData( const FFunctionData& Other )
	{
		(*this) = Other;
	}
	FFunctionData( const FFuncInfo& inFunctionData )
	: FunctionData(inFunctionData)
	{}

	/** Copy operator */
	FFunctionData& FFunctionData::operator=( const FFunctionData& Other )
	{
		FunctionData	= Other.FunctionData;
		ParameterData	= Other.ParameterData;
		LocalData		= Other.LocalData;
		ReturnTypeData.Clone(Other.ReturnTypeData);
		return *this;
	}
	
	/** @name getters */
	//@{
	const	FFuncInfo&		GetFunctionData()	const	{	return FunctionData;	}
	const	FToken&			GetReturnData()		const	{	return ReturnTypeData;	}
	const	FPropertyData&	GetParameterData()	const	{	return ParameterData;	}
	const	FPropertyData&	GetLocalData()		const	{	return LocalData;		}
	FPropertyData&			GetParameterData()			{	return ParameterData;	}
	FPropertyData&			GetLocalData()				{	return LocalData;		}
	//@}

	/**
	 * Adds a new function property to be tracked.  Determines whether the property is a
	 * function parameter, local property, or return value, and adds it to the appropriate
	 * list
	 * 
	 * @param	PropertyToken	the property to add
	 */
	void AddProperty( const FToken& PropertyToken )
	{
		const UProperty* Prop = PropertyToken.TokenProperty;
		check(Prop);

		if ( (Prop->PropertyFlags&CPF_ReturnParm) != 0 )
		{
			SetReturnData(PropertyToken);
		}
		else if ( (Prop->PropertyFlags&CPF_Parm) != 0 )
		{
			AddParameter(PropertyToken);
		}
		else
		{
			AddLocalProperty(PropertyToken);
		}
	}

	/**
	 * Empties the list of local property data for this function.  Called once the function is out of scope to reduce unecessary memory bloat.
	 */
	void ClearLocalPropertyData()
	{
		LocalData.Empty();
		LocalData.Shrink();
	}

	/**
	 * (debug) Dumps the values of this FFunctionData to the log file
	 * 
	 * @param	Indent	number of spaces to insert at the beginning of each line
	 */	
	void Dump( INT Indent )
	{
		debugf(TEXT("%slocals:"), appSpc(Indent));
		LocalData.Dump(Indent + 4);

		debugf(TEXT("%sparameters:"), appSpc(Indent));
		ParameterData.Dump(Indent + 4);

		debugf(TEXT("%sreturn prop:"), appSpc(Indent));
		if ( ReturnTypeData.Type != CPT_None )
			debugf(TEXT("%s%s"), appSpc(Indent + 4), *ReturnTypeData.Describe());
	}

	/**
	 * Sets the specified function export flags
	 */
	void SetFunctionExportFlag( DWORD NewFlags )
	{
		FunctionData.FunctionExportFlags |= NewFlags;
	}

	/**
	 * Clears the specified function export flags
	 */
	void ClearFunctionExportFlags( DWORD ClearFlags )
	{
		FunctionData.FunctionExportFlags &= ~ClearFlags;
	}
};

/**
 * Tracks information about a multiple inheritance parent declaration for native script classes.
 */
struct FMultipleInheritanceBaseClass
{
	/**
	 * The name to use for the base class when exporting the script class to header file.
	 */
	FString ClassName;

	/**
	 * For multiple inheritance parents declared using 'Implements', corresponds to the UClass for the interface.  For multiple inheritance parents declared
	 * using 'Inherits', this value will be NULL.
	 */
	UClass* InterfaceClass;

	/**
	 * Constructors
	 */
	FMultipleInheritanceBaseClass(const FString& BaseClassName)
	: ClassName(BaseClassName), InterfaceClass(NULL)
	{}

	FMultipleInheritanceBaseClass(UClass* ImplementedInterfaceClass)
	: InterfaceClass(ImplementedInterfaceClass)
	{
		ClassName = FString::Printf(TEXT("I%s"), *ImplementedInterfaceClass->GetName());
	}
};

/**
 * Class for storing compiler metadata about a class's properties.
 */
class FClassMetaData
{
	/** member properties for this class */
	FPropertyData							GlobalPropertyData;

	/** structs declared in this class */
	TMap<UScriptStruct*,FStructData>		StructData;

	/** functions of this class */
	TMap<UFunction*,FFunctionData>			FunctionData;

	/** base classes to multiply inherit from (other than the main base class */
	TArray<FMultipleInheritanceBaseClass>	MultipleInheritanceParents;

	/** whether this class declares delegate functions or properties */
	UBOOL									bContainsDelegates;

public:
	/** Default constructor */
	FClassMetaData()
	: bContainsDelegates(FALSE)
	{
	}

	/**
	 * Adds a new function to be tracked
	 * 
	 * @param	FunctionInfo	the function to add
	 *
	 * @return	a pointer to the newly added FFunctionData
	 */
	FFunctionData* AddFunction( const FFuncInfo& FunctionInfo )
	{
		check(FunctionInfo.FunctionReference!=NULL);
		FFunctionData* Result = FunctionData.Find(FunctionInfo.FunctionReference);
		if ( Result == NULL )
		{
			FFunctionData& NewFunctionData = FunctionData.Set(FunctionInfo.FunctionReference, FFunctionData(FunctionInfo));
			Result = &NewFunctionData;

			// update optimization flags
			bContainsDelegates = bContainsDelegates || (FunctionInfo.FunctionReference->FunctionFlags&FUNC_Delegate) != 0;
		}
		return Result;
	}

	/**
	 * Adds a new struct to be tracked
	 * 
	 * @param	StructToken		the token for the struct to add
	 *
	 * @return	a pointer to the newly added FStructData
	 */
	FStructData* AddStruct( const FToken& StructToken )
	{
		check(StructToken.Struct != NULL);

		FStructData* Result = StructData.Find(StructToken.Struct);
		if ( Result == NULL )
		{
			FStructData& NewStructData = StructData.Set(StructToken.Struct,FStructData(StructToken));
			Result = &NewStructData;
		}
		return Result;
	}

	/**
	 * Adds a new property to be tracked.  Determines the correct list for the property based on
	 * its owner (function, struct, etc).
	 * 
	 * @param	PropertyToken	the property to add
	 */
	void AddProperty( const FToken& PropertyToken )
	{
		UProperty* Prop = PropertyToken.TokenProperty;
		check(Prop);

		UObject* Outer = Prop->GetOuter();
		UClass* OuterClass = Cast<UClass>(Outer);
		if ( OuterClass != NULL )
		{
			// global property
			GlobalPropertyData.Set(Prop,PropertyToken);
		}
		else
		{
			UFunction* OuterFunction = Cast<UFunction>(Outer);
			if ( OuterFunction != NULL )
			{
				// function parameter, return, or local property
				FFunctionData* FuncData = FunctionData.Find(OuterFunction);
				check(FuncData != NULL);

				FuncData->AddProperty(PropertyToken);
			}
			else
			{
				// struct property
				UScriptStruct* OuterStruct = Cast<UScriptStruct>(Outer);
				check(OuterStruct != NULL);

				FStructData* StructInfo = StructData.Find(OuterStruct);
				check(StructInfo!=NULL);

				StructInfo->AddStructProperty(PropertyToken);
			}
		}

		// update the optimization flags
		if ( !bContainsDelegates )
		{
			if ( Prop->IsA(UDelegateProperty::StaticClass()) )
			{
				bContainsDelegates = TRUE;
			}
			else if ( Prop->IsA(UArrayProperty::StaticClass()) )
			{
				bContainsDelegates = static_cast<UArrayProperty*>(Prop)->Inner->IsA(UDelegateProperty::StaticClass());
			}
		}
	}

	/**
	 * Adds new editor-only property metadata (key/value pairs) to the class or struct that
	 * owns this property.
	 * 
	 * @param	PropertyToken	the property to add to
	 */
	void AddMetaData(const FToken& PropertyToken)
	{
		// only add if we have some!
		if (PropertyToken.MetaData.Num() == 0)
		{
			return;
		}

		// get the property and its outer
		UProperty* Prop = PropertyToken.TokenProperty;
		check(Prop);

		// only allow class/struct properties to have data
		// get (or create) a metadata object for this package
		UMetaData* MetaData = Prop->GetOutermost()->GetMetaData();

		// set the metadata for this property
		MetaData->SetObjectValues(Prop, PropertyToken.MetaData);
	}


	/**
	 * Finds the metadata for the function specified
	 * 
	 * @param	Func	the function to search for
	 *
	 * @return	pointer to the metadata for the function specified, or NULL
	 *			if the function doesn't exist in the list (for example, if it
	 *			is declared in a package that is already compiled and has had its
	 *			source stripped)
	 */
	FFunctionData* FindFunctionData( UFunction* Func );

	/**
	 * Finds the metadata for the struct specified
	 * 
	 * @param	Struct	the struct to search for
	 *
	 * @return	pointer to the metadata for the struct specified, or NULL
	 *			if the struct doesn't exist in the list (for example, if it
	 *			is declared in a package that is already compiled and has had its
	 *			source stripped)
	 */
	FStructData* FindStructData( UScriptStruct* Struct );

	/**
	 * Finds the metadata for the property specified
	 * 
	 * @param	Prop	the property to search for
	 *
	 * @return	pointer to the metadata for the property specified, or NULL
	 *			if the property doesn't exist in the list (for example, if it
	 *			is declared in a package that is already compiled and has had its
	 *			source stripped)
	 */
	FTokenData* FindTokenData( UProperty* Prop );

	/**
	 * (debug) Dumps the values of this FFunctionData to the log file
	 * 
	 * @param	Indent	number of spaces to insert at the beginning of each line
	 */	
	void Dump( INT Indent );

	/**
	 * Add a string to the list of inheritance parents for this class.
	 *
	 * @param Inparent	The C++ class name to add to the multiple inheritance list
	 */
	void AddInheritanceParent(const FString& InParent)
	{
		new(MultipleInheritanceParents) FMultipleInheritanceBaseClass(InParent);
	}

	/**
	 * Add a string to the list of inheritance parents for this class.
	 *
	 * @param Inparent	The C++ class name to add to the multiple inheritance list
	 */
	void AddInheritanceParent(UClass* ImplementedInterfaceClass)
	{
		new(MultipleInheritanceParents) FMultipleInheritanceBaseClass(ImplementedInterfaceClass);
	}

	/**
	 * Return the list of inheritance parents
	 */
	const TArray<FMultipleInheritanceBaseClass>& GetInheritanceParents() const
	{
		return MultipleInheritanceParents;
	}

	/**
	 * Returns whether this class contains any delegate properties which need to be fixed up.
	 */
	UBOOL ContainsDelegates() const
	{
		return bContainsDelegates;
	}

	/**
	 * Shrink TMaps to avoid slack in Pairs array.
	 */
	void Shrink()
	{
		GlobalPropertyData.Shrink();
		StructData.Shrink();
		FunctionData.Shrink();
		MultipleInheritanceParents.Shrink();
	}
};

/**
 * Class for storing and linking data about properties and functions that is only required by the compiler.
 * The type of data tracked by this class is data that would otherwise only be accessible by adding a 
 * member property to UFunction/UProperty.  
 */
class FCompilerMetadataManager : protected TMap<UClass*,FClassMetaData>
{
public:

	~FCompilerMetadataManager()
	{
		if ( this == GScriptHelper )
			GScriptHelper = NULL;
	}

	/**
	 * Adds a new class to be tracked
	 * 
	 * @param	Cls	the UClass to add
	 *
	 * @return	a pointer to the newly added metadata for the class specified
	 */
	FClassMetaData* AddClassData( UClass* Cls )
	{
		FClassMetaData* Result = Find(Cls);
		if ( Result == NULL )
		{
			FClassMetaData& NewClassData = Set(Cls, FClassMetaData());
			Result = &NewClassData;
		}

		return Result;
	}

	/**
	 * Find the metadata associated with the class specified
	 * 
	 * @param	Cls	the UClass to add
	 *
	 * @return	a pointer to the newly added metadata for the class specified
	 */
	FClassMetaData* FindClassData( UClass* Cls )
	{
		return Find(Cls);
	}

	/**
	 * (debug) Dumps the values of this FFunctionData to the log file
	 * 
	 * @param	Indent	number of spaces to insert at the beginning of each line
	 */	
	void Dump()
	{
		for ( TMap<UClass*,FClassMetaData>::TIterator It(*this); It; ++It )
		{
			UClass* Cls = It.Key();
			FClassMetaData& Data = It.Value();
			debugf(TEXT("=== %s ==="), *Cls->GetName());
			Data.Dump(4);
		}
	}

	/**
	 * Shrink TMaps to avoid slack in Pairs array.
	 */
	void Shrink()
	{
		TMap<UClass*,FClassMetaData>::Shrink();
		for( TMap<UClass*,FClassMetaData>::TIterator It(*this); It; ++It )
		{
			FClassMetaData& MetaData = It.Value();
			MetaData.Shrink();
		}
	}
};

/*-----------------------------------------------------------------------------
	Retry points.
-----------------------------------------------------------------------------*/

/**
 * A point in the script compilation state that can be set and returned to
 * using InitScriptLocation() and ReturnToLocation().  This is used in cases such as testing
 * to see which overridden operator should be used, where code must be compiled
 * and then "undone" if it was found not to match.
 * <p>
 * Retries are not allowed to cross command boundaries (and thus nesting 
 * boundaries).  Retries can occur across a single command or expressions and
 * subexpressions within a command.
 */
struct FScriptLocation
{
	static class FScriptCompiler* Compiler;

	/** the text buffer for the class associated with this retry point */
	const TCHAR* Input;

	/** the position into the Input buffer where this retry point is located */
	INT InputPos;

	/** the LineNumber of the compiler when this retry point was created */
	INT InputLine;

	/** the index into the Script array associated with this retry */
	INT CodeTop;

	/** Constructor */
	FScriptLocation();
};

/**
 * Information about a local variable declaration.
 */
struct FLocalProperty
{
	/** the UProperty represented by this FLocalProperty */
	UProperty *property;

	/** linked list functionality */
	FLocalProperty *next;


	/** first line that the property is assigned a value */
	INT AssignedLine;
	/** first line that the property is referenced */
	INT ReferencedLine;
	/** property is referenced in the function */
	UBOOL	bReferenced;
	/** property is used before it has ever been assigned a value. */
	UBOOL bUninitializedValue;//
	/** property's value is used for something */
	UBOOL bValueAssigned;
	/** property is assigned a value somewhere in the function */
	UBOOL bValueReferenced;

	FLocalProperty( UProperty* inProp )
	: bReferenced(0), bValueAssigned(0), bValueReferenced(0), bUninitializedValue(0), next(NULL)
	, AssignedLine(0), ReferencedLine(0)
	, property(inProp)
	{
		// if this is a struct property that has a default values, always consider it as having been assigned a value
		UStructProperty* StructProp = Cast<UStructProperty>(inProp,CLASS_IsAUStructProperty);
		if ( StructProp && StructProp->Struct->GetDefaultsCount() )
		{
			UClass* Class = StructProp->GetOwnerClass();

			FClassMetaData* ClassData = GScriptHelper->FindClassData(Class);
			if ( ClassData )
			{
				FTokenData* PropData = ClassData->FindTokenData(StructProp);
				if ( PropData )
				{
					ValueAssigned(PropData->Token.StartLine);
				}
			}
		}
	}

	FLocalProperty* GetLocalProperty( UProperty* SearchProp )
	{
		FLocalProperty* Result = NULL;
		if ( SearchProp == property )
			Result = this;

		if ( Result == NULL && next != NULL )
			Result = next->GetLocalProperty(SearchProp);

		return Result;
	}

	void ValueReferenced( INT Line )
	{
		bValueReferenced = true;
		if ( ReferencedLine == 0 )
			ReferencedLine = Line;
	}

	void ValueAssigned( INT Line )
	{
		bValueAssigned = true;

		if ( AssignedLine == 0 )
			AssignedLine = Line;
	}
};



#endif	// __UNCOMPILEHELPER_H__
