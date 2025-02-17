/*=============================================================================
	UnGenericDlgOptions.h: Option classes for generic dialogs
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/*------------------------------------------------------------------------------
	UOptionsProxy
------------------------------------------------------------------------------*/
class UOptionsProxy : public UObject
{
	DECLARE_ABSTRACT_CLASS(UOptionsProxy,UObject,0,UnrealEd)

	// Constructors.
	virtual void InitFields();
};


/*------------------------------------------------------------------------------
	UOptions2DShaper
------------------------------------------------------------------------------*/
class UOptions2DShaper : public UOptionsProxy
{
	DECLARE_CLASS(UOptions2DShaper,UOptionsProxy,0,UnrealEd)

	UEnum* AxisEnum;

    BYTE Axis;

	UOptions2DShaper();
	virtual void InitFields();
	void StaticConstructor();
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
};


/*------------------------------------------------------------------------------
	UOptions2DShaperSheet
------------------------------------------------------------------------------*/
class UOptions2DShaperSheet : public UOptions2DShaper
{
	DECLARE_CLASS(UOptions2DShaperSheet,UOptions2DShaper,0,UnrealEd)

	UOptions2DShaperSheet();
	virtual void InitFields();
	void StaticConstructor();
};


/*------------------------------------------------------------------------------
	UOptions2DShaperExtrude
------------------------------------------------------------------------------*/
class UOptions2DShaperExtrude : public UOptions2DShaper
{
	DECLARE_CLASS(UOptions2DShaperExtrude,UOptions2DShaper,0,UnrealEd)

	INT Depth;

	UOptions2DShaperExtrude();
	virtual void InitFields();
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
};


/*------------------------------------------------------------------------------
	UOptions2DShaperExtrudeToPoint
------------------------------------------------------------------------------*/
class UOptions2DShaperExtrudeToPoint : public UOptions2DShaper
{
	DECLARE_CLASS(UOptions2DShaperExtrudeToPoint,UOptions2DShaper,0,UnrealEd)

	INT Depth;

	UOptions2DShaperExtrudeToPoint();
	virtual void InitFields();
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
};


/*------------------------------------------------------------------------------
	UOptions2DShaperExtrudeToBevel
------------------------------------------------------------------------------*/
class UOptions2DShaperExtrudeToBevel : public UOptions2DShaper
{
	DECLARE_CLASS(UOptions2DShaperExtrudeToBevel,UOptions2DShaper,0,UnrealEd)

	INT Height, CapHeight;

	UOptions2DShaperExtrudeToBevel();
	virtual void InitFields();
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
};


/*------------------------------------------------------------------------------
	UOptions2DShaperRevolve
------------------------------------------------------------------------------*/
class UOptions2DShaperRevolve : public UOptions2DShaper
{
	DECLARE_CLASS(UOptions2DShaperRevolve,UOptions2DShaper,0,UnrealEd)

	INT SidesPer360, Sides;

	UOptions2DShaperRevolve();
	virtual void InitFields();
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
};


/*------------------------------------------------------------------------------
	UOptions2DShaperBezierDetail
------------------------------------------------------------------------------*/
class UOptions2DShaperBezierDetail : public UOptionsProxy
{
	DECLARE_CLASS(UOptions2DShaperBezierDetail,UOptionsProxy,0,UnrealEd)

	INT DetailLevel;

	UOptions2DShaperBezierDetail();
	virtual void InitFields();
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
};


/*------------------------------------------------------------------------------
	UOptionsDupObject
------------------------------------------------------------------------------*/
class UOptionsDupObject : public UOptionsProxy
{
	DECLARE_CLASS(UOptionsDupObject,UOptionsProxy,0,UnrealEd)

public:
	FString Package, Name;

	UOptionsDupObject();
	virtual void InitFields();
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
};


/*------------------------------------------------------------------------------
	UOptionsNewClassFromSel
------------------------------------------------------------------------------*/
class UOptionsNewClassFromSel : public UOptionsProxy
{
	DECLARE_CLASS(UOptionsNewClassFromSel,UOptionsProxy,0,UnrealEd)

public:
	FString Package, Name;

	UOptionsNewClassFromSel();
	virtual void InitFields();
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
};


