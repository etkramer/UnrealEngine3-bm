/*=============================================================================
	UnGenericDlgOptions.cpp: Option classes for generic dialogs
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"

/*------------------------------------------------------------------------------
	UOptionsProxy
------------------------------------------------------------------------------*/

void UOptionsProxy::InitFields()
{
}
IMPLEMENT_CLASS(UOptionsProxy);

/*------------------------------------------------------------------------------
	UOptions2DShaper
------------------------------------------------------------------------------*/
UOptions2DShaper::UOptions2DShaper()
{}

void UOptions2DShaper::InitFields()
{
	new(GetClass(),TEXT("Axis"), RF_Public)UByteProperty(CPP_PROPERTY(Axis), TEXT(""), CPF_Edit, AxisEnum );

	InitializeIntrinsicPropertyValues();
}

void UOptions2DShaper::StaticConstructor()
{
	AxisEnum = new( GetClass(), TEXT("Axis") )UEnum( NULL );
	TArray<FName> EnumNames;
	EnumNames.AddItem( FName( TEXT("AXIS_X") ) );
	EnumNames.AddItem( FName( TEXT("AXIS_Y") ) );
	EnumNames.AddItem( FName( TEXT("AXIS_Z") ) );
	AxisEnum->SetEnums( EnumNames );
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UOptions2DShaper::InitializeIntrinsicPropertyValues()
{
	Axis = AXIS_Y;
}
IMPLEMENT_CLASS(UOptions2DShaper);

/*------------------------------------------------------------------------------
	UOptions2DShaperSheet
------------------------------------------------------------------------------*/

UOptions2DShaperSheet::UOptions2DShaperSheet()
{}

void UOptions2DShaperSheet::InitFields()
{
	UOptions2DShaper::InitFields();

	InitializeIntrinsicPropertyValues();
}

void UOptions2DShaperSheet::StaticConstructor()
{
}
IMPLEMENT_CLASS(UOptions2DShaperSheet);

/*------------------------------------------------------------------------------
	UOptions2DShaperExtrude
------------------------------------------------------------------------------*/
UOptions2DShaperExtrude::UOptions2DShaperExtrude()
{}

void UOptions2DShaperExtrude::InitFields()
{
	UOptions2DShaper::InitFields();
	new(GetClass(),TEXT("Depth"), RF_Public)UIntProperty(CPP_PROPERTY(Depth), TEXT(""), CPF_Edit );

	InitializeIntrinsicPropertyValues();
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UOptions2DShaperExtrude::InitializeIntrinsicPropertyValues()
{
	Depth = 256;
}
IMPLEMENT_CLASS(UOptions2DShaperExtrude);

/*------------------------------------------------------------------------------
	UOptions2DShaperExtrudeToPoint
------------------------------------------------------------------------------*/
UOptions2DShaperExtrudeToPoint::UOptions2DShaperExtrudeToPoint()
{}

void UOptions2DShaperExtrudeToPoint::InitFields()
{
	UOptions2DShaper::InitFields();
	new(GetClass(),TEXT("Depth"), RF_Public)UIntProperty(CPP_PROPERTY(Depth), TEXT(""), CPF_Edit );

	InitializeIntrinsicPropertyValues();
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UOptions2DShaperExtrudeToPoint::InitializeIntrinsicPropertyValues()
{
	Depth = 256;
}
IMPLEMENT_CLASS(UOptions2DShaperExtrudeToPoint);

/*------------------------------------------------------------------------------
	UOptions2DShaperExtrudeToBevel
------------------------------------------------------------------------------*/
UOptions2DShaperExtrudeToBevel::UOptions2DShaperExtrudeToBevel()
{}

void UOptions2DShaperExtrudeToBevel::InitFields()
{
	UOptions2DShaper::InitFields();
	new(GetClass(),TEXT("Height"), RF_Public)UIntProperty(CPP_PROPERTY(Height), TEXT(""), CPF_Edit );
	new(GetClass(),TEXT("CapHeight"), RF_Public)UIntProperty(CPP_PROPERTY(CapHeight), TEXT(""), CPF_Edit );

	InitializeIntrinsicPropertyValues();
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UOptions2DShaperExtrudeToBevel::InitializeIntrinsicPropertyValues()
{
	Height = 128;
	CapHeight = 32;
}
IMPLEMENT_CLASS(UOptions2DShaperExtrudeToBevel);

/*------------------------------------------------------------------------------
	UOptions2DShaperRevolve
------------------------------------------------------------------------------*/
UOptions2DShaperRevolve::UOptions2DShaperRevolve()
{}

void UOptions2DShaperRevolve::InitFields()
{
	UOptions2DShaper::InitFields();
	new(GetClass(),TEXT("SidesPer360"), RF_Public)UIntProperty(CPP_PROPERTY(SidesPer360), TEXT(""), CPF_Edit );
	new(GetClass(),TEXT("Sides"), RF_Public)UIntProperty(CPP_PROPERTY(Sides), TEXT(""), CPF_Edit );

	InitializeIntrinsicPropertyValues();
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UOptions2DShaperRevolve::InitializeIntrinsicPropertyValues()
{
	SidesPer360 = 12;
	Sides = 3;
}
IMPLEMENT_CLASS(UOptions2DShaperRevolve);

/*------------------------------------------------------------------------------
	UOptions2DShaperBezierDetail
------------------------------------------------------------------------------*/

UOptions2DShaperBezierDetail::UOptions2DShaperBezierDetail()
{}

void UOptions2DShaperBezierDetail::InitFields()
{
	new(GetClass(),TEXT("DetailLevel"), RF_Public)UIntProperty(CPP_PROPERTY(DetailLevel), TEXT(""), CPF_Edit );
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UOptions2DShaperBezierDetail::InitializeIntrinsicPropertyValues()
{
	DetailLevel = 10;
}
IMPLEMENT_CLASS(UOptions2DShaperBezierDetail);

/*------------------------------------------------------------------------------
	UOptionsDupObject
------------------------------------------------------------------------------*/
UOptionsDupObject::UOptionsDupObject()
{}

void UOptionsDupObject::InitFields()
{
	new(GetClass(),TEXT("Name"), RF_Public)UStrProperty(CPP_PROPERTY(Name), TEXT(""), CPF_Edit );
	new(GetClass(),TEXT("Package"), RF_Public)UStrProperty(CPP_PROPERTY(Package), TEXT(""), CPF_Edit );

	new(GetClass()->HideCategories) FName(NAME_Object);
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UOptionsDupObject::InitializeIntrinsicPropertyValues()
{
	Package = TEXT("MyLevel");
	Name = TEXT("NewObject");

}
IMPLEMENT_CLASS(UOptionsDupObject);

/*------------------------------------------------------------------------------
	UOptionsNewClassFromSel
------------------------------------------------------------------------------*/
UOptionsNewClassFromSel::UOptionsNewClassFromSel()
{}

void UOptionsNewClassFromSel::InitFields()
{
	new(GetClass(),TEXT("Name"), RF_Public)UStrProperty(CPP_PROPERTY(Name), TEXT(""), CPF_Edit );
	new(GetClass(),TEXT("Package"), RF_Public)UStrProperty(CPP_PROPERTY(Package), TEXT(""), CPF_Edit );
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UOptionsNewClassFromSel::InitializeIntrinsicPropertyValues()
{
	Package = TEXT("MyLevel");
	Name = TEXT("MyClass");

}
IMPLEMENT_CLASS(UOptionsNewClassFromSel);

